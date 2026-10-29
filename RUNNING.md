# 编译、实验和验收

所有命令在IrrGeometryDesign目录执行。Python3.12；原生工具链为VS2008 SP1 x64及Qt4.7.4。

```powershell
python -m pip install -r requirements.txt
.\build.bat Release
.\build.bat Debug
python -B pipeline.py test
python -B pipeline.py thesis --workers 16
```

GUI运行相同的 `pipeline.py thesis` 入口。每个阶段失败会停止，输出至 `results/current`。首次运行重新完成完整检查、合成搜索与恢复、Viking稳定性、F3重复迁移、预测独立复核、波形诊断、SPS Gram复核和图件生成；后续同身份批次核对登记文件后复用。

仅检查GUI图件加载、缩放、翻页和刷新：

```powershell
python -B pipeline.py run -- --gui-gallery-test
```

从Qt实际启动完整流程并在完成后逐张检查图库：

```powershell
$env:IRR_PYTHON = (Get-Command python).Source
python -B pipeline.py run -- --gui-workflow-test
```

截图全部GUI窗口：`python -B pipeline.py run -- --screenshot-all`。GUI截图生成在figures/gui；部分独立视图的测试输入是合成演示数据，正式科研图仅收录figures/current.tsv中的清单。

单独复核当前批次：

```powershell
python -B pipeline.py module research.validation.verify_synthetic --root results/current
python -B pipeline.py module research.validation.verify_viking_batch --root results/current/viking --out checks/viking_recheck
python -B tools/verify_f3_transfer.py --root results/current/f3 --out checks/f3_recheck
```

核验输出目录须不存在。不要删除原始预测、掩码、观测输入、选择记录或求解轨迹，否则完整审计会明确失败。`--keep-intermediates`为兼容参数，当前实验始终保留复核依赖，不再自动清理。

当前结果保留大量逐案例审计文件；同版本复用也会逐文件核验哈希。磁盘较慢时这一阶段日志间隔较长，保持窗口开启并等待最终状态即可。编译对象和重复截图可以清理，正式观测/预测及选择记录需要保留。

结果不理想时，应检查逐条件配对差、线性/f-k基线、RMS比与相位，不得按外层分数改协议后继续称为独立验证。科学结论以本轮实际CSV为准。

当前 `output/runtime_current` 已补齐VC90 CRT、VC90 OpenMP和生成的中文翻译，全部文件身份见其runtime_manifest.json。若自行调用runtime_bundle重新创建运行包，该工具首先生成批处理运行依赖；GUI打包还需复制 `build/bin/Release/IrrGeometryDesign_zh.qm` 和本机VS2008 `VC/redist/amd64/Microsoft.VC90.OpenMP` 整个程序集目录，再登记新增文件哈希，并在移除QTDIR、PATH仅含System32的环境中重新执行核心及图库测试。不要将这些VC运行库误当作所有Windows机器自带的系统DLL。

本机实际验收：Qt启动的从零完整实验75.16分钟；清理后冷数据完整留存哈希复核约30.00分钟。耗时受磁盘缓存、硬盘速度和并行数影响，同版本复用也需等待核验完成。完整结果与限制见ACCEPTANCE.md。
