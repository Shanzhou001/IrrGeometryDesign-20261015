"""One-time, recorded relocation of the pre-refactor active code (c191e6c5).

Historical runtime snapshots and scientific data are intentionally not source trees.
"""
from pathlib import Path
import ast
import json
import os
import re
import shutil
import subprocess
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]

GROUPS = {
    'data': ['convert_rraw_to_engine_csv', 'make_59x256', 'audit_real_gather', 'audit_segy_bytes'],
    'experiments': ['audit_joint_studies', 'review_studies', 'review_window_study', 'run_revised_studies', 'compare_masks'],
    'analysis': ['ablation_stats', 'review_diagnostics', 'summarize_review', 'summarize_audit_v5', 'make_audit_reports', 'render_audit_figures'],
    'validation': ['audit_numerics', 'audit_projection', 'audit_preprocessing_isolation', 'run_audit_checks', 'run_fdct_probe', 'run_legacy_codec_checks', 'replay_audit_science', 'verify_audit_summary', 'verify_audit_v5', 'verify_review_results', 'verify_revised_results', 'mu_properties_test'],
    'maintenance': ['audit_change_annotations', 'finalize_audit_delivery', 'freeze_audit_runtime'],
}


def relative(path, base):
    return os.path.relpath(path, base).replace('\\', '/')


