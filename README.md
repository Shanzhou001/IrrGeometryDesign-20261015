# 地震采样点筛选与恢复研究

论文项目：《基于混合优化算法的地震采样点智能筛选方法研究》。
包含 Qt4 桌面程序、固定采样数优化、实际坐标 Fourier/PSF 评价、FDCT 恢复、SEG-Y 数据处理和批量统计。

## 论文稿与最新审查（2026-10-25）

[8 页英文期刊稿 PDF](output/pdf/tgrs_trace_selection.pdf) · [论文源码与复现说明](docs/paper/README.md) · [项目进展、问题修复及剩余事项](docs/PAPER_PROGRESS_20261025.md) · [本轮实际验证](docs/paper/VALIDATION_20261025.md)。
作者采用用户提供的 Wu shanzhou；稿件按 IEEEtran 双栏编排，保留负结果和真实资料编码未确认等限制，投稿前仍需作者审阅、身份资料及数据授权确认。

## 当前入口

在项目根目录执行；也可以从其他工作目录用 `pipeline.py` 的绝对路径运行。

```powershell
python pipeline.py all
python pipeline.py build
python pipeline.py test
python pipeline.py figures
python pipeline.py verify
python pipeline.py inspect
```

- `all`：Release/Debug 构建、Python/C++ 回归、独立数值与投影核验、FDCT/旧工具类专项、搜索与预测重放、两例完整数据流水线、历史结果核验、全部图片重绘和目录检查。
- `figures`：从归档 CSV 重绘研究图片；恢复图缺少轨迹时，在新的验证目录重放指定掩码；再捕获 8 个 GUI 画面。覆盖 `figures/` 中的规范文件，不创建图片副本。
- `verify`：核验工作区中的科学数据、原生输出及当前图片；迁移前的来源和旧版图片通过明确的 Git/运行快照核验。
- `inspect`：检查 Python 导入、依赖环、C++ 包含路径、Qt 图标资源及科学核心代码迁移等价性。

每次命令在 `results/validation/<时间>/` 保存命令、日志、退出码、程序/源码/输入/输出哈希；可以用 `--out` 指定一个**尚不存在**的验证目录。
独立探针的构建与执行产物随验证记录归档；日常应用构建产物保存在 `build/`。
测试失败返回非零；跳过不计通过。需要完整执行记录时，以生成的 `pipeline.json` 和各专项 JSON 为准。

## 目录

```text
src/native/              C++/Qt 的活动源码
  app/                   程序启动与原窗口
  core/                  数学基础、随机数
  geometry/              SPS 属性与几何加载
  io/                    SPS 数据模型、扫描与既有 SEG-Y 工具
  sampling/              固定点数采样、互干度与掩码
  optimization/          SA、历史 Hybrid、自适应搜索
  reconstruction/        FDCT 恢复与实际坐标观测算子
  experiments/           数据入口、采样/几何/工作流/研究 CLI
  gui/                   对话框、视图和观测系统界面
  plotting/              原生绘图与统一图片路径
  tests/                 原生回归及专项片段
research/                可通过 python -m 运行的 Python 包
  common/                路径、哈希、CSV、子进程和唯一图片写入器
  data/                  SEG-Y 审计、转换
  experiments/           v3、v4、v5 研究执行
  analysis/              统计、诊断、研究图和历史图重绘
  validation/            科学复核、重放、自动测试与结构检查
  maintenance/           既有审计归档工具
build/legacy/            VS2008 工程
build/qt/                与 VS 工程一致的 Qt4 qmake 工程
build/bin/               构建后的 Release/Debug 程序（忽略提交）
resources/qt/            Qt 资源清单与翻译；图标实体位于 figures/assets
figures/                 唯一的图片目录，包含总览、清单和数据说明
  research/v5/           10 组当前论文图
  research/v4/           6 组历史正式图
  legacy/                按实际输入去重后的历史示例图
  gui/                   8 个程序界面截图
  assets/                Qt 原始图标素材
  paper/                 从归档数据生成的4张英文期刊稿图
results/                 科学 CSV、掩码、预测、日志、清单及冻结运行快照
config/path_migrations.json  旧路径到规范位置的映射
scripts/                 本轮一次性迁移/引用更新脚本
```

[架构和依赖说明](docs/ARCHITECTURE.md) · [命令目录](docs/CLI_REFERENCE.md) · [本轮验证](docs/REFACTOR_VALIDATION.md) · [图片总览](figures/index.html)

## 环境与启动

保留 VS2008 SP1、Qt 4.7.4、原 FFTW/FDCT 和 Eigen；不需要升级原生工具链。
`build_release.bat`、`build_debug.bat` 是兼容入口，共同调用 `build/native.bat`。
可通过 `QTDIR`、`VCVAUTO`、`VCARCH` 指定现有安装。

```powershell
python -m pip install -r requirements-research.lock.txt
.\build_release.bat
.\build\bin\Release\IrrGeometryDesign.exe
```

Python 锁定文件记录本机实际使用的 Python 3.12.10 运行依赖；C++ 构建不依赖 Python 包。
Qt4 qmake 工程位于 `build/qt/IrrGeometryDesign.pro`，须使用相同的 Qt4/MSVC2008 x64 环境。
本轮实际执行情况见验证报告，不将配置文件的存在当作验证通过。

## 图表规则

所有自动生成的图片都是 PNG，每个图 ID 在一次绘图过程中只允许一个保存调用。
`figures/manifest.json` 记录文件哈希、生成次数及来源；`figures/aliases.tsv` 供旧 GUI 图库解析共享图片。
静态 Qt 图标作为源素材移动到 `figures/assets/`，保持原有 Qt 资源标识。
历史草稿、SVG 和重复副本从工作目录移除，原版保留于 Git 提交 `c191e6c5`。
归档结果中的原始命令和数值数据继续保留；这些路径是历史记录，由显式迁移/快照核验读取。

批量实验只产生数据，统一绘图阶段生成图片，避免在每个候选掩码上重复保存。
重绘直接覆盖规范位置；不再生成新的图片草稿目录。

## 研究资料与结论边界

正式历史数据分别位于 `results/revised_20261023/`、`results/enhanced_20261023/`、
`results/review_20261024_v4/final/`；补充审计数据位于 `results/audit_20261024_v5/`。
各协议的结果独立保留，结构整理不改变采样目标、搜索机制或已归档的科学数值。

预算回收有明确的预算利用证据，尚未证明稳定优于 SA-only；低互干度不保证更高恢复 SNR。
RRAW 的 IEEE-LE payload 解释仍未由提供方证实，真实波形来源仍只有一个。

[研究审计索引](docs/AUDIT_ISSUES.md) · [实验结果](docs/EXPERIMENT_RESULTS.md) · [数据验证](docs/DATA_VALIDATION_REPORT.md) · [贡献边界](docs/THESIS_CONTRIBUTIONS.md) · [未解决项](docs/UNRESOLVED_ISSUES.md)
