# 8 页 TGRS 格式英文论文稿

题目：**Budget-Aware Hybrid Seismic Trace Selection: Separating Search Efficiency From Reconstruction Quality**。

作者：**Wu shanzhou**（按用户提供的拼写）。当前为研究稿，尚未投稿，不虚构单位、邮箱、ORCID、资助、公开仓库地址或数据分发许可。

## 交付文件

| 文件 | 用途 |
| --- | --- |
| [论文 PDF](../../output/pdf/tgrs_trace_selection.pdf) | 英文、IEEEtran journal 双栏、连续页码；严格检查八页。 |
| [LaTeX 源稿](tgrs_trace_selection.tex) | 公式、算法、协议、结果、讨论、数据声明、致谢和九篇真实文献。 |
| [生成的结果表](generated_tables.tex) | 构建脚本从冻结表和独立复算生成，避免手填结果。 |
| [PDF 构建记录](../../output/pdf/tgrs_trace_selection.build.json) | 实际命令、编译器/输入/PDF SHA、页数、引用和文本边界检查。 |
| [英文图件](../../figures/paper/index.html) | 四张从归档数据生成的 PNG，统一纳入项目图片管理。 |
| [独立统计复算](../../results/paper_20261025/evidence/recomputed_statistics.json) | 原始表哈希、30 项均值/CI、24 项配对统计、v4 核心对照、真实负结果。 |
| [方法核对](METHODS_REVIEW_NOTES.md) | 方法与实际源码的一致性及需要披露的科学限制。 |
| [本轮验证](VALIDATION_20261025.md) | 新执行的构建/回归/独立检查；明确保留的失败与未执行事项。 |
| [项目进展与问题修复](../PAPER_PROGRESS_20261025.md) | 详细进度、已修材料问题、核心结论及投稿前缺口。 |

## 复现步骤

在项目根目录、既有 Python 环境下执行：

```powershell
# 只读复核历史统计；输出必须为新路径，拒绝覆盖证据。
python docs/paper/recompute_evidence.py --out results/paper_stats_recheck.json

# 统一重绘全部规范图片，包含四张英文论文图；每个图 ID 每遍仅保存一次。
python pipeline.py figures

# 编译并检查论文，最终 PDF 覆盖规范位置；每次构建尝试另存证据。
python docs/paper/build_paper.py --compiler C:/path/to/tectonic.exe
```

统计脚本使用 NumPy。绘图沿用项目 Matplotlib/Pillow 环境。PDF 构建脚本额外使用 `pypdf` 和 `PyMuPDF`，可通过本目录的 `requirements.txt` 安装。

编译器为 [Tectonic 官方发行版](https://github.com/tectonic-typesetting/tectonic/releases/tag/tectonic%400.17.0) 0.17.0 Windows x64，位于用户缓存，未替换原 C++/Qt 工具链。脚本优先使用 `--compiler`，其次查找 PATH，最后查找本机用户缓存。初次编译需联网获取 TeX 资源；之后可使用缓存。LaTeX 使用官方 IEEEtran journal 类，不通过缩小字体、裁掉页面或补空白页凑八页。

每次编译的原始日志、辅助文件、提取文本及检查 JSON 位于 `results/paper_20261025/manuscript_build/<UTC时间>/`。失败尝试保留。最终质量检查另见 `results/paper_20261025/pdf_quality.json`。

运行 `python docs/paper/check_pdf.py` 可重新检查最终 PDF 并渲染预览；预览是用户缓存中的临时检查产物，不在项目各目录保存图片副本。只有实际查看全部最新页面后，才使用 `--record-visual-review` 登记视觉检查；该检查不代替作者的科学审稿。正文及数学使用 Times 风格字体，全部字体嵌入；未修改 IEEEtran 的正常字号或页边距。

## 数据和统计解释

1. v4 与 v5 是独立协议，不能拼成一个更大的配对样本。
2. 1080 是联合恢复记录数；六个干净结构模板才是所用模型层单位，两噪声和多掩码不增加独立地下结构。
3. SNR 的合成参考包含预加噪声，不是无噪去噪精度。
4. Fourier 几何 μ 不是实际曲波观测字典的互干度；降低 μ 不提供本恢复器的普遍保证。
5. 相同预算上限、相同消耗评价次数和相同墙钟是三个不同比较。
6. 小样本精确检验分辨率限制了 Holm 检出；未显著不能解释成效果为零。未校正 bootstrap CI 也不能代替校正检验。
7. 真实 RRAW 仍只有一个来源，编码、几何比例和资料语义未被来源方确认，全部真实结果是条件性诊断。

## TGRS 要求与当前稿件边界

已核对 [TGRS 官方作者须知](https://www.grss-ieee.org/publications/author-resources/tgrs-information-for-authors/)：英文、IEEE 标准双栏、单倍行距、连续页码；八页是本次任务要求。期刊采用单盲评审，本稿未将缺少作者资料解释为期刊要求匿名。

依照 [IEEE 生成式 AI 内容指南](https://open.ieee.org/author-guidelines-for-artificial-intelligence-ai-generated-text/)，致谢披露 OpenAI Codex 辅助撰写的章节，以及验证、绘图和构建脚本的辅助范围。图是实际数据的确定性绘制，没有生成式伪造实验图。

六篇 DOI 经 Crossref 登记元数据核对，并对相关主张查阅作者/出版方页面；Holm 论文使用真实书目，不将一次 Crossref 404 的查询当作有效 DOI 证据。其稳定文献地址为 [JSTOR 4615733](https://www.jstor.org/stable/4615733)。文献核对记录保存在 `results/paper_20261025/evidence/references.json`。

当前稿没有声称已证明普遍优于 SA-only、所有自适应机制有效、真实高保真恢复、最先进恢复性能或跨工区泛化。稿件格式、引用和本机复现检查通过，不等于期刊科学录用条件全部具备。

投稿前需由作者确认单位、通讯邮箱、ORCID、资助和署名；完成最终科学/语言审阅，核实数据许可与可访问性，解决真实编码与独立资料问题，并按拟主张设计充分功效的确认性对照。完整剩余事项见项目进展报告和 `docs/UNRESOLVED_ISSUES.md`。
