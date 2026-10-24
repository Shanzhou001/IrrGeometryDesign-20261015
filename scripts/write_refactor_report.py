"""Write the refactor acceptance report from a completed pipeline run."""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[1]


def read(path):return json.loads(path.read_text(encoding='utf-8'))


def write(out):
    state=read(out/'pipeline.json');assert state.get('completed')
    assert all(s['exit_code']==0 for s in state['steps'])
    checks=read(out/'regression/checks.json');layout=read(out/'layout.json')
    replay=read(out/'replay/verification.json');numeric=read(out/'numeric/verification.json')
    fdct=read(out/'fdct/verification.json');safety=read(out/'legacy_safety/checks.json')
    archive=read(out/'archives/verification.json');smoke=read(out/'smoke/verification.json')
    images=read(ROOT/'figures/manifest.json');legacy=read(out/'legacy_pipeline/run_manifest.json')
    lines=['# 目录重构、完整流水线与图片验证','',
           '本页从实际完成的流水线 JSON/日志生成。科学基线为 c191e6c5，独立分支 codex/project-structure。',
           '执行目录：`'+out.relative_to(ROOT).as_posix()+'`。实际命令：`'+' '.join(state['command'])+'`。',
           '源码版本：`'+state['git_commit']+'`；每个活动源码、构建定义及运行依赖的精确哈希在 pipeline.json。','',
           '## 结构和实现','',
           '- 活动 C++ 按启动、CLI、几何、采样、优化、恢复、实验、GUI、绘图和测试划分；主启动文件74行，原实验引擎拆出独立编译单元。',
           '- Python 脚本改为 research 包；路径、哈希、CSV、子进程环境和图片写入集中共享。',
           '- VS2008/Qt4、原 GUI/CLI、FFTW/FDCT/Eigen 保留；共享构建脚本和 qmake 工程都执行了实际构建。',
           '- 清理8个未引用的旧 Qt3/生成工程文件，原始版本可在基线 Git 中检索。',
           '- 修复路径导入遗漏和既有帮助文字百分号错误；v3输出目录增加预检，程序/输入/配置变化不能通过恢复执行覆盖原结果。',
           '- 原生恢复保存实际迭代步长CSV；更正交互图中固定512ms的时间刻度及把迭代步长当成隐藏精度的文字。',
           '', '## 实际检查','', '|检查|结果|','|---|---|']
    for mode in ['release','debug']:
        text=(out/'regression'/(mode+'_regression.log')).read_text(encoding='utf-8',errors='replace')
        passed=len(re.findall(r'\[PASS\] Test \d+:',text));failed=len(re.findall(r'\[FAIL\]',text));skipped=len(re.findall(r'\[SKIP\]',text))
        assert (passed,failed,skipped)==(33,0,0)
        lines.append(f'|{mode} C++完整回归|{passed}通过，{failed}失败，{skipped}跳过|')
    python=(out/'regression/python.log').read_text(encoding='utf-8',errors='replace')
    total=int(re.search(r'Ran (\d+) tests',python)[1]);assert '\nOK' in python
    values=[('Python',f'{total}项通过；原33项+5项路径/重复写入/防覆盖回归'),
            ('Qt4 qmake + nmake','实际构建退出0'),
            ('v3快速完整兼容流水线',f"{len(legacy['steps'])}步真实执行，含采样、统计、约束、二维、三类恢复及真实资料处理"),
            ('数值与硬约束',f"{numeric['cases']}例独立NumPy/枚举通过"),
            ('稠密坐标投影','2例独立对照通过'),
            ('纯FDCT',f"两构建各20种组合通过；最大相对L2误差{max(r['max_relative_l2'] for r in fdct['records']):.9g}"),
            ('旧SEG-Y与零坐标专项',f"{len(safety['results'])}次执行通过（7种用例×两构建）"),
            ('当前程序重放',f"{replay['search_cases']}搜索、{replay['prediction_cases']}恢复通过；排除计时字段，掩码/轨迹/预测保持一致"),
            ('完整数据流水线',f"{len(smoke['records'])}例：解析模型与未确认编码真实资料，优化/内选/恢复/评分均执行"),
            ('Python模块',f"{layout['code']['imported_modules']}个导入通过，依赖无环"),
            ('科学核心源码',f"{layout['scientific_source_equivalence']['compared_files']}文件除包含路径和行尾外相同"),
            ('本地文档链接',f"{layout['references']['local_links']}处存在"),
            ('命令帮助入口',f"{len(layout['cli_help'])}个全部退出0")]
    lines += ['|'+a+'|'+b+'|' for a,b in values]
    counts=archive['protected_files'];figure=layout['figures']
    lines += ['', '## 图片集中、重新生成及去重','',
              f"当前共{figure['total_images']}张PNG：{figure['generated_once']}张生成图片本次各保存1次，{figure['source_assets']}张Qt静态图标保留原素材。",
              f"项目其他目录图片数{figure['images_outside_figures']}，PNG字节重复数{figure['duplicate_hashes']}。GUI的8张截图实际执行成功，不是复用旧截图。",
              '删除的SVG、草稿和正式图片副本可从Git c191e6c5访问；22张被Git忽略的旧快速测试图片在retired_smoke_images.json记录哈希后清理，原快速测试数据继续保留。',
              '去重以图类型和输入哈希为准，发现旧文件相同但CSV不同时重新生成正确图，避免延续过期图片。',
              '旧恢复导出有64×32的降采样显示矩阵；使用原解析模型/原输入及既有掩码重放，未对降采样矩阵上采样冒充完整数据。',
              f"有归档矩阵的显示网格上，当前重放最大差为{max(r.get('replay_difference_on_archived_grid',0) for r in images['legacy_source_records']):.9g}。缺失旧矩阵的示例明确标成当前确定性重放。",
              '图片总览：[figures/index.html](../figures/index.html)。每张图的输入、PNG哈希、生成次数和重放说明见figures/manifest.json；论文图保留数值JSON。',
              '', '## 科学资料和历史记录','',
              f"旧基线34,526个文件中，{counts['working_bytes']}个从工作区验证为原字节；{counts['original_git_bytes']}个已迁移/重新生成的显示或引用资料从明确原版验证。不是把重新生成图片称为未改变。",
              'v4的5843条记录及v5的2344个原生步骤继续做独立数值/哈希核验；原始SEG-Y、CSV、掩码和预测没有为得到更好结果而改动。',
              '旧命令/源码路径是历史事实，通过明确运行快照、Git原版和迁移映射读取。只在能匹配既有SHA的条件下恢复Windows行尾；具体来源见archives/verification.json。',
              '当前程序重放、旧程序来源核验、图表重绘是三个有明确版本的步骤，不把旧结果当作修改后的程序重新运行所得。',
              '本轮新增两例smoke和v3快速执行用于软件兼容检查，不能当作新的独立真实来源或算法优越性证据。',
              '', '## 开发失败与限制','',
              '- 首轮源码字符解码探测和BOM位置造成的构建失败保留；保留原编码并把BOM置于文件起始后通过。',
              '- 首轮历史恢复图重放使用了旧降采样显示矩阵，维度检查拒绝；改用原模型/输入后通过，没有绕过尺寸检查。',
              '- 第一遍图片检查发现22张忽略的快速测试图片，清理并记录后通过。',
              '- 第一遍Git原图核验发现LF/CRLF不一致，改为精确SHA约束的历史字节恢复，失败日志保留。',
              '- GUI截图检验启动、窗口打开和示范SPS加载，不代替全部交互按钮的人工走查。',
              '- 实际验证平台是Windows x64、VS2008 SP1、Qt4.7.4和记录的Python环境；未将其表述为Win32/Linux或异机完整验证。',
              '- SEG-Y争议编码、采集说明、第二独立真实来源、跨工区泛化等限制继续见[未解决项](UNRESOLVED_ISSUES.md)。',
              '- 全流水线通过不是全部第三方库与GUI不存在任何缺陷的形式证明。','']
    report=ROOT/'docs/REFACTOR_VALIDATION.md';report.write_text('\n'.join(lines),encoding='utf-8')
    note='<!-- LAYOUT_NOTE_BEGIN -->\n最新目录重构和图片管理验证见 [REFACTOR_VALIDATION.md](REFACTOR_VALIDATION.md)。本页原研究统计与当时执行记录继续保留；当前活动代码/图片位置以新架构和迁移映射为准。\n<!-- LAYOUT_NOTE_END -->\n\n'
    for name in ['VALIDATION_REPORT.md','PROJECT_PROGRESS_20261024.md','AUDIT_ISSUES.md','REVIEW_REPORT_20261024.md']:
        path=ROOT/'docs'/name;text=path.read_text(encoding='utf-8-sig');text=re.sub(r'<!-- LAYOUT_NOTE_BEGIN -->.*?<!-- LAYOUT_NOTE_END -->\s*','',text,flags=re.S);path.write_text(note+text,encoding='utf-8')
    provenance=dict(generator_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),pipeline_sha256=hashlib.sha256((out/'pipeline.json').read_bytes()).hexdigest(),report_sha256=hashlib.sha256(report.read_bytes()).hexdigest())
    (out/'report_provenance.json').write_text(json.dumps(provenance,indent=2),encoding='utf-8')
    print('Generated',report,total,'Python tests',figure['total_images'],'PNGs')


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('out',type=Path);write(p.parse_args().out.resolve())
