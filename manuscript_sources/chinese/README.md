# 学位论文研究初稿

主编辑源为项目内 [thesis.md](thesis.md)；展开稿、阅读PDF与TeX由构建器生成至工作区thesis目录。

六章已经覆盖背景、理论与模型、实际算法、合成和公开资料实验、系统与SPS扩展、总结与局限。它是可供作者和导师修改的研究初稿，尚未套用学校模板，也不代表学位评审通过。

从sci目录执行`python -B paper-tgrs/build_manuscripts.py`同时更新中英文稿。表格由保存CSV生成；源码、结果和图的对应关系在paper-tgrs/build_manifest.json中记录。最终研究结论以真实实验为准，不能因软件检查通过而写成普遍性能优势。
