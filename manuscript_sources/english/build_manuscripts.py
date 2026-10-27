"""Build editable English/Chinese research manuscripts from audited result tables.

Run from any directory: python -B paper-tgrs/build_manuscripts.py
The Markdown sources are authoritative. TeX is exported for XeLaTeX; the local
PDF build uses ReportLab, so a TeX installation is optional.
"""
from __future__ import annotations

import csv
import hashlib
import html
import json
import re
import shutil
from datetime import datetime, timezone
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_JUSTIFY, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (
    BaseDocTemplate, Flowable, Frame, Image, KeepTogether, PageBreak,
    PageTemplate, Paragraph, Preformatted, Spacer, Table, TableStyle,
)
from PIL import Image as PILImage

CANONICAL = Path(__file__).resolve().parent
PROJECT = CANONICAL.parents[1]
ROOT = PROJECT.parent
HERE = ROOT / "paper-tgrs"
THESIS = ROOT / "thesis"
ASSETS = HERE / "assets"
EVIDENCE: dict[str, dict] = {}


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source(path: Path) -> Path:
    if not path.is_file():
        raise FileNotFoundError(f"Required manuscript evidence is absent: {path}")
    EVIDENCE[str(path.relative_to(ROOT)).replace("\\", "/")] = {
        "sha256": sha(path), "bytes": path.stat().st_size,
    }
    return path


def rows(relative: str) -> list[dict]:
    path = source(PROJECT / relative)
    recorded = path.parent / "manifest.json"
    if recorded.is_file():
        state = json.loads(source(recorded).read_text(encoding="utf-8"))
        expected = state.get("table_sha256", state.get("output_tables_sha256", {})).get(path.name)
        if expected is not None and sha(path) != expected:
            raise ValueError("Manuscript table differs from its frozen analysis: " + str(path))
    with path.open(encoding="utf-8-sig", newline="") as stream:
        return list(csv.DictReader(stream))


def number(value, digits=4):
    return f"{float(value):.{digits}f}"


def mdtable(headers, data):
    lines = ["| " + " | ".join(map(str, headers)) + " |",
             "| " + " | ".join("---" for _ in headers) + " |"]
    lines.extend("| " + " | ".join(map(str, row)) + " |" for row in data)
    return "\n".join(lines)


