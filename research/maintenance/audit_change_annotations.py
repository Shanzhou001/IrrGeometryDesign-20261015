"""Apply/check dated author comments on changed code relative to the previous baseline."""  # add by wushanzhou in 2026/10/23
import argparse  # add by wushanzhou in 2026/10/23
from datetime import date  # add by wushanzhou in 2026/10/24
import ast  # add by wushanzhou in 2026/10/23
import difflib  # add by wushanzhou in 2026/10/23
import io  # add by wushanzhou in 2026/10/23
import json  # add by wushanzhou in 2026/10/23
from pathlib import Path  # add by wushanzhou in 2026/10/23
import re  # add by wushanzhou in 2026/10/23
import subprocess  # add by wushanzhou in 2026/10/23
import tokenize  # add by wushanzhou in 2026/10/23

from research.common.paths import ROOT  # add by wushanzhou in 2026/10/23
MARKER = f"add by wushanzhou in {date.today():%Y/%m/%d}"  # add by wushanzhou in 2026/10/24
SUFFIXES = {".cpp", ".h", ".inc", ".py", ".bat", ".ui"}  # add by wushanzhou in 2026/10/23


def cpp_end_states(lines):  # add by wushanzhou in 2026/10/23
    state = "code"; result = []  # add by wushanzhou in 2026/10/23
    for line in lines:  # add by wushanzhou in 2026/10/23
        i = 0  # add by wushanzhou in 2026/10/23
        while i < len(line):  # add by wushanzhou in 2026/10/23
            c = line[i]; pair = line[i:i+2]  # add by wushanzhou in 2026/10/23
            if state == "block":  # add by wushanzhou in 2026/10/23
                if pair == "*/": state = "code"; i += 2; continue  # add by wushanzhou in 2026/10/23
            elif state in ('"', "'"):  # add by wushanzhou in 2026/10/23
                if c == "\\": i += 2; continue  # add by wushanzhou in 2026/10/23
                if c == state: state = "code"  # add by wushanzhou in 2026/10/23
            elif pair == "//": break  # add by wushanzhou in 2026/10/23
            elif pair == "/*": state = "block"; i += 2; continue  # add by wushanzhou in 2026/10/23
            elif c in ('"', "'"): state = c  # add by wushanzhou in 2026/10/23
            i += 1  # add by wushanzhou in 2026/10/23
        result.append(state)  # add by wushanzhou in 2026/10/23
    return result  # add by wushanzhou in 2026/10/23