def relocate():
    output = ROOT / 'config/path_migrations.json'
    assert not output.exists(), 'migration already recorded'
    tracked = subprocess.check_output(['git', 'ls-files', '-z'], cwd=ROOT).decode().split('\0')
    mapping = {}
    modules = {name: 'research.' + group + '.' + name for group, names in GROUPS.items() for name in names}
    for name in modules:
        mapping['tools/' + name + '.py'] = modules[name].replace('.', '/') + '.py'
    for name in ['test_audit_v5', 'test_review_science', 'test_science_tools']:
        mapping['tools/' + name + '.py'] = 'tests/python/' + name + '.py'
    for name in ['fdct_roundtrip_probe.cpp', 'legacy_codec_regression.cpp']:
        mapping['tools/' + name] = 'tests/native/' + name

    def native_target(old):
        p = Path(old).relative_to('IrrGeometryDesign')
        parts = p.parts
        if parts[0] in ['GeneratedFiles', 'x64', 'Debug', 'Release']:
            return None
        if parts[0] == 'Resources':
            return 'figures/assets/' + p.name
        if parts[0] in ['Curvelet2d', 'Curvelet3d']:
            return 'third_party/curvelet/' + '/'.join(parts)
        if parts[0] == 'baseclass':
            return 'src/native/core/' + '/'.join(parts[1:])
        if parts[0] == 'spssegy':
            return 'src/native/io/sps/' + '/'.join(parts[1:])
        if parts[0] == 'spsshow':
            return 'src/native/gui/survey/' + '/'.join(parts[1:])
        if p.suffix in ['.dll', '.lib', '.def']:
            return 'third_party/legacy-libs/' + p.name
        if p.name == 'IrrGeometryDesign.vcproj':
            return 'build/legacy/' + p.name
        if p.name in ['DataReconstruction.pro', 'DataReconstruction.pri', 'tt.rules']:
            return 'build/legacy/' + p.name
        if p.name in ['datareconstruction.qrc', 'DataReconstruction_zh.ts', 'IrrGeometryDesign_zh.qm']:
            return 'resources/qt/' + p.name
        if p.stem in ['Sampling', 'sample', 'CurveletMiu', 'CalculateMiu', 'Diagmul', 'DiagMulmatrix', 'spsmaskbuilder']:
            module = 'sampling'
        elif p.stem in ['OptimizeMiu', 'adaptivesearch']:
            module = 'optimization'
        elif p.stem in ['ExpRng']:
            module = 'core'
        elif p.stem in ['geometryexperimentengine', 'samplingstudies', 'reviewexperiments', 'auditqueries']:
            module = 'experiments'
        elif p.stem in ['coordinateholdout', 'reconstructionoptions']:
            module = 'reconstruction'
        elif p.stem in ['reviewregression', 'scienceregression']:
            module = 'tests'
        elif p.stem in ['scientificplotter']:
            module = 'plotting'
        elif p.stem in ['spsloadcal', 'gAttrAnalyzeParam']:
            module = 'geometry'
        elif p.stem in ['main', 'datareconstruction']:
            module = 'app'
        elif p.stem == 'qcustomplot':
            return 'third_party/qcustomplot/' + p.name
        else:
            module = 'gui/dialogs' if p.suffix == '.ui' or (ROOT/'IrrGeometryDesign'/p.with_suffix('.ui')).exists() else 'gui/views'
        return 'src/native/' + module + '/' + p.name

    native_files = [p for p in tracked if p.startswith('IrrGeometryDesign/')]
    # Import libraries were supplied outside Git; they are still required by VS2008.
    native_files += [str(p.relative_to(ROOT)).replace('\\', '/') for p in (ROOT/'IrrGeometryDesign').glob('*.lib')]
    for name in native_files:
        dest = native_target(name)
        if dest:
            mapping[name] = dest
    project = ET.parse(ROOT/'IrrGeometryDesign/IrrGeometryDesign.vcproj').getroot()
    old_includes = [ROOT/'IrrGeometryDesign'/p for p in ['', 'baseclass', 'spssegy', 'spsshow', 'Curvelet2d', 'Curvelet3d']]
    native_text = {}
    for old, new in mapping.items():
        p = ROOT/old
        if not old.startswith('IrrGeometryDesign/') or p.suffix not in ['.cpp', '.h', '.hpp', '.inc', '.ui']:
            continue
        # Latin-1 is a byte-preserving transport for ASCII-only include edits.
        # Legacy sources mix encodings; changing their Chinese bytes changes VS2008 behavior.
        encoding = 'latin-1'
        text = p.read_bytes().decode(encoding)
        if p.suffix != '.ui':
            def rewrite(match):
                inc = match[1].replace('\\', '/')
                found = next((candidate.resolve() for base in [p.parent] + old_includes if (candidate := base/inc).is_file()), None)
                if found:
                    key = found.relative_to(ROOT).as_posix()
                    if key in mapping:
                        return '#include "' + relative(ROOT/mapping[key], (ROOT/new).parent) + '"'
                return match[0]
            text = re.sub(r'#include\s*"([^"\n]+)"', rewrite, text)
        else:
            text = re.sub(r'location="([^"]*datareconstruction.qrc)"',
                          'location="' + relative(ROOT/'resources/qt/datareconstruction.qrc', (ROOT/new).parent) + '"', text)
        native_text[new] = (text, encoding)
    # Capture before moving: all native references are resolved against the original tree.
    for old, new in mapping.items():
        source, target = ROOT/old, ROOT/new
        assert source.is_file() and not target.exists(), (old, new)
        target.parent.mkdir(parents=True, exist_ok=True)
        source.rename(target)
    for name, (text, encoding) in native_text.items():
        (ROOT/name).write_bytes(text.encode(encoding))

    # Package imports are explicit; executing a module never depends on sys.path hacks.
    for old, new in mapping.items():
        if not old.startswith('tools/') or not new.endswith('.py'):
            continue
        path = ROOT/new
        text = path.read_text(encoding='utf-8-sig')
        for name, module in sorted(modules.items(), key=lambda x: -len(x[0])):
            text = re.sub(r'(?m)^(\s*)from ' + re.escape(name) + r' import ', r'\1from ' + module + ' import ', text)
            text = re.sub(r'(?m)^(\s*)import ' + re.escape(name) + r'(?: as (\w+))?(\s*(?:#.*)?)$',
                          lambda m: m[1] + 'import ' + module + ' as ' + (m[2] or name) + m[3], text)
        text = re.sub(r'ROOT\s*=\s*Path\(__file__\)\.resolve\(\)\.parents\[1\]', 'from research.common.paths import ROOT', text)
        text = text.replace('Path(__file__).resolve().parents[1]/', 'ROOT/')
        text = text.replace('Path(__file__).resolve().parents[1] /', 'ROOT /')
        if 'ROOT/' in text or 'ROOT /' in text:
            if 'from research.common.paths import ROOT' not in text:
                tree = ast.parse(text)
                start = next((n.lineno - 1 for n in tree.body if isinstance(n, (ast.Import, ast.ImportFrom)) and not (isinstance(n, ast.ImportFrom) and n.module == '__future__')), 1)
                lines = text.splitlines(True);lines.insert(start, 'from research.common.paths import ROOT\n');text = ''.join(lines)
        # Replace explicit active source paths, leaving historical archived manifests unchanged.
        for before, after in sorted(mapping.items(), key=lambda x: -len(x[0])):
            text = text.replace(before, after)
        text = text.replace("ROOT/'x64/", "ROOT/'build/bin/").replace('ROOT / "x64/', 'ROOT / "build/bin/').replace("ROOT/'x64'/", "ROOT/'build/bin'/")
        path.write_text(text, encoding='utf-8')
    for directory in [ROOT/'research', ROOT/'tests/python'] + [ROOT/'research'/x for x in list(GROUPS)+['common']]:
        directory.mkdir(parents=True, exist_ok=True)
        (directory/'__init__.py').write_text('', encoding='utf-8')
    (ROOT/'tests/__init__.py').write_text('', encoding='utf-8')

    qrc = ROOT/'resources/qt/datareconstruction.qrc'
    tree = ET.parse(qrc)
    for element in tree.iter('file'):
        original = element.text
        element.set('alias', original)
        element.text = relative(ROOT/mapping['IrrGeometryDesign/' + original], qrc.parent)
    ET.indent(tree);tree.write(qrc, encoding='utf-8', xml_declaration=True)

    # Keep the tested VS configuration settings, regenerate path-dependent rules.
    project.find('ToolFiles').clear()
    include_dirs = sorted({str((ROOT/p).parent.relative_to(ROOT)).replace('\\', '/') for p in mapping.values() if p.endswith(('.h', '.hpp'))})
    include_dirs += ['third_party/eigen-x64', 'build/generated', 'build/generated/$(ConfigurationName)']
    project_dir = ROOT/'build/legacy'
    include_value = ';'.join('"' + relative(ROOT/p, project_dir).replace('/', '\\') + '"' for p in include_dirs)
    include_value += ';' + ';'.join('"$(QTDIR)\\include' + p + '"' for p in ['', '\\QtCore', '\\QtGui', '\\QtOpenGL', '\\Qt3Support'])
    for config in project.find('Configurations'):
        config.set('OutputDirectory', '$(SolutionDir)build\\bin\\$(ConfigurationName)')
        config.set('IntermediateDirectory', '..\\obj\\$(PlatformName)\\$(ConfigurationName)')
        for tool in config.findall('Tool'):
            if tool.get('Name') == 'VCCLCompilerTool':
                tool.set('AdditionalIncludeDirectories', include_value)
            if tool.get('Name') == 'VCLinkerTool':
                tool.set('AdditionalLibraryDirectories', '$(QTDIR)\\lib;..\\..\\third_party\\legacy-libs')
            if tool.get('Name') == 'VCPreBuildEventTool':
                tool.set('CommandLine', 'if not exist "..\\generated\\$(ConfigurationName)" mkdir "..\\generated\\$(ConfigurationName)"')
    for element in list(project.find('Files').iter('File')):
        old = 'IrrGeometryDesign/' + element.get('RelativePath').replace('\\', '/').removeprefix('./')
        if old in mapping:
            new = mapping[old]
        elif '/GeneratedFiles/' in old:
            new = old.replace('IrrGeometryDesign/GeneratedFiles/', 'build/generated/')
        else:
            # Entries for files never supplied (e.g. GeometryAT1.PNG) are not sources.
            for parent in project.find('Files').iter():
                if element in list(parent):parent.remove(element);break
            continue
        rel = relative(ROOT/new, project_dir).replace('/', '\\')
        element.set('RelativePath', rel)
        for config in element.findall('FileConfiguration'):
            tool = config.find('Tool')
            if tool is None or tool.get('Name') != 'VCCustomBuildTool':continue
            suffix = Path(new).suffix
            if suffix == '.h':
                dest = '..\\generated\\$(ConfigurationName)\\moc_$(InputName).cpp'
                command = '"$(QTDIR)\\bin\\moc.exe" "$(InputPath)" -o "' + dest + '"'
                deps = '$(InputPath);$(QTDIR)\\bin\\moc.exe'
            elif suffix == '.ui':
                dest = '..\\generated\\ui_$(InputName).h'
                command = '"$(QTDIR)\\bin\\uic.exe" "$(InputPath)" -o "' + dest + '"'
                deps = '$(InputPath);$(QTDIR)\\bin\\uic.exe'
            elif suffix == '.qrc':
                dest = '..\\generated\\qrc_datareconstruction.cpp'
                command = '"$(QTDIR)\\bin\\rcc.exe" -name datareconstruction -no-compress "$(InputPath)" -o "' + dest + '"'
                deps = '$(InputPath);' + ';'.join(relative(ROOT/p, project_dir).replace('/', '\\') for p in mapping.values() if p.startswith('figures/assets/'))
            else:continue
            tool.set('CommandLine', command + '\r\n');tool.set('AdditionalDependencies', deps);tool.set('Outputs', dest)
    ET.indent(project)
    ET.ElementTree(project).write(ROOT/'build/legacy/IrrGeometryDesign.vcproj', encoding='Windows-1252', xml_declaration=True)
    solution = ROOT/'IrrGeometryDesign.sln'
    solution.write_bytes(solution.read_bytes().replace(b'IrrGeometryDesign\\IrrGeometryDesign.vcproj', b'build\\legacy\\IrrGeometryDesign.vcproj'))
    output.parent.mkdir(exist_ok=True)
    output.write_text(json.dumps(dict(baseline_commit='c191e6c5',files=mapping,python_modules=modules),indent=2),encoding='utf-8')
    print('Relocated', len(mapping), 'files')


if __name__ == '__main__':
    relocate()
