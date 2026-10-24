"""Explicit access to original relocated artifacts; scientific CSV/data stay live-checked."""
import hashlib
import json
from pathlib import Path
import subprocess

from research.common.hashing import digest
from research.common.paths import ROOT, executable


class HistoricalFiles:
    def __init__(self):
        self.mapping = json.loads((ROOT/'config/path_migrations.json').read_text(encoding='utf-8'))
        self.process = subprocess.Popen(['git','cat-file','--batch'],cwd=ROOT,
                                        stdin=subprocess.PIPE,stdout=subprocess.PIPE)
        self.cache = {}
        self.originals = set()
        baseline=json.loads((ROOT/'results/audit_20261024_v5/baseline_manifest.json').read_text(encoding='utf-8'))
        self.expected={name:r['sha256'] for name,r in baseline['protected_files'].items()}
        source=json.loads((ROOT/'results/review_20261024_v4/final/run_manifest.json').read_text(encoding='utf-8'))['fingerprint']
        self.expected.update(source['source_sha256']);self.expected.update(source['tool_sha256'])
        self.expected={name.replace('\\','/'):value for name,value in self.expected.items()}
        v5=json.loads((ROOT/'results/audit_20261024_v5/search/manifest.json').read_text(encoding='utf-8'))
        self.expected['docs/EXPERIMENT_PROTOCOL.md']=v5['configuration']['configuration_document_sha256']
        self.line_ending_reconstructed=set()
        self.exact_snapshot_sources=set()

    def close(self):
        self.process.stdin.close();self.process.stdout.close();self.process.wait()

    def original(self, name, revision='c191e6c5'):
        key=revision+':'+name
        if key not in self.cache:
            expected=self.expected.get(name)
            snapshot=ROOT/'results/audit_20261024_v5/runtimes/v5/sources'/name
            if name.startswith(('IrrGeometryDesign/','tools/')) and expected and snapshot.is_file() and digest(snapshot)==expected:
                self.cache[key]=snapshot.read_bytes();self.exact_snapshot_sources.add(snapshot.relative_to(ROOT).as_posix())
                return self.cache[key]
            self.process.stdin.write((key+'\n').encode('utf-8'));self.process.stdin.flush()
            self.originals.add(key)
            header=self.process.stdout.readline().decode().split()
            if header[-1]=='missing':raise FileNotFoundError(key)
            data=self.process.stdout.read(int(header[-1]));assert self.process.stdout.read(1)==b'\n'
            expected=self.expected.get(name)
            if expected and hashlib.sha256(data).hexdigest()!=expected:
                # Git may store LF while the frozen Windows experiment hashed CRLF.
                # Accept a reconstruction only if it reproduces the recorded SHA exactly.
                for candidate in [data.replace(b'\r\n',b'\n'),data.replace(b'\r\n',b'\n').replace(b'\n',b'\r\n')]:
                    if hashlib.sha256(candidate).hexdigest()==expected:
                        data=candidate;self.line_ending_reconstructed.add(key);break
            self.cache[key]=data
        return self.cache[key]

    def digest(self, path, algorithm='sha256'):
        path=Path(path)
        if not path.is_absolute():path=ROOT/path
        try:name=path.relative_to(ROOT).as_posix()
        except ValueError:return digest(path,algorithm)
        if path==executable():
            return digest(ROOT/'results/audit_20261024_v5/baseline_runtime/x64/Release/IrrGeometryDesign.exe',algorithm)
        if name.startswith(('IrrGeometryDesign/','tools/')):
            data=self.original(name,'72240bbf')
        elif name in self.mapping.get('images',{}).get('aliases',{}) or name in self.mapping.get('rewritten_references',{}):
            data=self.original(name)
        elif name in self.mapping.get('retired',{}):
            data=self.original(name)
        else:
            # No fallback for a changed or missing scientific input/output.
            return digest(path,algorithm)
        return hashlib.new(algorithm,data).hexdigest()

    def check(self, name, expected):
        path=ROOT/name
        if path.is_file() and digest(path)==expected:return 'working_bytes'
        actual=self.digest(path)
        if actual!=expected:raise AssertionError('archive hash mismatch: '+name)
        return 'original_git_bytes'