def tables(lang):
    cn = lang == "zh"
    search = rows("results/repair_20261026/analysis/search_means.csv")
    search_index = {(int(r["budget"]), int(r["m"]), r["method"]): r for r in search}
    search_data = [[b, m] + [number(search_index[b, m, method]["mu"])
                           for method in ["Jitter", "SAOnly", "Hybrid", "SA_Budget", "Adaptive"]]
                   for b in [200, 600] for m in [38, 64, 90]]
    syn = rows("results/repair_20261026/analysis/joint_means.csv")
    syn_index = {(int(r["m"]), r["method"], r["recovery"]): r for r in syn}
    syn_data = [[m] + [number(syn_index[m, method, mode]["mean_snr"], 3)
                      for method, mode in [("Jitter", "fixed"), ("Adaptive", "fixed"),
                                           ("Jitter", "selected"), ("Adaptive", "selected")]]
                for m in [38, 64, 90]]
    v1 = rows("results/viking_20261026/analysis/recovery_means.csv")
    v1_index = {(int(r["m"]), r["method"], r["recovery"]): r for r in v1}
    v1_data = [[m] + [number(v1_index[m, method, mode]["mean_snr"])
                     for method, mode in [("Jitter", "fixed"), ("Adaptive", "fixed"),
                                          ("Jitter", "selected"), ("Adaptive", "selected")]]
               for m in [36, 60, 84]]
    v2 = rows("results/viking_v2_20261026/analysis/overall_means.csv")
    v2_index = {(int(r["m"]), r["method"], r["recovery"]): r for r in v2}
    methods = ["Jitter", "SA_Mu", "Adaptive_Mu", "Adaptive_Gap", "Adaptive_Signal"]
    v2_methods = [[m] + [number(v2_index[m, method, "selected"]["mean_snr"])
                         for method in methods] for m in [36, 60, 84]]
    v2_recovery = [[m] + [number(sum(float(v2_index[m, method, mode]["mean_snr"])
                                     for method in methods) / len(methods))
                          for mode in ["linear", "cold", "fk", "selected"]]
                    for m in [36, 60, 84]]
    geometry = rows("results/viking_v2_20261026/analysis/geometry_means.csv")
    gi = {(int(r["m"]), r["method"]): r for r in geometry}
    gap_data = [[m, number(gi[m, "Adaptive_Mu"]["mean_max_gap_m"], 1),
                 number(gi[m, "Adaptive_Gap"]["mean_max_gap_m"], 1),
                 number(float(v2_index[m, "Adaptive_Gap", "selected"]["mean_snr"]) -
                        float(v2_index[m, "Adaptive_Mu", "selected"]["mean_snr"])),
                 number(float(v2_index[m, "Adaptive_Signal", "selected"]["mean_snr"]) -
                        float(v2_index[m, "Adaptive_Gap", "selected"]["mean_snr"]))]
                for m in [36, 60, 84]]
    sps = rows("checks/repair_20261026_sps/real_sps/attribute_evaluation/sps_spatial_sampling_comparison.csv")
    sps_data = [[r["method"], r["receivers"], number(r["spatial_fourier_mu"], 9), r["evaluations"]] for r in sps]
    # Independently derive the tables used in both manuscripts from saved CSVs.
    # No manual favorable-value substitution is accepted by this build.
    result = {
        "search": mdtable(["B", "m", "Jitter", "SAOnly", "Hybrid", "SA_Budget", "Adaptive"], search_data),
        "synthetic": mdtable(["m", "Jitter fixed", "Adaptive fixed", "Jitter selected", "Adaptive selected"], syn_data),
        "viking_v1": mdtable(["m", "Jitter fixed", "Adaptive fixed", "Jitter selected", "Adaptive selected"], v1_data),
        "viking_v2_methods": mdtable(["m"] + methods, v2_methods),
        "viking_v2_recovery": mdtable(["m", "Linear", "Cold curvelet", "Weak f-k", "Inner-selected"], v2_recovery),
        "viking_v2_gap": mdtable(["m", "Mu gap / m", "Gap gap / m", "Gap-Mu / dB", "Signal-Gap / dB"], gap_data),
        "sps": mdtable(["Method", "Receivers", "2D Fourier mu", "Evaluations"], sps_data),
    }
    robust_manifest = PROJECT / "results/viking_robust_20261026/manifest.json"
    if robust_manifest.is_file():
        robust_state = json.loads(robust_manifest.read_text(encoding="utf-8"))
        if robust_state.get("status") == "passed" and robust_state.get("completed"):
            source(robust_manifest)
            robust = rows("results/viking_robust_20261026/analysis/overall_means.csv")
            index = {(int(r["m"]), r["method"], r["recovery"]): r for r in robust}
            result["robust_methods"] = mdtable(["m"] + methods, [
                [m] + [number(index[m, method, "selected"]["mean_snr"]) for method in methods]
                for m in [36, 60, 84]])
            result["robust_recovery"] = mdtable(["m", "Cold curvelet", "Weak f-k", "Inner-selected"], [
                [m] + [number(sum(float(index[m, method, mode]["mean_snr"])
                                 for method in methods)/len(methods)) for mode in ["cold", "fk", "selected"]]
                for m in [36, 60, 84]])
    return result


FIGURES = {
    "search": ("repair_20261026/search.png", "repair_search.png"),
    "budget": ("repair_20261026/budget.png", "repair_budget.png"),
    "synthetic": ("repair_20261026/recovery.png", "repair_recovery.png"),
    "viking_v1": ("viking_20261026/recovery.png", "viking_v1_recovery.png"),
    "viking_v2": ("viking_v2_20261026/recovery.png", "viking_v2_recovery.png"),
    "viking_geometry": ("viking_v2_20261026/geometry.png", "viking_v2_geometry.png"),
    "viking_conditions": ("viking_v2_20261026/conditions.png", "viking_v2_conditions.png"),
    "robust_waveform": ("viking_robust_waveforms_20261026/waveform.png", "viking_robust_waveform.png"),
    "f3_recovery": ("f3_transfer_20261027/recovery.png", "f3_transfer_recovery.png"),
}


