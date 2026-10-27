# 可编辑英文研究稿

主编辑源为项目内 [manuscript.md](manuscript.md)，生成阅读版位于工作区 paper-tgrs/tgrs_trace_selection.pdf。它是研究初稿，尚未提交，也不是已验收的IEEE期刊模板定稿。

从sci工作区运行：

```powershell
python -m pip install -r paper-tgrs/requirements.txt
python -B paper-tgrs/build_manuscripts.py
```

脚本同时构建相邻thesis目录的中文六章初稿。Markdown是主编辑源，`*_expanded.md`为宏展开版本，TeX和BibTeX可继续编辑；本机未安装XeLaTeX，因此PDF由ReportLab生成，未声称TeX编译已验证。Windows中文字体取自系统STSong/SimHei，英文使用Times New Roman/Consolas。

表格直接从IrrGeometryDesign的冻结统计CSV读取并核对分析清单；图件从已保存结果复制到assets。缺少本地实验统计时构建会报错，不会编造数值。[build_manifest.json](build_manifest.json)记录输入与输出身份，[reference_audit.json](reference_audit.json)记录一手文献与元数据核对。

`robustness.md`与`robustness_zh.md`记录补充稳定性，必须以完整运行和审计为依据。只保留当前研究稿，旧PDF已清理；仍被引用的不同实验批次保留原始证据。当前稿件不宣称跨工区泛化、稳定优于SA或已经提高成像质量。

交付包包含源稿和生成依赖说明；正文、构建器和校验器已纳入IrrGeometryDesign/manuscript_sources。相邻paper-tgrs/thesis是再生成的兼容产物目录；大型实验材料仍按本地交付清单提供。
