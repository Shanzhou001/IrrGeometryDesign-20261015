"""Render the delivered PDF for review and record reproducible quality checks."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import re

import pymupdf as fitz

ROOT=Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--record-visual-review',action='store_true',
                        help='Record only after all latest rendered pages have actually been inspected.')
    args=parser.parse_args()
    pdf=ROOT/'output/pdf/tgrs_trace_selection.pdf'
    build=json.loads(pdf.with_suffix('.build.json').read_text(encoding='utf-8'))
    doc=fitz.open(pdf)
    cache=Path.home()/'.cache/codex-paper-tools/paper-preview'
    cache.mkdir(parents=True,exist_ok=True)
    pages=[];fonts={};references=set()
    for i,page in enumerate(doc):
        preview=cache/f'page-{i+1:02d}.png'
        page.get_pixmap(matrix=fitz.Matrix(1.7,1.7),alpha=False).save(preview)
        text=page.get_text()
        words=page.get_text('words')
        header_number=any(w[4]==str(i+1) and w[0]>page.rect.width*.85 and w[1]<50 for w in words)
        references.update(int(v) for v in re.findall(r'\[(\d+)\]',text))
        for font in page.get_fonts(full=True):
            name,ext,kind,buffer=doc.extract_font(font[0])
            fonts[font[0]]={'name':name,'format':ext,'type':kind,'embedded_bytes':len(buffer)}
        pages.append({'page':i+1,'width_pt':page.rect.width,'height_pt':page.rect.height,
                      'characters':len(text),'header_page_number_matches':header_number,
                      'raster_figures':len(page.get_images()),
                      'preview_sha256':sha(preview),'preview_file':str(preview)})
    image_manifest=json.loads((ROOT/'figures/manifest.json').read_text(encoding='utf-8'))
    paper_images={name:value for name,value in image_manifest['images'].items() if name.startswith('figures/paper/')}
    checks={'eight_pages':len(doc)==8,
            'letter_size':all(abs(p['width_pt']-612)<.1 and abs(p['height_pt']-792)<.1 for p in pages),
            'continuous_page_numbers':all(p['header_page_number_matches'] for p in pages),
            'all_fonts_embedded':all(f['embedded_bytes']>0 for f in fonts.values()),
            'times_family_body':any('Termes' in f['name'] for f in fonts.values()),
            'four_numbered_figures':sum(p['raster_figures'] for p in pages)==4,
            'references_one_to_nine':set(range(1,10)).issubset(references),
            'author_matches_user':doc.metadata['author']=='Wu shanzhou',
            'no_boundary_or_reference_errors':build['passed'] and not build['boundary_errors'] and not build['log_errors'],
            'pdf_matches_build':sha(pdf)==build['output_sha256'],
            'build_inputs_unchanged':all(sha(ROOT/name)==value for name,value in build['inputs_sha256'].items()),
            'paper_images_registered':len(paper_images)==4 and all(sha(ROOT/name)==r['sha256'] and r['generated_this_pass']==1 for name,r in paper_images.items())}
    result={'checked_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
            'command':['python','docs/paper/check_pdf.py',*(['--record-visual-review'] if args.record_visual_review else [])],
            'script_sha256':sha(__file__),'pdf_sha256':sha(pdf),'checks':checks,'pages':pages,
            'fonts':fonts,'metadata':doc.metadata,
            'visual_review':{'completed':args.record_visual_review,'reviewer':'Codex visual inspection' if args.record_visual_review else None,
                             'scope':'All eight latest rendered pages: columns, equations, tables, figure legends, references, and page endings.',
                             'note':'Automated boundary checks alone do not establish visual correctness; no human author review is claimed.'},
            'passed':all(checks.values()) and args.record_visual_review}
    out=ROOT/'results/paper_20261025/pdf_quality.json'
    out.write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps({'checks':checks,'visual_review_complete':args.record_visual_review,'report':str(out)},indent=2))
    if not all(checks.values()):raise SystemExit(1)


if __name__=='__main__':
    main()
