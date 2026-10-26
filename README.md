# 地震采样点筛选与恢复

C++/Qt4 桌面程序与 Python 研究工具，包含采样优化、Fourier/PSF 评价、FDCT 恢复和 SEG-Y 处理。

## 目录

```text
src/
  native/         C++ 界面、算法和命令行
  research/       Python 实验、分析、绘图和数值检查
  tests/          Python 与独立 C++ 测试
  third_party/    Eigen、Curvelet、QCustomPlot 和必要的 FFTW/FDCT 链接库
  resources/      Qt 资源与翻译
  build/          VS2008、Qt4 工程和构建脚本
figures/
  assets/         程序图标
  gui/            界面截图
  paper/          论文图片
  research/       研究图片
  legacy/         历史示例图片
  index.html      统一图库
build/            编译后的程序、Qt 自动生成代码和编译中间文件
data/             输入数据，包括 SPS 和自行提供的地震资料
checks/           保存的检查记录
results/          运行实验后生成的结果
build.bat                 原生构建入口
pipeline.py               运行与研究入口
IrrGeometryDesign.sln     VS2008 解决方案
requirements.txt          Python 依赖
```

源代码、图片、输入数据和运行产物统一放在本项目目录内，无需额外的工作区文件夹。Git 跟踪源代码、图片和工程配置；编译产物、输入数据、检查记录和实验结果保留在本地。

## 构建与运行

原生环境：VS2008 SP1、Qt 4.7.4、x64。脚本自动查找已有安装，也可设置 `QTDIR`、`VCVAUTO`、`VCARCH`。

```powershell
.\build.bat Release
.\build.bat Debug
python pipeline.py run
```

构建配置集中在 `src/build/`，编译产物写入项目内的 `build/`。也可直接从 Visual Studio 打开解决方案，无需设置工作区路径。

Python 使用 3.12：

```powershell
python -m pip install -r requirements.txt
python pipeline.py test
python pipeline.py figures
python pipeline.py module research.validation.audit_numerics --out checks/numerics
python pipeline.py module research.validation.audit_projection --out checks/projection
```

`figures` 只更新现有图片的图库；查看 [图片资料库](figures/index.html)。绘图和实验模块通过 `module` 入口运行，可在模块名后加 `--help` 查看参数。

## 输出与输入数据

工作目录就是本项目根目录；构建、运行和研究入口均使用以下位置：

```text
IrrGeometryDesign/
  build/bin/Release/      编译后的程序
  build/bin/Debug/
  build/generated/       Qt 自动生成代码
  build/obj/             编译中间文件
  data/                  自行提供的输入数据
  results/               新实验结果
  checks/                指定保存的检查记录
```

真实资料实验需将外部数据放入 `data/real_seismic/`，原始 SEG-Y 放入 `data/real_seismic/Data/RRAW.SGY`，SPS 文件放入 `data/sps/`。未安装真实数据时，相应 Python 测试会明确跳过；原生完整 `--test` 还需要这些输入。自包含的数值、统计和格式检查可独立运行。

图片统一写入 `figures/`；生成目录、输入数据和 IDE 缓存均已在 `.gitignore` 中排除。