def prepare_assets():
    ASSETS.mkdir(exist_ok=True)
    for relative, target in FIGURES.values():
        src = source(PROJECT / "figures/research" / relative)
        shutil.copy2(src, ASSETS / target)
    robust = PROJECT / "figures/research/viking_robust_20261026/recovery.png"
    if robust.is_file():
        shutil.copy2(source(robust), ASSETS / "viking_robust_recovery.png")


def references():
    return json.loads(source(HERE / "references.json").read_text(encoding="utf-8"))


def expand(path, lang):
    text = source(path).read_text(encoding="utf-8")
    robust_source = HERE / ("robustness_zh.md" if lang == "zh" else "robustness.md")
    text = text.replace("{{robustness}}", source(robust_source).read_text(encoding="utf-8"))
    text = text.replace("{{external_and_quality}}", source(CANONICAL / ("external_quality_zh.md" if lang == "zh" else "external_quality.md")).read_text(encoding="utf-8"))
    external = rows("results/f3_transfer_20261027/means.csv")
    methods = ['Jitter','SA_Mu','Adaptive_Mu','Adaptive_Gap','Adaptive_Signal']
    index = {(int(r['m']),r['method'],r['recovery']):r for r in external}
    text = text.replace('{{external_methods}}',mdtable(['m']+methods,
        [[m]+[number(index[m,method,'selected']['mean_snr']) for method in methods] for m in (36,60,84)]))
    text = text.replace('{{external_recovery}}',mdtable(['m','Cold curvelet','Weak f-k','Inner-selected','Linear'],
        [[m]+[number(sum(float(index[m,method,kind]['mean_snr']) for method in methods)/5) for kind in ('cold','fk','selected','linear')] for m in (36,60,84)]))
    waveform = rows('results/viking_quality_20261027/waveform_means.csv')
    diagnostics = []
    for m in (36,60,84):
        for kind in ('cold','fk','selected'):
            same=[r for r in waveform if int(r['m'])==m and r['recovery']==kind]
            diagnostics.append([m,kind]+[number(sum(float(r[key]) for r in same)/len(same),3)
                for key in ('mean_rms_ratio','mean_centered_waveform_correlation','mean_band_phase_mae_degrees')])
    text = text.replace('{{waveform_quality}}',mdtable(['m','Recovery','RMS ratio','Correlation','Phase MAE (deg)'],diagnostics))
    for key, table in tables(lang).items():
        text = text.replace("{{table:" + key + "}}", table)
    prefix = "assets/" if lang == "en" else "../paper-tgrs/assets/"
    for key, (_, name) in FIGURES.items():
        text = text.replace("{{figure:" + key + "}}", prefix + name)
    refs = references()
    ref_text = "\n\n".join(f"[{i}] {r['text']} [DOI/source]({r['url']})." for i, r in enumerate(refs, 1))
    text = text.replace("{{references}}", ref_text)
    lookup = {r["key"]: str(i) for i, r in enumerate(refs, 1)}
    text = re.sub(r"\[@([a-z0-9_-]+)\]", lambda m: "[" + lookup[m[1]] + "]", text)
    if "{{" in text:
        raise ValueError(f"Unresolved manuscript macro in {path}")
    return text


def inline(text, font="Body"):
    text = html.escape(text)
    text = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", lambda m: f'<a href="{m[2]}" color="#244e76">{m[1]}</a>', text)
    text = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", text)
    text = re.sub(r"`([^`]+)`", lambda m: f'<font name="Mono">{m[1]}</font>', text)
    return text


