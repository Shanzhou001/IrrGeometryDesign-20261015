# Viking 海上地震接收道选择与缺失道重建

项目使用 Viking Graben Line 12 的登记实测炮集，完成受约束选道、缺失道重建和独立评分。

- [完整项目说明](PROJECT_OVERVIEW.md)
- [实测结果](results/viking_only/report.html)
- [实验图件](figures/index.html)
- [验收记录](results/viking_only/workflow.json)

在本目录运行 `python -B pipeline.py run` 打开 Qt 程序，或运行 `python -B pipeline.py closure-status` 核对活动批次。重算实验使用 `python -B pipeline.py thesis --scope viking --out results/viking_replay`。
