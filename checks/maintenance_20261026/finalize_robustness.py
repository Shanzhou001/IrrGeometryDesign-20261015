"""Write the final robustness report and bilingual manuscript inserts from CSVs."""
import csv
from datetime import datetime
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]


def read(path):
    with path.open(encoding='utf8',newline='') as stream:return list(csv.DictReader(stream))


def table(headers,records):
    return '\n'.join(['| '+' | '.join(headers)+' |','| '+' | '.join('---' for _ in headers)+' |']+
                     ['| '+' | '.join(map(str,r))+' |' for r in records])


def main():
    root=ROOT/'results/viking_robust_20261026';folder=Path(__file__).resolve().parent
    manifest=json.loads((root/'manifest.json').read_text());audit=json.loads((folder/'robust_archive_audit/verification.json').read_text())
    assert manifest.get('completed') and manifest.get('status')=='passed' and audit['status']=='passed'
    means=read(root/'analysis/overall_means.csv');raw=read(root/'recovery.csv')
    methods=['Jitter','SA_Mu','Adaptive_Mu','Adaptive_Gap','Adaptive_Signal']
    values={(int(r['m']),r['method'],r['recovery']):float(r['mean_snr']) for r in means}
    avg=lambda m,k:sum(values[m,method,k] for method in methods)/len(methods)
    rec=[[m]+[f'{avg(m,k):.4f}' for k in ['cold','fk','selected']]+[f'{avg(m,"selected")-avg(m,"cold"):+.4f}'] for m in [36,60,84]]
    method_table=[[m]+[f'{values[m,method,"selected"]:.4f}' for method in methods] for m in [36,60,84]]
    effects=[[m,f'{values[m,"Adaptive_Gap","selected"]-values[m,"Adaptive_Mu","selected"]:+.4f}',
              f'{values[m,"Adaptive_Signal","selected"]-values[m,"Adaptive_Gap","selected"]:+.4f}'] for m in [36,60,84]]
    fold=json.loads((folder/'inner_fold_comparison.json').read_text())
    fold_table=[[r['m'],f'{r["one_fold_mean_snr"]:.4f}',f'{r["two_fold_mean_snr"]:.4f}',f'{r["mean_delta_snr"]:+.4f}'] for r in fold['records']]
    seed_table=[]
    for m in [36,60,84]:
        per_seed=[sum(float(r['selected_snr']) for r in raw if int(r['m'])==m and int(r['seed'])==seed)/
                  sum(int(r['m'])==m and int(r['seed'])==seed for r in raw) for seed in manifest['configuration']['plan']['seeds']]
        seed_table.append([m]+[f'{v:.4f}' for v in per_seed])
    seconds=(datetime.fromisoformat(manifest['completed_utc'])-datetime.fromisoformat(manifest['started_utc'])).total_seconds()
    text=f'''# 多种子与双内层分组稳定性结果

日期：2026-10-26。完整执行 **{len(raw)} 个恢复案例、{len(read(root/'search.csv'))} 个唯一搜索、{manifest['native_calls']} 次原生调用**，批次与独立归档复核均通过。运行约{seconds/60:.1f}分钟，16个工作线程；不是单次搜索计时基准。

## 协议

保持首轮v2的3炮、3时间窗、3组H、3保留数、5方法、预算300、先导12道及8个恢复候选。掩码种子固定为84011/94217/105953，每案两个不重叠内层验证分组。阈值和候选未按本批外层成绩重调，全部预设条件保留。

这是观察已有结果后的同测线计算稳定性研究，未增加独立工区。掩码种子同时决定布局和配对内层随机划分，种子间差异不能解释为纯布局效应。两个分组减少对单一验证划分的依赖，不保证改善外层恢复。

## 恢复均值

在五种布局及全部预设条件上等权汇总，单位dB：

{table(['m','固定冷曲波','固定弱f-k','双分组内部选择','选择−冷曲波'],rec)}

## 各搜索方法

{table(['m']+methods,method_table)}

{table(['m','Gap−Mu 内部选择','Signal−Gap 内部选择'],effects)}

这些是描述性均值，没有将相关条件或种子当成独立工区做显著性推断。

## 种子敏感性

下表每个数在同一掩码种子的全部方法、炮、窗口及H上平均：

{table(['m','84011','94217','105953'],seed_table)}

逐方法的种子均值见[mask_seed_means.csv](results/viking_robust_20261026/analysis/mask_seed_means.csv)。不把种子数乘入独立工区数。

## 严格匹配的单/双内层分组比较

只比较首轮v2与本轮共有的84011种子，共405案例。掩码、先导、H、程序、第一内层划分均相同，固定恢复最大SNR差为{fold['fixed_recovery_max_snr_difference']:.3g}，由此避免把换布局混入分组效果。

{table(['m','单分组','双分组','双−单'],fold_table)}

其他两个种子没有对应单分组基线，不从三种子总体与旧单种子总体的差推断因果提升。

## 核验与材料

独立复核{audit['checked_unique_files']}个文件、{audit['checked_prediction_arrays']}份预测，观测投影最大误差为{audit['projection_max_error']:.3g}，重算指标最大误差约{audit['score_max_error']:.3g}。全部351搜索的全频/加权分数、预算、覆盖上限、必留先导、共同隐藏和内部划分均核对，实际振幅干预隔离通过。

- [实际完整清单](results/viking_robust_20261026/manifest.json)
- [自动汇总](results/viking_robust_20261026/analysis/summary.md)
- [审计](checks/maintenance_20261026/robust_archive_audit/verification.json)
- [匹配分组对照](checks/maintenance_20261026/inner_fold_comparison.json)
- [主要图集](figures/research/viking_robust_20261026/index.html)
- [预先指定的波形示例](figures/research/viking_robust_waveforms_20261026/index.html)

示例固定FFID453、起始512、H=r271828、m60、种子94217，未按外层分数挑选。含噪实测道是评价目标，不据此推断无噪真值、绝对保幅或成像收益。坐标单位码冲突、绝对标定和单测线边界仍保留。旧RRAW档案未被替代为已复现。
'''
    (ROOT/'VIKING_ROBUST_20261026.md').write_text(text,encoding='utf8')
    root_summary=' / '.join(f'{avg(m,"selected"):.4f}' for m in [36,60,84])
    paper=ROOT.parent/'paper-tgrs'
    en=f'''The completed supplementary batch contains {len(raw)} recovery cases, 351 unique searches and {manifest['native_calls']} successful native calls. It retains the same shots, windows, hidden sets, counts, pilot cost and candidate pool, using mask seeds 84011, 94217 and 105953 with two disjoint inner-validation groups. Aggregate selected SNR is {root_summary} dB at m=36/60/84. These results characterize computational sensitivity in the existing development survey, not an independent field confirmation. Mask seeds also change paired inner permutations, so between-seed differences are not pure layout effects.

**Supplementary Table S1. Three-seed, two-group recovery means, dB, averaged equally over the five layout methods.**

{{{{table:robust_recovery}}}}

**Supplementary Table S2. Internally selected SNR by layout method in the same robustness protocol.**

{{{{table:robust_methods}}}}

The exact 84011 overlap has 405 matched cases: layouts, pilot/H masks and the first validation split agree with the original one-group experiment. Fixed-recovery SNR differs by at most {fold['fixed_recovery_max_snr_difference']:.3g} dB. Two-group minus one-group selected means at m=36/60/84 are {' / '.join(f'{r["mean_delta_snr"]:+.4f}' for r in fold['records'])} dB. Additional validation is therefore reported as a measured sensitivity, not presumed to improve every condition. No cross-survey confidence interval or significance claim is made.

![Supplementary Figure S1. Fixed illustrative middle hidden trace, FFID453, window2.048-3.072s, random H, m60, seed94217. The case was specified before completion; it is not a highest-score example.](assets/viking_robust_waveform.png)
'''
    zh=f'''补充批次已完整完成{len(raw)}个恢复案例、351个唯一搜索、{manifest['native_calls']}次成功原生调用。保持相同炮、窗口、H、采样数、先导成本与候选池，固定种子84011/94217/105953，每案两个不重叠内层分组。三档内部选择平均SNR为{root_summary}dB。它描述同一开发测线上的计算敏感性，不是独立外部确认。种子同时改变布局和内部随机排列，不能把种子差异归结为纯布局影响。

**补充表S1 三种子、双分组的恢复均值，单位dB。**

{{{{table:robust_recovery}}}}

**补充表S2 同一稳定性协议的逐布局方法内部选择SNR。**

{{{{table:robust_methods}}}}

严格匹配的84011种子共405案例，其掩码、先导、H和第一内层划分均与首轮一致，固定恢复SNR最大差为{fold['fixed_recovery_max_snr_difference']:.3g}dB。双分组相对单分组的三档内部选择均值差为{' / '.join(f'{r["mean_delta_snr"]:+.4f}' for r in fold['records'])}dB。因此增加分组后的结果作为实测敏感性报告，不预设每个条件必然改善。其他种子没有对应单分组对照，不能将总体差值作为分组因果收益。

![补充图S1 预先指定的隐藏道波形示例，FFID453、2.048至3.072秒、随机H、m60、种子94217。案例未按外层得分挑选。](../paper-tgrs/assets/viking_robust_waveform.png)
'''
    (paper/'robustness.md').write_text(en,encoding='utf8');(paper/'robustness_zh.md').write_text(zh,encoding='utf8')
    summary=dict(recovery_cases=len(raw),unique_searches=351,native_calls=manifest['native_calls'],runtime_minutes=seconds/60,
                 recovery_means=rec,method_means=method_table,paired_effects=effects,seed_means=seed_table,fold_comparison=fold_table)
    (folder/'robust_summary.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf8')
    print(json.dumps(summary,indent=2))


if __name__=='__main__':main()