def annotate(path, old, text):  # add by wushanzhou in 2026/10/23
    lines = text.split("\n")  # Preserve legacy byte 0x85 inside comments. add by wushanzhou in 2026/10/23
    if lines and not lines[-1]: lines.pop()  # add by wushanzhou in 2026/10/23
    selected = set()  # add by wushanzhou in 2026/10/23
    previous_lines = old.split("\n")  # add by wushanzhou in 2026/10/23
    if previous_lines and not previous_lines[-1]: previous_lines.pop()  # add by wushanzhou in 2026/10/23
    for op, a, b, c, d in difflib.SequenceMatcher(None, previous_lines, lines, autojunk=False).get_opcodes():  # add by wushanzhou in 2026/10/23
        if op != "equal": selected.update(range(c, d) if c < d else [min(c, len(lines)-1)])  # add by wushanzhou in 2026/10/23
    deferred = {}  # add by wushanzhou in 2026/10/23
    if path.suffix == ".py":  # add by wushanzhou in 2026/10/23
        before = ast.dump(ast.parse(text))  # add by wushanzhou in 2026/10/23
        for token in tokenize.generate_tokens(io.StringIO(text).readline):  # add by wushanzhou in 2026/10/23
            if token.type == tokenize.STRING and token.start[0] != token.end[0]:  # add by wushanzhou in 2026/10/23
                for i in range(token.start[0]-1, token.end[0]-1): deferred[i] = token.end[0]-1  # add by wushanzhou in 2026/10/23
        states = None  # add by wushanzhou in 2026/10/23
    else: states = cpp_end_states(lines) if path.suffix in (".cpp", ".h", ".inc") else None  # add by wushanzhou in 2026/10/23
    marked = 0; covered = 0; insertions = {}  # add by wushanzhou in 2026/10/23
    for original in sorted(selected):  # add by wushanzhou in 2026/10/23
        if original < 0 or not lines[original].strip(): continue  # add by wushanzhou in 2026/10/23
        if re.search(r"add by wushanzhou in \d{4}[/.\-]\d{1,2}[/.\-]\d{1,2}", lines[original]): covered += 1; continue  # add by wushanzhou in 2026/10/23
        i = deferred.get(original, original)  # add by wushanzhou in 2026/10/23
        if path.suffix == ".ui": continue  # Existing XML comments are renamed separately.  # add by wushanzhou in 2026/10/23
        while i+1 < len(lines) and (lines[i].rstrip().endswith(("\\", "^")) or (states and states[i] in ('"', "'"))): i += 1  # add by wushanzhou in 2026/10/23
        if re.search(r"add by wushanzhou in \d{4}[/.\-]\d{1,2}[/.\-]\d{1,2}", lines[i]): covered += 1; continue  # add by wushanzhou in 2026/10/24
        if path.suffix == ".bat" or path.name == ".gitignore":  # add by wushanzhou in 2026/10/23
            if i+1 < len(lines) and re.fullmatch(r"(?:rem|#) add by wushanzhou in \d{4}[/.\-]\d{1,2}[/.\-]\d{1,2}", lines[i+1].strip()):  # add by wushanzhou in 2026/10/24
                covered += 1; continue  # add by wushanzhou in 2026/10/23
            # add by wushanzhou in 2026/10/23
            insertions[i] = ("rem " if path.suffix == ".bat" else "# ")+MARKER  # add by wushanzhou in 2026/10/23
        else:  # add by wushanzhou in 2026/10/23
            lines[i] = lines[i].rstrip()+("  # " if path.suffix == ".py" or path.name == "requirements.txt" else " // ")+MARKER  # add by wushanzhou in 2026/10/23
        marked += 1  # add by wushanzhou in 2026/10/23
    output = []  # add by wushanzhou in 2026/10/23
    for i, line in enumerate(lines):  # add by wushanzhou in 2026/10/23
        output.append(line)  # add by wushanzhou in 2026/10/23
        if i in insertions: output.append(insertions[i])  # add by wushanzhou in 2026/10/23
    result = "\n".join(output)+("\n" if text.endswith("\n") else "")  # add by wushanzhou in 2026/10/23
    if path.suffix == ".py": assert before == ast.dump(ast.parse(result)), path  # add by wushanzhou in 2026/10/23
    return result, marked, covered  # add by wushanzhou in 2026/10/23