def register_fonts():
    font_dir = Path("C:/Windows/Fonts")
    pdfmetrics.registerFont(TTFont("Body", str(font_dir / "times.ttf")))
    pdfmetrics.registerFont(TTFont("BodyBold", str(font_dir / "timesbd.ttf")))
    pdfmetrics.registerFont(TTFont("BodyItalic", str(font_dir / "timesi.ttf")))
    pdfmetrics.registerFont(TTFont("Mono", str(font_dir / "consola.ttf")))
    pdfmetrics.registerFont(TTFont("CJK", str(font_dir / "STSONG.TTF")))
    pdfmetrics.registerFont(TTFont("CJKHead", str(font_dir / "simhei.ttf")))
    pdfmetrics.registerFontFamily("Body", normal="Body", bold="BodyBold", italic="BodyItalic", boldItalic="BodyBold")
    pdfmetrics.registerFontFamily("CJK", normal="CJK", bold="CJKHead", italic="CJK", boldItalic="CJKHead")


class ResearchDoc(BaseDocTemplate):
    def __init__(self, path, lang, title):
        self.lang = lang
        self.short_title = title
        super().__init__(str(path), pagesize=A4, leftMargin=21*mm,
                         rightMargin=21*mm, topMargin=23*mm, bottomMargin=22*mm,
                         title=title, author="Wu Shanzhou / 吴善州")
        self.lang = lang
        frame = Frame(self.leftMargin, self.bottomMargin, self.width, self.height,
                      leftPadding=0, rightPadding=0, topPadding=0, bottomPadding=0)
        self.addPageTemplates(PageTemplate(id="main", frames=frame, onPage=self.page_chrome))

    def page_chrome(self, canvas, doc):
        canvas.saveState()
        canvas.setStrokeColor(colors.HexColor("#bbcad8"))
        canvas.setLineWidth(.5)
        canvas.line(self.leftMargin, A4[1]-16*mm, A4[0]-self.rightMargin, A4[1]-16*mm)
        canvas.setFont("CJK" if self.lang == "zh" else "Body", 8)
        canvas.setFillColor(colors.HexColor("#526579"))
        label = "地震采样点筛选研究初稿 · 2026-10-27" if self.lang == "zh" else "Research draft | 27 October 2026 | Not submitted"
        canvas.drawString(self.leftMargin, A4[1]-13*mm, label)
        canvas.drawString(self.leftMargin, 12*mm, "可编辑研究初稿" if self.lang == "zh" else "Budget-aware trace selection")
        canvas.drawRightString(A4[0]-self.rightMargin, 12*mm, str(doc.page))
        canvas.restoreState()

    def afterFlowable(self, flowable):
        if isinstance(flowable, Paragraph) and flowable.style.name in ("H1", "H2"):
            level = 0 if flowable.style.name == "H1" else 1
            key = "heading-%s" % self.seq.nextf("outline")
            self.canv.bookmarkPage(key)
            self.canv.addOutlineEntry(flowable.getPlainText(), key, level=level, closed=False)


