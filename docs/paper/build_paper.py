"""Compile the IEEEtran manuscript, reject unresolved references, and audit the PDF."""
import argparse
import csv
import datetime
import hashlib
import importlib.metadata
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def result_tables():
    evidence = ROOT/'results/paper_20261025/evidence/recomputed_statistics.json'
    pairs = ROOT/'results/audit_20261024_v5/analysis_thesis/joint_pairs.csv'
    data=json.loads(evidence.read_text(encoding='utf-8'))
    search=[]
    for r in data['v4_recomputed_bootstrap']:
        ref={'Adaptive-Hybrid':'Legacy hybrid','Adaptive-SAOnly':'SA-only'}[r['comparison']]
        lo,hi=r['ci95']
        search.append(f"{ref} & ${r['mean_difference']:.5f}$ & $[{lo:.5f},{hi:.5f}]$ & ${r['archived_p_holm']:.5g}$ "+r'\\')
    recovery=[]
    with pairs.open(encoding='utf-8',newline='') as stream:
        for r in csv.DictReader(stream):
            if r['recovery']!='fixed' or r['comparison'] not in ['Adaptive-Jitter','Adaptive-SAOnly']:continue
            ref={'Adaptive-Jitter':'Jitter','Adaptive-SAOnly':'SA-only'}[r['comparison']]
            recovery.append(f"{r['m']} & {ref} & ${float(r['difference']):.3f}$ & $[{float(r['ci_low']):.3f},{float(r['ci_high']):.3f}]$ & ${float(r['p_holm']):.2f}$ "+r'\\')
    assert len(search)==2 and len(recovery)==6
    target=Path(__file__).with_name('generated_tables.tex')
    target.write_text('% Generated from frozen tables; do not enter results by hand.\n'+
                      '\\newcommand{\\SearchContrastRows}{\n'+'\n'.join(search)+'\n}\n'+
                      '\\newcommand{\\RecoveryContrastRows}{\n'+'\n'.join(recovery)+'\n}\n',encoding='utf-8')
    return [evidence,pairs,target]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', type=Path)
    parser.add_argument('--require-pages', type=int, default=8)
    args = parser.parse_args()
    compiler = args.compiler or shutil.which('tectonic') or Path.home()/'.cache/codex-paper-tools/tectonic.exe'
    compiler = Path(compiler).resolve()
    if not compiler.is_file():
        parser.error('Tectonic compiler missing; provide --compiler PATH. See README.md.')
    from pypdf import PdfReader
    import pymupdf as fitz
    source = Path(__file__).with_name('tgrs_trace_selection.tex')
    inputs = [source, Path(__file__), ROOT/'research/analysis/paper_figures.py', *result_tables()]
    inputs += sorted((ROOT/'figures/paper').glob('*.png'))
    inputs += sorted((ROOT/'figures/paper').glob('*.data.json'))
    if len(list((ROOT/'figures/paper').glob('*.png'))) != 4:
        parser.error('Generate the four manuscript figures using python pipeline.py figures first.')
    before = {p.relative_to(ROOT).as_posix(): sha(p) for p in inputs}
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    attempt = ROOT/'results/paper_20261025/manuscript_build'/stamp
    attempt.mkdir(parents=True, exist_ok=False)
    command = [str(compiler), '--keep-logs', '--keep-intermediates', '--outdir', str(attempt), str(source)]
    run = subprocess.run(command, cwd=source.parent, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    (attempt/'command.log').write_bytes(run.stdout)
    state = {'command': command, 'exit_code': run.returncode, 'cwd': str(source.parent),
             'git_commit': subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT).decode().strip(),
             'compiler_sha256':sha(compiler), 'inputs_sha256':before,
             'python':sys.version,
             'packages':{name:importlib.metadata.version(name) for name in ['pypdf','PyMuPDF','numpy','matplotlib']},
             'timestamp_utc':stamp, 'passed':False}
    (attempt/'build.json').write_text(json.dumps(state, indent=2), encoding='utf-8')
    if run.returncode:
        print(run.stdout.decode(errors='replace'))
        raise RuntimeError('LaTeX compilation failed; see '+str(attempt))
    pdf = attempt/(source.stem+'.pdf')
    log = (attempt/(source.stem+'.log')).read_text(encoding='utf-8', errors='replace')
    bad = [line for line in log.splitlines() if any(token in line for token in
           ['Undefined control sequence','undefined references','Citation `','Reference `','Overfull \\hbox'])]
    reader = PdfReader(pdf)
    texts = [page.extract_text() or '' for page in reader.pages]
    doc = fitz.open(pdf)
    boundary_errors = []
    page_stats = []
    for i, page in enumerate(doc):
        spans=[s for b in page.get_text('dict')['blocks'] if 'lines' in b for l in b['lines'] for s in l['spans']]
        for span in spans:
            box=fitz.Rect(span['bbox'])
            if box.x0 < -1 or box.y0 < -1 or box.x1 > page.rect.width+1 or box.y1 > page.rect.height+1:
                boundary_errors.append({'page':i+1,'text':span['text'],'bbox':span['bbox']})
        page_stats.append({'page':i+1,'width_pt':page.rect.width,'height_pt':page.rect.height,
                           'characters':len(texts[i]),'text_spans':len(spans)})
    state.update(pages=len(reader.pages), required_pages=args.require_pages, log_errors=bad,
                 page_stats=page_stats, boundary_errors=boundary_errors,
                 references_resolved=not bad, image_count=sum(len(page.get_images()) for page in doc))
    state['inputs_unchanged'] = all(sha(ROOT/name)==value for name,value in before.items())
    state['passed'] = (len(reader.pages)==args.require_pages and not bad and not boundary_errors
                       and state['inputs_unchanged'] and all(len(t)>100 for t in texts))
    (attempt/'extracted_text.txt').write_text('\n\n'.join(texts), encoding='utf-8')
    (attempt/'build.json').write_text(json.dumps(state, indent=2), encoding='utf-8')
    print(json.dumps({k:state[k] for k in ['pages','required_pages','log_errors','boundary_errors','passed']}))
    print(attempt)
    if not state['passed']:
        raise RuntimeError('PDF checks failed; retain this attempt and revise the source.')
    output=ROOT/'output/pdf/tgrs_trace_selection.pdf'
    output.parent.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(pdf, output)
    state['output']=output.relative_to(ROOT).as_posix()
    state['output_sha256']=sha(output)
    (output.parent/'tgrs_trace_selection.build.json').write_text(json.dumps(state,indent=2),encoding='utf-8')
    print('PASS', output)


if __name__ == '__main__':
    main()