def run(base, apply):  # add by wushanzhou in 2026/10/23
    tracked = subprocess.check_output(["git", "ls-files", "-z"], cwd=ROOT).decode().split("\0")  # add by wushanzhou in 2026/10/23
    names = {p for p in tracked if p and (Path(p).suffix in SUFFIXES or p in (".gitignore", "requirements.txt"))  # add by wushanzhou in 2026/10/23
             and not any(part in Path(p).parts for part in ("GeneratedFiles", "Curvelet2d", "Curvelet3d", "third_party"))}  # add by wushanzhou in 2026/10/23
    names.update(str(p.relative_to(ROOT)).replace("\\", "/") for p in (ROOT / "research").rglob("*.py"))  # add by wushanzhou in 2026/10/23
    report = {"base_commit": base, "annotation_date": f"{date.today():%Y/%m/%d}", "files": []}  # add by wushanzhou in 2026/10/24
    for name in sorted(names):  # add by wushanzhou in 2026/10/23
        path = ROOT / name  # add by wushanzhou in 2026/10/23
        raw = path.read_bytes()  # add by wushanzhou in 2026/10/23
        try:  # add by wushanzhou in 2026/10/23
            text = raw.decode("utf-8-sig"); encoding = "utf-8"  # add by wushanzhou in 2026/10/23
        except UnicodeDecodeError:  # add by wushanzhou in 2026/10/23
            text = raw.decode("latin1"); encoding = "latin1"  # Preserve legacy/mixed bytes exactly. add by wushanzhou in 2026/10/23
        text = text.replace("\r\n", "\n")  # add by wushanzhou in 2026/10/23
        # add by wushanzhou in 2026/10/23
        renamed, count = re.subn(r"add\s+by\s+shanzhou\b", "add by wushanzhou in 2026/10/23", text, flags=re.I)
        renamed, undated = re.subn(r"add\s+by\s+wushanzhou\b(?!\s+in\b)", MARKER, renamed, flags=re.I)  # add by wushanzhou in 2026/10/23
        old_raw = subprocess.run(["git", "show", base+":"+name], cwd=ROOT, capture_output=True).stdout  # add by wushanzhou in 2026/10/23
        try:  # add by wushanzhou in 2026/10/23
            old = old_raw.decode("utf-8-sig")  # add by wushanzhou in 2026/10/23
        except UnicodeDecodeError:  # add by wushanzhou in 2026/10/23
            old = old_raw.decode("latin1")  # add by wushanzhou in 2026/10/23
        old = old.replace("\r\n", "\n")  # add by wushanzhou in 2026/10/23
        # add by wushanzhou in 2026/10/23
        result, marked, covered = annotate(path, old, renamed)  # add by wushanzhou in 2026/10/23
        changed = result != text  # add by wushanzhou in 2026/10/23
        if changed and apply:  # add by wushanzhou in 2026/10/23
            newline = "\r\n" if path.suffix in (".cpp", ".h", ".inc", ".ui", ".bat") or b"\r\n" in raw else "\n"  # add by wushanzhou in 2026/10/23
            bom = b"\xef\xbb\xbf" if raw.startswith(b"\xef\xbb\xbf") else b""  # add by wushanzhou in 2026/10/23
            path.write_bytes(bom+result.replace("\n", newline).encode(encoding))  # add by wushanzhou in 2026/10/23
        if count or undated or marked or covered:  # add by wushanzhou in 2026/10/23
            report["files"].append({"path": name, "renamed_markers": count, "dated_undated_markers": undated,  # add by wushanzhou in 2026/10/23
                                    "new_annotations": marked, "already_annotated_changed_lines": covered,  # add by wushanzhou in 2026/10/23
                                    "author_markers_present": len(re.findall(r"add by wushanzhou in \d", result)), "needs_annotation": changed and not apply})  # add by wushanzhou in 2026/10/23
    if apply:  # add by wushanzhou in 2026/10/23
        target = ROOT / "docs/CHANGE_ANNOTATIONS_20261023.json"  # add by wushanzhou in 2026/10/23
        target.write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")  # add by wushanzhou in 2026/10/23
    missing = [r["path"] for r in report["files"] if r["needs_annotation"]]  # add by wushanzhou in 2026/10/23
    print(json.dumps({"files": len(report["files"]), "renamed": sum(r["renamed_markers"] for r in report["files"]),  # add by wushanzhou in 2026/10/23
                      "new_annotations": sum(r["new_annotations"] for r in report["files"]), "pending": missing if not apply else []}))  # add by wushanzhou in 2026/10/23
    return bool(missing) and not apply  # add by wushanzhou in 2026/10/23


if __name__ == "__main__":  # add by wushanzhou in 2026/10/23
    parser = argparse.ArgumentParser(description=__doc__)  # add by wushanzhou in 2026/10/23
    parser.add_argument("--base", default="63d1408")  # add by wushanzhou in 2026/10/23
    parser.add_argument("--apply", action="store_true")  # add by wushanzhou in 2026/10/23
    args = parser.parse_args()  # add by wushanzhou in 2026/10/23
    raise SystemExit(run(args.base, args.apply))  # add by wushanzhou in 2026/10/23
# add by wushanzhou in 2026/10/23