def pdf_from_markdown(text, path, lang, basedir):
    cn = lang == "zh"
    body_font = "CJK" if cn else "Body"
    heading_font = "CJKHead" if cn else "BodyBold"
    normal = ParagraphStyle("Normal", fontName=body_font, fontSize=10.5,
                            leading=17 if cn else 14.7, spaceAfter=7.5,
                            alignment=TA_JUSTIFY, wordWrap="CJK" if cn else None)
    styles = {
        "body": normal,
        "title": ParagraphStyle("Title", parent=normal, fontName=heading_font, fontSize=23 if cn else 20,
                                leading=31 if cn else 25, alignment=TA_LEFT, spaceAfter=15),
        "h1": ParagraphStyle("H1", parent=normal, fontName=heading_font, fontSize=16 if cn else 14,
                             leading=23 if cn else 19, spaceBefore=15, spaceAfter=9, keepWithNext=True),
        "h2": ParagraphStyle("H2", parent=normal, fontName=heading_font, fontSize=12,
                             leading=18, spaceBefore=11, spaceAfter=6, keepWithNext=True),
        "caption": ParagraphStyle("Caption", parent=normal, fontSize=8.7, leading=12.5,
                                  textColor=colors.HexColor("#38516a"), spaceBefore=5, spaceAfter=11),
        "tablecaption": ParagraphStyle("TableCaption", parent=normal, fontSize=9.3,
                                       leading=13, keepWithNext=True, spaceBefore=5, spaceAfter=7),
        "table": ParagraphStyle("Table", parent=normal, fontSize=8.2 if not cn else 8.7,
                                leading=11.7 if not cn else 13, spaceAfter=0,
                                alignment=TA_LEFT, wordWrap="CJK" if cn else None),
        "code": ParagraphStyle("Code", fontName="CJK" if cn else "Mono", fontSize=8.1, leading=11.4,
                               leftIndent=5, backColor=colors.HexColor("#f2f5f8"),
                               borderPadding=6, spaceBefore=5, spaceAfter=9),
    }
    lines = text.splitlines()
    story, i = [], 0
    width = A4[0]-42*mm
    while i < len(lines):
        line = lines[i].strip()
        if not line:
            i += 1; continue
        if line == "<!-- pagebreak -->":
            story.append(PageBreak()); i += 1; continue
        if line.startswith("<!--"):
            i += 1; continue
        if line.startswith("```"):
            code=[]; i+=1
            while i < len(lines) and not lines[i].startswith("```"):
                code.append(lines[i]); i+=1
            story.append(Preformatted("\n".join(code), styles["code"], maxLineLength=91))
            i+=1; continue
        if line.startswith("|"):
            data=[]
            while i < len(lines) and lines[i].strip().startswith("|"):
                cells=[x.strip() for x in lines[i].strip().strip("|").split("|")]
                if not all(re.fullmatch(r":?-+:?", c) for c in cells):
                    data.append([Paragraph(inline(c), styles["table"]) for c in cells])
                i+=1
            ncols=len(data[0])
            weights=[1.0]*ncols
            if ncols>=5:
                weights[0]=.55
                if ncols==6 and any("Adaptive" in p.getPlainText() for p in data[0]):
                    weights=[.5,1,1,1.15,1.2,1.3]
            colwidths=[width*w/sum(weights) for w in weights]
            table=Table(data, colWidths=colwidths, repeatRows=1, hAlign="LEFT")
            table.setStyle(TableStyle([
                ("BACKGROUND",(0,0),(-1,0),colors.HexColor("#e5edf4")),
                ("ROWBACKGROUNDS",(0,1),(-1,-1),[colors.white, colors.HexColor("#f5f7fa")]),
                ("LINEBELOW",(0,0),(-1,0),.7,colors.HexColor("#8198ad")),
                ("LINEBELOW",(0,-1),(-1,-1),.5,colors.HexColor("#b6c4d1")),
                ("VALIGN",(0,0),(-1,-1),"TOP"),
                ("LEFTPADDING",(0,0),(-1,-1),5), ("RIGHTPADDING",(0,0),(-1,-1),5),
                ("TOPPADDING",(0,0),(-1,-1),6), ("BOTTOMPADDING",(0,0),(-1,-1),6),
            ]))
            story.extend([table, Spacer(1,10)]); continue
        match=re.fullmatch(r"!\[([^]]*)\]\(([^)]+)\)",line)
        if match:
            img_path=(basedir/match[2]).resolve()
            with PILImage.open(img_path) as picture:
                iw,ih=picture.size
            scale=min(width/iw,105*mm/ih)
            img=Image(str(img_path),width=iw*scale,height=ih*scale)
            cap=Paragraph(inline(match[1]),styles["caption"])
            story.append(KeepTogether([Spacer(1,6),img,cap])); i+=1; continue
        heading=re.match(r"^(#{1,3}) (.+)$",line)
        if heading:
            level=len(heading[1]); value=heading[2]
            if cn and level==2 and value.startswith("第"):
                story.append(PageBreak())
            story.append(Paragraph(inline(value),styles[{1:"title",2:"h1",3:"h2"}[level]]))
            i+=1; continue
        if line.startswith("- "):
            story.append(Paragraph("• "+inline(line[2:]), normal)); i+=1; continue
        para=[line]; i+=1
        while i<len(lines) and lines[i].strip() and not re.match(r"^(#|\||!\[|```|<!--|- )",lines[i].strip()):
            para.append(lines[i].strip()); i+=1
        value="".join(para) if cn else " ".join(para)
        style = styles["tablecaption"] if re.match(r"^\*\*(?:(?:Supplementary )?Table\s+S?\d|(?:补充)?表S?\d)", value) else normal
        story.append(Paragraph(inline(value), style))
    ResearchDoc(path, lang, "地震采样点智能筛选方法研究" if cn else "Budget-Aware Seismic Trace Selection").build(story)


