"""Verify manuscript evidence and render every final PDF page for visual review."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path

import pymupdf
from PIL import Image, ImageDraw

ROOT=Path(__file__).resolve().parents[3]
OUTPUT=ROOT/'IrrGeometryDesign/checks/closure_20261027'
HISTORICAL=ROOT/'IrrGeometryDesign/checks/maintenance_20261026'


def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def run(final=False):
    build=json.loads((ROOT/'paper-tgrs/build_manifest.json').read_text(encoding='utf8'))
    for name,record in build['evidence'].items():
        if digest(ROOT/name)!=record['sha256']:raise ValueError('Manuscript evidence changed: '+name)
    if final:
        experiment=json.loads((ROOT/'IrrGeometryDesign/results/viking_robust_20261026/manifest.json').read_text())
        audit=json.loads((HISTORICAL/'robust_archive_audit/verification.json').read_text())
        if not experiment.get('completed') or experiment.get('status')!='passed' or audit['status']!='passed':
            raise ValueError('Robustness result or its independent audit is incomplete')
        external=json.loads((ROOT/'IrrGeometryDesign/results/f3_transfer_20261027/manifest.json').read_text())
        external_audit=json.loads((OUTPUT/'f3_archive_audit/verification.json').read_text())
        if not external.get('completed') or external.get('status')!='passed' or external_audit['status']!='passed':
            raise ValueError('External F3 transfer or its independent audit is incomplete')
        if external_audit['experiment_manifest_sha256']!=digest(ROOT/'IrrGeometryDesign/results/f3_transfer_20261027/manifest.json'):
            raise ValueError('External F3 audit belongs to a different manifest')
    folder=OUTPUT/'pdf_qa_final';folder.mkdir(parents=True,exist_ok=True);reports=[]
    for output in build['outputs']:
        pdf=ROOT/output['pdf']
        if digest(pdf)!=output['sha256']:raise ValueError('PDF changed since build: '+str(pdf))
        lang='chinese' if 'thesis/' in output['pdf'] else 'english'
        expanded=pdf.parent/('thesis_expanded.md' if lang=='chinese' else 'manuscript_expanded.md')
        text=expanded.read_text(encoding='utf8')
        if '{{' in text or '\ufffd' in text:raise ValueError('Unresolved text in '+str(expanded))
        if final and ('planned grid' in text or '计划形成1215' in text):raise ValueError('Prospective robustness text remains')
        doc=pymupdf.open(pdf); extracted='';outside=[];tiles=[]
        for index,page in enumerate(doc):
            extracted+=page.get_text()
            for block in page.get_text('blocks'):
                if block[0]<18 or block[1]<10 or block[2]>page.rect.width-18 or block[3]>page.rect.height-10:
                    outside.append(dict(page=index+1,bounds=list(block[:4])))
            picture=folder/f'{lang}_{index+1:02d}.png'
            page.get_pixmap(matrix=pymupdf.Matrix(1.4,1.4),alpha=False).save(picture)
            im=Image.open(picture).convert('RGB');im.thumbnail((390,555))
            tile=Image.new('RGB',(410,590),'#dbe3eb');tile.paste(im,((410-im.width)//2,20))
            ImageDraw.Draw(tile).text((12,572),f'{lang} {index+1}',fill='black');tiles.append(tile)
        if outside or '\ufffd' in extracted:raise ValueError('PDF clipping or replacement glyph: '+str(outside))
        if lang=='chinese' and any(f'第{n}章' not in extracted for n in range(1,7)):
            raise ValueError('A thesis chapter is absent')
        if lang=='chinese' and '地震采样点筛选研究初稿' not in extracted:raise ValueError('Incorrect Chinese page header')
        contacts=[]
        for start in range(0,len(tiles),9):
            same=tiles[start:start+9];canvas=Image.new('RGB',(1230,590*((len(same)+2)//3)),'white')
            for j,tile in enumerate(same):canvas.paste(tile,((j%3)*410,(j//3)*590))
            path=folder/f'{lang}_contact_{start//9+1}.jpg';canvas.save(path,quality=90);contacts.append(str(path))
        reports.append(dict(pdf=output['pdf'],sha256=digest(pdf),pages=len(doc),extracted_characters=len(extracted),
                            all_text_within_page=True,contact_sheets=contacts))
    result=dict(status='machine_checks_passed',checked_utc=datetime.now(timezone.utc).isoformat(),
                final_robustness_required=final,visual_review='pending manual inspection',outputs=reports,
                build_manifest_sha256=digest(ROOT/'paper-tgrs/build_manifest.json'))
    (OUTPUT/'pdf_qa.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result,ensure_ascii=False,indent=2))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--final',action='store_true')
    run(parser.parse_args().final)
