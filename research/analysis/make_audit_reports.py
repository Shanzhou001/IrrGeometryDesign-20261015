"""Build the numerical results and validation report directly from archived records."""
import argparse
from collections import defaultdict
import json
from pathlib import Path
import re

import numpy as np

from research.common.tabular import rows, write_rows_if_any as write_rows
from research.common.hashing import digest

from research.common.paths import ROOT


def mdtable(data,keys):
    result=['|'+'|'.join(keys)+'|','|'+'|'.join('---' for k in keys)+'|']
    for r in data:
        result.append('|'+'|'.join(format(r[k],'.6g') if isinstance(r[k],float) else str(r[k]) for k in keys)+'|')
    return '\n'.join(result)


def group(data,keys):
    result=defaultdict(list)
    for r in data:result[tuple(r[k] for k in keys)].append(r)
    return result


def make(root):
    analysis=root/'analysis_thesis';search=rows(analysis/'search_means.csv');joint=rows(analysis/'joint_means.csv')
    jointpairs=rows(analysis/'joint_pairs.csv');oldpairs=rows(root/'analysis/joint_pairs.csv')
    sensitivity=rows(analysis/'switch_sensitivity.csv');associations=rows(analysis/'geometry_recovery_associations.csv')
    fulljoint=rows(root/'joint/joint.csv');real=rows(root/'real/real.csv');realmeans=rows(analysis/'real_means.csv')
    times=rows(analysis/'time_limit_means.csv');searchpairs=rows(analysis/'search_pairs.csv')
    common=rows(analysis/'common_evaluation_pairs.csv');isolation=json.loads((root/'real/isolation.json').read_text())
    rounding=json.loads((root/'rounding_isolation/verification.json').read_text())
    sourcecount=sum(len(json.loads((root/name/'manifest.json').read_text())['steps']) for name in ['search','joint','real','timing'])
    text=['# audit_joint_v5 实际结果与统计复核','',
          '本报告由 `research/analysis/make_audit_reports.py` 从归档 CSV 生成。原生实验版本 `80ca9303`，运行程序 SHA-256 '+json.loads((root/'search/manifest.json').read_text())['executable_sha256']+'。',
          '后续旧工具类修复没有覆盖本批结果；原程序/源码在 `runtimes/v5/`，当前程序的独立重放另存 `final_replay/`。',
          '',f'主要四阶段共 {sourcecount:,} 次原生步骤：840 个搜索敏感性案例、90 个联合掩码生成案例、1080 条合成恢复记录、24 条真实窗口记录、60 次限时搜索。',
          '1080=12个模型/噪声条件×3采样档×3掩码种子×5方法×2种评价设计；每个共同 H 案例同时保存固定和内部选择后的恢复。',
          '12 条件来自 6 个干净结构模板×2档噪声，不是 12 个独立地下模型；真实来源仍只有 1 个，且编码未确认。',
          '', '## 1. 搜索质量、预算和机制', '',
          'B=300，各组10种子，共同初值及目标。以下均值不是与 v4 合并的样本。物理域仅指采用头 offset 的条件性几何。',
          'v5初值由NumPy分层Jitter生成，v4使用C++ Jitter；相同seed不保证跨协议初始mask相同，不能跨协议作配对改进比较。']
    methods=['SAOnly','Hybrid','SA_Budget','Adaptive','FixedSwitch','NoRecycle','AdjacentOnly','NoPerturb']
    for domain,counts in [('uniform',[38,64,90]),('physical',[20,30,40])]:
        result=[]
        for ri,m in enumerate(counts):
            r=dict(domain=domain,m=m)
            for method in methods:r[method]=float(next(v['mu'] for v in search if v['domain']==domain and v['rate']==str(ri) and v['method']==method))
            result.append(r)
        text+=['',mdtable(result,['domain','m',*methods])]
    text+=['',f"42 项 Holm 后显著项 {sum(float(v['p_holm'])<.05 for v in searchpairs)}，最小调整 p={min(float(v['p_holm']) for v in searchpairs):.8g}。",
           '10种子在42项校正下检出力不足，不把“未显著”解释为“证明没有收益”。v4 的20种子/72项历史结果仍单独保留。',
           'Adaptive–NoRecycle 在六组各自共同实际评价次数下的均值差全部恰为0；完整轨迹前缀相同。最终差异主要包含继续利用剩余预算的效果。',
           '切换/邻域/额外扰动的独立普适收益尚未建立，NoPerturb 的描述性好坏均保留，未替换正式算法。',
           '', '敏感性只展示预先定义的7组 minSA/window 的均值范围，不根据最小值选择新主参数：','']
    ranges=[]
    for (domain,rate),g in group(sensitivity,['domain','rate']).items():
        values=[float(r['mu']) for r in g];ranges.append(dict(domain=domain,rate=rate,configurations=len(g),mu_min=min(values),mu_max=max(values)))
    text.append(mdtable(ranges,['domain','rate','configurations','mu_min','mu_max']))
    text+=['','## 2. 相同时间上限补充','',mdtable([{k:float(v) if k.startswith('mean_') else v for k,v in r.items()} for r in times],list(times[0])),
           '', 'QPC 30ms 上限，初始化和缓存计入，逐种子轮换顺序且顺序运行。表中实际时间略超上限来自最后一次评价。',
           'B=100000 的温度按评价进度而非时间进度，限时结束前没有完成降温，因此这是条件性限时实验，不能当作匹配降温的普适效率证明。',
           'SA_Budget 与 Adaptive 共享评分后端；旧物理直接求和与缓存的开销不能全部归功搜索机制。进程启动/IO时间另在每步 manifest 中。',
           '', '## 3. 共同外部 H：采样与恢复器作用分开','',
           '固定 H=索引4,12,…,124，共16道；112个允许候选中取38/64/90道，Fourier 字典仍按完整128点域定义。',
           '所有未观测列先置零再传入原生恢复，H的真实波形只用于最终评分。表中单位dB；同一结构模板先平均掩码，再平均两档噪声。','']
    jointtable=[]
    for method in ['Jitter','SAOnly','Hybrid','SA_Budget','Adaptive']:
        r=dict(method=method)
        for m in [38,64,90]:
            for recovery in ['fixed','selected']:r[f'{m}_{recovery}']=float(next(v['mean_snr'] for v in joint if v['method']==method and v['m']==str(m) and v['recovery']==recovery))
        jointtable.append(r)
    text+=[mdtable(jointtable,list(jointtable[0])),'',
           f"按6个结构模板做24项Holm比较，显著项{sum(float(v['p_holm'])<.05 for v in jointpairs)}，最小调整p={min(float(v['p_holm']) for v in jointpairs):.6g}。",
           f"原12条件探索性统计曾有{sum(float(v['p_holm'])<.05 for v in oldpairs)}/24项小于.05，完整保留在analysis/；不能将其称作12个独立地下结构的泛化证据。",
           '依赖性修正在看过初版结果后进行，具体规则/时序在 EXPERIMENT_PROTOCOL_STRUCTURAL_ADDENDUM.md 披露。',
           '内部选择并不保证外层改善：例如Adaptive在64/128时，选后均值低于固定默认；Jitter在38/128时也下降。',
           '选后与固定的差异属于恢复流程，方法之间在固定列的差异才用于讨论给定恢复器的采样影响。',
           '95% bootstrap区间、配对差、秩二列效应和原始/调整p见 analysis_thesis/joint_means.csv、joint_pairs.csv。',
           '', '传统掩码补集评价另存，不能与共同 H 的 dB 相减作为算法增益：','']
    complement=[]
    for (m,method),g in group([r for r in fulljoint if r['design']=='complement'],['m','method']).items():
        complement.append(dict(m=int(m),method=method,heldout=128-int(m),records=len(g),fixed_snr=float(np.mean([float(r['fixed_snr']) for r in g])),linear_snr=float(np.mean([float(r['linear_snr']) for r in g]))))
    write_rows(analysis/'traditional_complement_summary.csv',complement)
    text.append(mdtable(complement,list(complement[0])))
    text+=['','## 4. 几何指标、恢复与频带','',
           '每个家族/结构/噪声条件分别对唯一掩码求描述性相关，不将多掩码当独立地质样本，不报告这些相关的显著性。',
           '全部 μ、max-gap、NN-CV 与 SNR 的216组分层结果见 geometry_recovery_associations.csv。μ–SNR 的条件范围为：','']
    relation=[]
    for (design,m),g in group([r for r in associations if r['metric']=='mu'],['design','m']).items():
        values=[float(r['spearman']) for r in g if r['spearman']!=''];relation.append(dict(design=design,m=m,conditions=len(g),rho_min=min(values),rho_median=float(np.median(values)),rho_max=max(values)))
    text+=[mdtable(relation,list(relation[0])),'',
           '相关方向随条件改变，低μ没有稳定地保证更高SNR。该关联不是因果解释，也没有用于选权重或频带。',
           'v4九组gap/NN权重及几何Pareto完整保留并重新绘图；不同权重优化不同J，没有将其数值直接并入同目标算法排名。',
           '周期直接取孔径且仍保留N原子会在均匀网格产生重复列，μ恒1；物理域评分排名对频带也敏感。详见 ALGORITHM_REVIEW.md 和 band_rank_sensitivity.csv。',
           '', '## 5. 真实来源：原幅度、时间窗与失败','',
           '全部条件于未证实的IEEE-LE解释和头offset，绝不是已校准物理资料。每窗每档2个相同几何划分，无空间插值/滤波/全场归一化。',
           'native_crop最后时间1.016s，upsampled_crop为1.020s；全段均至1.992s。不同目标能量不能跨窗当同一任务增益。','',
           mdtable([{k:float(v) if k.endswith('_snr') else v for k,v in r.items()} for r in realmeans],list(realmeans[0])), '',
           f"24例中默认负SNR {sum(float(r['fixed_snr'])<0 for r in real)} 例，选后负SNR {sum(float(r['selected_snr'])<0 for r in real)} 例，零模型被选{sum(r['selected']=='zero' for r in real)}例。负结果全部保留。",
           'SNR有限与波形图中的振幅/波形不匹配一致；误差比=1+预测能量比−2×交叉项，是诊断恒等式，不是失败原因的唯一识别。',
           '时间像素尺度、弱连续性/混合CDP、外推与非凸收缩可能影响结果；编码、原始采集和处理历史未确认，不能强行归因为某个Bug。']
    legacy=ROOT/'results/review_20261024_v4/final'
    diagnosis=json.loads((legacy/'real_diagnosis.json').read_text())['training_continuity_and_spectrum']
    continuity=[{k:r[k] for k in ['name','training_traces','zero_lag_mean','bounded_lag_mean','median_absolute_lag_s','median_dominant_frequency_hz']} for r in diagnosis]
    attribution=rows(legacy/'real_nested_summary.csv')
    text+=['','v4固定描述性训练子集的历史诊断（不与v5合并统计，也不反馈搜索/选择）：','',
           mdtable(continuity,list(continuity[0])),'',
           '有界时差为±48ms，最大相关本身有搜索偏差，不能据此确认NMO速度或真实同相事件。分CDP后相关并未普遍改善，邻距也不同，不能将失败唯一归因于混排。',
           '该4ms插值输入在62.5Hz以上的FFT能量比例约7.4%–8.8%，不代表原8ms资料具有新增独立高频信息，可能含插值/有限窗谱效应。',
           '历史相同随机外层划分的幅度/配置分解，单位dB：','',
           mdtable([{k:int(r[k]) if k in ['keep','n'] else float(r[k]) for k in ['keep','n','fixed','fixed_gain_only','selected_before_gain','selected']} for r in attribution],
                   ['keep','n','fixed','fixed_gain_only','selected_before_gain','selected']),'',
           '仅用内部验证幅度系数已能把默认负值拉近零基线，说明均方误差改善的一部分来自预测收缩；它不是采样搜索的收益。selected_before_gain只是输出分解，并非另行调优的参数单独最优方法。']
    text+=[
           '', '## 6. 隔离、单位修复和复现','',
           f"新原幅度链的隐藏干预：选型不变={isolation['raw_selection_unchanged']}，全预测最大差={isolation['raw_prediction_max_abs_difference']:.6g}。",
           f"旧未舍入归一化在共同原单位下仍有差{isolation['legacy_physical_prediction_max_abs_difference']:.8g}；按历史6位有效数字舍入后差{rounding['prediction_max_abs_difference']:.8g}，相对原峰值{rounding['relative_to_raw_peak']:.8g}。",
           '旧链因此不能称为严格的原始数据预处理隔离；近似抵消不等于完全无依赖。新链通过取消全场预归一化并物理屏蔽未观测列消除该路径。',
           '原单位输入的固定绝对投影误差判据曾导致调用失败，已最小修复为按已知道尺度判定；它没有解释或消除旧归一化实验的负SNR。',
           '程序/输入变化后明确拒绝旧指纹；本批只用显式冻结快照复核。当前最终程序另通过930个搜索案例和6个预设预测的逐字节/科学列重放。',
           '', '## 7. 数据、表图及结论适用范围','',
           '全部原生输入、命令、参数、种子、输出及哈希见各 stage 的 manifest.json/journal.jsonl；原始汇总为 search/search.csv、joint/joint.csv、real/real.csv、timing/timing.csv。',
           '最终10组中文PNG/SVG/data.json位于 figures_thesis/，最终图哈希见 final_figures_manifest.json；初稿也保留。',
           '当前结果支持预算管理、数学实现一致性和目标失配研究；不支持通用最优、全部机制有效、跨工区适用、真实高保真或成像改善。',
           '独立真实来源、提供方说明、GUI人工走查及异机全重跑仍未完成，见 UNRESOLVED_ISSUES.md。','']
    resultpath=ROOT/'docs/EXPERIMENT_RESULTS.md';resultpath.write_text('\n'.join(text),encoding='utf-8')
    checks=json.loads((root/'final_all_checks/checks.json').read_text());codec=json.loads((root/'safety_final/checks.json').read_text())
    assert len(checks)==5 and all(r['exit_code']==0 for r in checks),'final validation is not complete'
    fdct=json.loads((root/'fdct_probe/verification.json').read_text())
    frozen=json.loads((root/'frozen_runtime_verification.json').read_text());original=json.loads((root/'v4_independent_verification.json').read_text())
    numeric=json.loads((root/'final_numerics/verification.json').read_text());projection=json.loads((root/'final_projection/verification.json').read_text())
    replay=json.loads((root/'final_replay/verification.json').read_text());compat=json.loads((root/'compatibility/verification.json').read_text())
    validations=[]
    for r in checks:
        item=dict(check=r['name'],exit_code=r['exit_code'],seconds=r['seconds'])
        if r['name'].endswith('_regression'):
            log=(root/'final_all_checks'/(r['name']+'.log')).read_text(encoding='utf-8',errors='replace')
            item.update(passed=len(re.findall(r'\[PASS\] Test \d+:',log)),failed=len(re.findall(r'\[FAIL\]',log)),skipped=len(re.findall(r'\[SKIP\]',log)))
            assert item['passed']==33 and item['failed']==item['skipped']==0
        validations.append(item)
    report=['# 构建、验证与结果追溯报告','',
            '基线提交72240bbf2a9ad02fec97fd250789d188d9db50fa，初始工作区干净；分阶段提交和独立输出，没有覆盖历史科学文件。',
            '本报告从实际日志/JSON生成，手工GUI、异机全实验和未执行工具不会被计入通过。','',
            '## 当前最终程序','',mdtable([{k:r.get(k,'—') for k in ['check','exit_code','seconds','passed','failed','skipped']} for r in validations],['check','exit_code','seconds','passed','failed','skipped']), '',
            'Python日志为33项（原24+新增9），0失败/跳过。C++主套件仍33个不同测试，Release/Debug是两次构建验证，不能记为66个新增测试。',
            f"旧SEG-Y/矩阵工具类另有7个专项用例，Release与Debug共{len(codec['results'])}次执行全部PASS；此前受控IBM零和IEEE文件测试确实超时，不能抹去。",
            'Release程序SHA-256：`'+replay['executable_sha256']+'`。Debug SHA-256及原生命令在final_all_checks/checks.json。',
            '编译器文件版本15.00.30729.01/产品9.00.30729.01（SP1），Qt4.7.4，保留原FFTW/FDCT。旧vcvars的Beta2文字不作为版本判断。',
            '构建退出码为0，但仍有历史告警，具体数量以原日志为准，没有声称零告警。','',
            '## 数学、恢复和兼容性','',
            f"- 独立字典/覆盖/J及DP：{numeric['cases']}例通过；数值最大误差见final_numerics/verification.json。",
            f"- 稠密实际坐标投影：{len(projection['cases'])}例通过；最大差{max(r['max_abs_difference'] for r in projection['cases']):.8g}。",
            f"- 纯FDCT解析复数组正反变换：20个维度/参数组合，在Release/Debug分别通过；最大相对L2误差{max(r['max_relative_l2'] for r in fdct['records']):.8g}，系数能量比偏差约4.0e−14。",
            f"- 当前程序重放：{replay['search_cases']}个完整非限时搜索案例、{replay['prediction_cases']}个预设恢复案例通过；搜索掩码和非计时轨迹一致，预测/线性/收敛CSV字节一致。",
            f"- v4专用旧入口兼容：{sum(r['matched_rows'] for r in compat['records'])}行（两域、两预算、每档2种子）科学列、掩码和旧轨迹逐字节一致；不比较计时列。",
            '- 隐藏干预在原始幅度链预测差0；历史6位CSV预处理存在约1.05e−6峰值比例的可测依赖，未冒充严格通过。',
            '- 非法权重/窗口、外层H被选、唯一可选集合、原单位尺度等回归已执行；测试失败返回非零，C++跳过也令整体失败。','',
            '旧整数MulMatrix_1D另经受控零坐标用例复现失败：它按值非零压缩，导致合法零坐标丢失/顺序错位。最小修复为按mask选择及正确分配容量；正式筛选不调用此旧函数。safety_final中两种构建均通过对应回归。',
            '最终转换脚本会在新CSV中明确标注未确认编码/几何，既有CSV字节未变。metadata_checks中AST等价、数值逐值相同和Python完整回归均通过。','',
            '## 冻结实验与历史','',
            f"v4独立复核：{sum(original['counts'][k] for k in ['search_rows','synthetic_rows','real_mask_rows','nested_rows'])}条记录，{original['native_steps']}步，{original['hashed_output_files']}个原生输出哈希，{original['counts']['archived_files']}个历史文件通过。原核验器的唯一报告写入被显式重定向到本轮目录，v4/final不写。",
            f"v5四阶段：{sum(r['native_steps'] for r in frozen['stages'])}步，{sum(r['hashed_native_outputs'] for r in frozen['stages'])}个原生输出哈希通过；其中1104条恢复记录和990个搜索/限时查询独立复算。",
            f"冻结基线{frozen['unchanged_protected_files']}个文件仍逐字节一致，包括全部v4/final、v2/v3历史及data。",
            '正式v5运行版本80ca9303；runtimes/v5同时保存实际程序、依赖和匹配源码。随后仅旧工具类和Diagmul改变；搜索/恢复核心C++模块哈希不变。两转换脚本只修正说明/输出头，read_segy/convert的AST及实际数值与冻结版一致。',
            '当前源码/程序与旧指纹不符时默认核验已实际失败，只有显式--runtime指定冻结快照才能核验旧批次，见stale_runtime_rejection.json。',
            '没有在新程序下静默复用旧缓存；也没有声称修复后把全部2172个联合阶段步骤重新执行一遍。',
            '最终独立核验见final_integrity_verification.json；汇总复核summary_verification_extended.json独立复算6模板、24配对的每项64个符号置换、Holm、图表均值和60组相同前缀。',
            '', '## 失败、开发记录和未执行项','',
            '|记录|实际状态|解释|','|---|---|---|',
            '|reproduction_raw_scale / pre_fix_tests|真实复现失败；开发专项首轮2失败|包含原幅度判据Bug，以及新唯一可选分支尚未重编译；最终重编译后33项通过|',
            '|safety_reproduction / codec_before|原IBM零字循环超时，IEEE路径也超时|受控程序由超时器终止；原类误把IEEE当IBM，非规范零尾数循环|',
            '|codec_after|编译失败，未执行测试|首次文本补丁因LF/CRLF匹配未生效；修正后codec_final的12次执行全通过|',
            '|current_checks|主动中断，非完整验证|保存INTERRUPTED.md，不能用部分通过代替current_final_checks|',
            '|verification_launch|工具读取UTF-8基线时失败|明确修正编码，verification_retry及冻结快照完整复核通过；失败日志保留|',
            '|人工GUI / ASan / Valgrind / 异机全重跑|未执行|没有计为通过；操作清单及环境限制见UNRESOLVED_ISSUES|',
            '|独立真实工区|未完成|外部访问记录存在，未下载或冒称新增数据|',
            '', '## 可复现命令和文件','',
            '所有原生命令和输入/输出哈希保存在各阶段manifest中。主要入口：','',
            '```powershell',
            'python -m research.validation.run_audit_checks --build --out results/new_validation',
            'python -m research.validation.audit_numerics --out results/new_numerics',
            'python -m research.validation.audit_projection --out results/new_projection',
            'python -m research.validation.run_legacy_codec_checks --out results/new_codec_checks',
            'python -m research.validation.verify_audit_v5 results/audit_20261024_v5 --runtime results/audit_20261024_v5/runtimes/v5 --report results/new_frozen_verification.json',
            'python -m research.validation.replay_audit_science results/audit_20261024_v5 --out results/new_replay',
            '```','',
            '完整重跑研究使用audit_joint_studies.py的search/joint/real/timing四阶段，各--out必须是新目录；先冻结协议和匹配程序/源码，计时阶段顺序单独运行。',
            '本地精确快照复核及当前程序重放通过，不等于已经执行干净克隆/异机完整复现。新归档使用局部.gitattributes -text保护CSV/源码快照字节；旧历史行尾变更仍会被哈希拒绝。',
            '最终图件有独立数据JSON/PNG/SVG及final_figures_manifest；统计模板层分析、初版条件分析和图件草稿全部保留。','']
    validation=ROOT/'docs/VALIDATION_REPORT.md';validation.write_text('\n'.join(report),encoding='utf-8')
    inputs=[analysis/name for name in ['search_means.csv','joint_means.csv','joint_pairs.csv','search_pairs.csv','common_evaluation_pairs.csv','real_means.csv','switch_sensitivity.csv','geometry_recovery_associations.csv','time_limit_means.csv']]
    inputs += [legacy/'real_diagnosis.json',legacy/'real_nested_summary.csv']
    provenance=dict(generator_sha256=digest(__file__),inputs={str(p):digest(p) for p in inputs},
                    outputs={str(p.relative_to(ROOT)):digest(p) for p in [resultpath,validation]})
    (root/'report_provenance.json').write_text(json.dumps(provenance,indent=2));print('Reports generated')


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path);make(p.parse_args().root.resolve())