def tex_escape(text):
    replacements = {"\\":r"\textbackslash{}", "&":r"\&", "%":r"\%", "$":r"\$", "#":r"\#", "_":r"\_", "{":r"\{", "}":r"\}", "~":r"\textasciitilde{}", "^":r"\textasciicircum{}"}
    return "".join(replacements.get(c,c) for c in text)


def tex_inline(text):
    # Keep numeric citations and DOI links identical to the expanded Markdown.
    parts=re.split(r"(\[[^\]]+\]\([^)]+\)|\*\*[^*]+\*\*|`[^`]+`)",text)
    output=[]
    for part in parts:
        link=re.fullmatch(r"\[([^]]+)\]\(([^)]+)\)",part)
        if link:
            output.append(r"\href{"+tex_escape(link[2])+"}{"+tex_escape(link[1])+"}")
        elif part.startswith("**"):
            output.append(r"\textbf{"+tex_escape(part[2:-2])+"}")
        elif part.startswith("`"):
            output.append(r"\texttt{"+tex_escape(part[1:-1])+"}")
        else:
            output.append(tex_escape(part))
    return "".join(output)


def export_tex(text, path, lang):
    cn=lang=="zh"
    header = r"""% Generated from the adjacent authoritative Markdown. Compile with XeLaTeX.
\documentclass[11pt,a4paper]{""" + ("ctexart" if cn else "article") + r"""}
\usepackage[margin=22mm]{geometry}
\usepackage{fontspec}
\usepackage{graphicx,longtable,booktabs,array,hyperref,fancyvrb}
\setmainfont{Times New Roman}
\setmonofont{Consolas}
\hypersetup{colorlinks=true,urlcolor=blue,linkcolor=blue}
\setlength{\parindent}{0pt}
\setlength{\parskip}{0.6em}
\emergencystretch=3em
\begin{document}
"""
    lines=text.splitlines(); out=[header]; i=0
    while i<len(lines):
        line=lines[i].strip()
        if not line:
            out.append(""); i+=1; continue
        if line.startswith("<!--"):
            if "pagebreak" in line: out.append(r"\clearpage")
            i+=1; continue
        if line.startswith("```"):
            out.append(r"\begin{Verbatim}[fontsize=\small]"); i+=1
            while i<len(lines) and not lines[i].startswith("```"):
                out.append(lines[i]); i+=1
            out.append(r"\end{Verbatim}"); i+=1; continue
        if line.startswith("|"):
            data=[]
            while i<len(lines) and lines[i].strip().startswith("|"):
                cells=[x.strip() for x in lines[i].strip().strip("|").split("|")]
                if not all(re.fullmatch(r":?-+:?",c) for c in cells): data.append(cells)
                i+=1
            n=len(data[0]); frac=.82/n
            out.append(r"{\small\begin{longtable}{"+"".join(r">{\raggedright\arraybackslash}p{"+f"{frac:.3f}"+r"\linewidth}" for _ in range(n))+"}")
            out.append(r"\toprule")
            for row_idx,row in enumerate(data):
                out.append(" & ".join(tex_inline(c) for c in row)+r" \\")
                if row_idx==0: out.append(r"\midrule\endhead")
            out.append(r"\bottomrule\end{longtable}}")
            continue
        image_match=re.fullmatch(r"!\[([^]]*)\]\(([^)]+)\)",line)
        if image_match:
            out.extend([r"\begin{figure}[htbp]\centering",
                        r"\includegraphics[width=\linewidth,height=.34\textheight,keepaspectratio]{"+image_match[2]+"}",
                        r"\caption{"+tex_inline(image_match[1])+"}",r"\end{figure}"])
            i+=1; continue
        heading=re.match(r"^(#{1,3}) (.+)$",line)
        if heading:
            command={1:"section*",2:"section*",3:"subsection*"}[len(heading[1])]
            if len(heading[1])==1:
                out.append(r"{\LARGE\bfseries "+tex_inline(heading[2])+r"\par}\vspace{1em}")
            else:
                out.append("\\"+command+"{"+tex_inline(heading[2])+"}")
            i+=1; continue
        out.append(tex_inline(line[2:] if line.startswith("- ") else line)+("\\par" if line.startswith("- ") else "")); i+=1
    out.append(r"\end{document}")
    path.write_text("\n".join(out),encoding="utf-8")


