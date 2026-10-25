# 论文编制前的软件与科学复核记录

日期：2026-10-25（Asia/Shanghai）。本页区分实际执行、历史记录和尚未完成项目，不把软件检查等同于研究假设成立。

## 1. 基线与执行环境

- 审查起点 Git 提交：`8b9ffef2731f2210a8144900f155385d1dba266c`，最初工作区干净。
- 新验证产物集中在 `results/paper_20261025/validation/`，没有覆盖 v2/v3/v4/v5 科学实验归档。
- 本机为 Windows x64、Python 3.12.10、Qt 4.7.4。
- 实际 MSVC 编译器文件版本为 `15.00.30729.01 built by: SP`，产品版本为 `9.00.30729.01`；qmake 为 `2.01a`。旧环境脚本中的 Beta2 横幅没有用作版本判断依据。
- Release 程序 SHA-256：`fb043f331ba38fe9f5d35d8feb3e6173f51db97778ecc191b3922b0230f4b568`。
- Debug 程序 SHA-256：`5df7d51d90fd421f66ea02b755aab7073a61dbfdb70fde2ce72764e9176694a4`。

## 2. 本轮实际执行

下表的相对路径以 `results/paper_20261025/validation/` 为起点。

| 检查 | 实际状态与范围 | 证据 |
| --- | --- | --- |
| VS2008 Release/Debug 构建 | 两次构建调用退出 0；日志为 up-to-date，并非全工程重新编译 | `build/pipeline.json`、两个构建日志 |
| Qt4 qmake/nmake 构建 | 退出 0，使用原 Qt4/MSVC 环境 | `qmake_build.json`、`qmake_build.log` |
| Python 自动测试 | 首轮及源码冻结后均实际 38 项通过，不累计为 76 种测试 | `test_final/regression/python.log` |
| Release C++ 回归 | 两轮均 PASS=33、FAIL=0、SKIP=0 | `test_final/regression/release_regression_native.log` |
| Debug C++ 回归 | 两轮均 PASS=33、FAIL=0、SKIP=0 | `test_final/regression/debug_regression_native.log` |
| 最终冻结后的 test wrapper | 退出 0，`completed=true`；源码指纹结束时保持一致 | `test_final/pipeline.json` |
| 独立目标与硬约束复核 | 588 例通过：504 个显式字典/覆盖/目标案例和 84 个小规模枚举 DP 案例 | `numeric/verification.json`、查询与原生输出 |
| 实际坐标观测投影 | 2 个独立 NumPy 对照通过，最大绝对差 `6.0091e-8` | `projection/verification.json` |
| FDCT 正反变换探针 | 两构建实际编译并各执行 20 种组合，最大相对 L2 误差 `2.2935e-15` | `fdct/verification.json`、各构建日志与 CSV |
| 旧解码/边界安全专项 | 两构建各 7 种用例通过，共 14 次执行 | `legacy_safety/checks.json` |
| 确定性搜索与恢复重放 | 930 搜索例和 6 个预设固定恢复例通过；搜索比较排除计时字段，预测文件逐字节对照 | `replay/verification.json` |
| 两例端到端软件检查 | 解析模型和争议编码真实资料各 1 例完成优化、内部选择、恢复和评分 | `smoke/verification.json` |
| v3 快速完整兼容流程 | 12 步全部实际完成，独立 NumPy 复核 234 行通过 | `legacy_pipeline/run_manifest.json`、`legacy_verification.json`、`legacy_verification.log` |
| 历史结果与哈希 | v4 5843 条记录、16716 个输出文件哈希通过；v5 四阶段核验通过 | `archives/verification.json`、`archives/v4.json` |
| 历史基线文件 | 34526 个文件中 34513 个使用现工作区原字节、13 个显式使用 Git 原版验证 | `archives/verification.json` |
| 结构统计复算 | 24 项以 6 个干净结构实现为单位的配对检验通过复算；Holm 显著项为 0 | `archives/structural_statistics.json` |
| 初始目录/图片/帮助入口 | 41 个 Python 模块、16 个科学源文件迁移等价性、66 个本地链接和 33 个帮助入口通过；81 张 PNG，目录外 0、字节重复 0 | `layout.json` |
| SEG-Y 字节审计 | 原始字节布局再次复核；波形编码仍为 UNRESOLVED | `segy/audit.json`、全部道头 CSV |

数值复核中，互干度最大绝对误差为 `2.7988e-8`，最近邻 CV 为 `5.5301e-8`，目标值为 `7.0559e-8`，最大物理间隔误差为 0。

“初始目录/图片”检查在新增论文英文图之前执行，不能用该 81 张计数代替论文图加入后的最终清单核验。后续图件和 PDF 的质量检查由论文构建记录单独说明。

主任务补充的最终检查见 `results/paper_20261025/layout_final.json`：42 个 Python 模块、34 个帮助入口、91 个本地链接通过；85 张 PNG（55 张生成、30 张素材），目录外与字节重复均为零。PDF 质量记录 `results/paper_20261025/pdf_quality.json` 确认八页、连续页码、四图、作者元数据、字体嵌入及构建输入哈希；Codex 已逐页查看最新八页渲染，不将其称为作者/导师人工审稿。

## 3. 保留的首次失败

首轮命令：

```powershell
python pipeline.py test --out results/paper_20261025/validation/test
```

其中 Python、Release、Debug 回归均通过，但运行期间报告生成器 `research/analysis/make_audit_reports.py` 的图路径文字被修正。流水线在结束时检测到源码 SHA 改变，按设计退出 1，错误为 `active source changed during pipeline`。

该次 wrapper 的 `completed=false` 和完整日志保留，没有改成通过。它证明源码变动保护实际生效，并不表示底层科学回归失败。源码冻结后另建 `test_final/` 重新执行，不覆盖首次记录。

## 4. 数据审计与结论边界

- RRAW SHA-256：`ddcb311b2406af17cf375b570b3eb8ea7ca2d81cdeb002f972e6d668cd7b2bc2`。
- 文件布局为 59 道、每道 250 个四字节样点、每道总跨度 1240 字节，样点数据合计 59000 字节，无尾部余数。
- 五个 CDP 的道数分别为 8、15、15、14、7；这些是字节级解析事实，不能证明波形的 IEEE-LE 解释就是数据提供方原意。
- 两例 smoke 属于软件检查，不增加独立真实来源数，也不是新增算法优越性的确认性实验。
- 回归中的曲波参数排序检查实际报告仅 4/7 种配置保持排序；程序通过表示检查逻辑正确执行，不代表排序在全部配置下稳定。
- 软件、数值与哈希检查不证明 Adaptive 普遍优于 SA-only，也不证明降低互干度必然改善恢复 SNR。

## 5. 尚未执行或仍受外部条件限制

- 新独立真实工区资料的下载、许可核验、预注册和完整实验。
- 数据提供方对 RRAW payload 编码、采集分组、坐标单位和采样历史的确认。
- 全部 GUI 功能的人工鼠标操作验收；已有自动化测试或截图不能代替这一步。
- ASan/Valgrind 等现代内存检测、另一台机器/操作系统的完整复现、干净克隆的全部实验逐字节复现。
- 严格按墙钟匹配降温日程的确认性对照，以及增加独立模型家族后的研究结论验证。

这些事项没有被记为通过或已修复。更多研究限制见 [未解决事项](../UNRESOLVED_ISSUES.md)。

机器可读总索引保存在 `results/paper_20261025/validation/index.json`，原始命令、程序与输入指纹以各专项 JSON 为准。