def export_bib(refs):
    chunks=[]
    for r in refs:
        values={k:v for k,v in r.items() if k in ["author","title","journal","year","volume","number","pages","doi","url","publisher"]}
        chunks.append("@"+r.get("type","article")+"{"+r["key"]+",\n"+",\n".join("  "+k+" = {"+v+"}" for k,v in values.items())+"\n}")
    (HERE/"references.bib").write_text("\n\n".join(chunks)+"\n",encoding="utf-8")


def main():
    # Versioned master sources live inside the project. Neighboring folders are
    # regenerated compatibility outputs for existing manuscript links.
    HERE.mkdir(exist_ok=True)
    THESIS.mkdir(exist_ok=True)
    for name in ('manuscript.md','robustness.md','robustness_zh.md','references.json','reference_audit.json','requirements.txt','README.md'):
        shutil.copy2(source(CANONICAL/name),HERE/name)
    for name in ('thesis.md','README.md'):
        shutil.copy2(source(CANONICAL.parent/'chinese'/name),THESIS/name)
    source(Path(__file__))
    source(HERE / "requirements.txt")
    source(HERE / "reference_audit.json")
    THESIS.mkdir(exist_ok=True)
    prepare_assets()
    register_fonts()
    refs=references()
    export_bib(refs)
    results=[]
    for src,lang,base,pdf_name,tex_name in [
        (HERE/"manuscript.md","en",HERE,"tgrs_trace_selection.pdf","tgrs_trace_selection.tex"),
        (THESIS/"thesis.md","zh",THESIS,"thesis.pdf","thesis.tex"),
    ]:
        text=expand(src,lang)
        expanded=base/("manuscript_expanded.md" if lang=="en" else "thesis_expanded.md")
        expanded.write_text(text,encoding="utf-8")
        export_tex(text,base/tex_name,lang)
        candidate=base/(pdf_name+".new")
        pdf_from_markdown(text,candidate,lang,base)
        from pypdf import PdfReader
        reader=PdfReader(candidate)
        extracted="\n".join(page.extract_text() or "" for page in reader.pages)
        if len(reader.pages)<5 or len(extracted)<8000:
            raise ValueError("Manuscript PDF failed basic completeness checks")
        if "Viking" not in extracted:
            raise ValueError("Current real-data evidence absent from PDF")
        candidate.replace(base/pdf_name)
        results.append({"pdf":str((base/pdf_name).relative_to(ROOT)).replace("\\","/"),
                        "pages":len(reader.pages),"extracted_characters":len(extracted),
                        "sha256":sha(base/pdf_name),"markdown_sha256":sha(src),
                        "tex_export":"generated; XeLaTeX is not installed and TeX compilation was not executed",
                        "pdf_engine":"ReportLab; embedded TrueType fonts"})
    result={"built_utc":datetime.now(timezone.utc).isoformat(),"evidence":EVIDENCE,"outputs":results,
            "table_policy":"Tables regenerated from cited CSVs; development conditions are not independent surveys.",
            "status":"built; visual QA recorded separately"}
    (HERE/"build_manifest.json").write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding="utf-8")
    print(json.dumps({"outputs":results,"evidence_files":len(EVIDENCE)},ensure_ascii=False,indent=2))


if __name__=="__main__":
    main()
