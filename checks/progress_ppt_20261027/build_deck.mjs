import fs from 'node:fs/promises';
import path from 'node:path';
import {spawnSync} from 'node:child_process';
import {createHash} from 'node:crypto';

const root='D:/Desktop/sci';
const project=path.join(root,'IrrGeometryDesign');
const build=path.join(project,'checks/progress_ppt_20261027');
const out=path.join(root,'delivery/研究进展汇报.pptx');
await fs.mkdir(build,{recursive:true});
const csv=async relative=>{
  const lines=(await fs.readFile(path.join(project,relative),'utf8')).trim().split(/\r?\n/);
  const headers=lines.shift().split(',');
  return lines.map(line=>Object.fromEntries(line.split(',').map((value,i)=>[headers[i],value])));
};
const viking=await csv('results/viking_robust_20261026/analysis/overall_means.csv');
const quality=await csv('results/viking_quality_20261027/waveform_means.csv');
const f3=await csv('results/f3_transfer_20261027/recovery.csv');
const search=await csv('results/repair_20261026/analysis/search_pairs.csv');
const common=await csv('results/repair_20261026/analysis/common_evaluation_pairs.csv');
const mean=(rows,key)=>rows.reduce((sum,r)=>sum+Number(r[key]),0)/rows.length;
const recovery=(m,kind)=>mean(viking.filter(r=>Number(r.m)===m&&r.recovery===kind),'mean_snr');
const qual=(m,key)=>mean(quality.filter(r=>Number(r.m)===m&&r.recovery==='selected'),key);
const significant=rows=>rows.filter(r=>r.comparison==='Adaptive-Hybrid'&&Number(r.p_holm)<.05).length;
if(f3.length!==270||f3.filter(r=>r.selected==='linear').length!==215)throw Error('F3 factual check failed');
const C={navy:'#102D3D',teal:'#096A70',body:'#233C4A',muted:'#667A86',line:'#D8E1E5',light:'#F2F6F7',white:'#FFFFFF',warn:'#A14F30'};
const text=(name,value,x,y,w,h,size=18.5,color=C.body,bold=false)=>({type:'text',name,value,x,y,w,h,size,color,bold});
const image=(name,relative,x,y,w,h,alt,cropRight=0)=>({type:'image',name,path:path.join(project,relative),x,y,w,h,alt,cropRight});
const table=(name,rows,x,y,widths,heights,size=17)=>({type:'table',name,rows,x,y,w:widths.reduce((a,b)=>a+b),h:heights.reduce((a,b)=>a+b),widths,heights,size});
const title=value=>text('slide-title',value,40,31,880,50,32,C.navy,true);
const foot=(value,page)=>[text('scope-note',value,40,508,830,21,10.5,C.muted),text('page-number',`${page} / 5`,886,508,40,18,10.5,C.muted)];
const sourceNotes=(files,extra='')=>'数据与图件来源：\n'+files.map(f=>path.join(project,f)).join('\n')+'\n\n'+extra;
const slides=[];
slides.push({name:'研究概况',elements:[
  text('cover-title','论文研究进展汇报',40,40,880,65,42,C.navy,true),
  text('project-title','基于混合优化算法的地震采样点智能筛选方法研究',42,118,875,45,22,C.body),
  text('presenter','吴善州    2026年10月27日',43,170,830,26,16,C.teal),
  image('measured-gathers','data/public/viking_line12_small/preview.png',45,212,870,306,'Viking Line12实测炮集，三个固定时间窗，保留完整坐标与显示说明')
],notes:sourceNotes(['README.md','data/public/viking_line12_small/preview.png'],
  '当前为研究进展汇报。主任务是在固定保留道数和评价预算下研究几何筛选与缺失道恢复。图为FFID53、453、953的实测炮集固定窗口，显示采用逐图分位数截幅，存储数据未因此改变。当前尚未完成学校格式定稿。')});

slides.push({name:'已完成工作',elements:[title('已完成的研究与系统实现'),
  image('sps-interface','figures/gui/02_survey_loaded.png',40,110,520,360,'原生程序读取SPS观测系统后的实际界面'),
  text('algorithm-label','算法与恢复',592,111,328,29,20,C.teal,true),
  text('algorithm-summary','固定道数与预算控制\n覆盖约束与观测内恢复选择',592,150,328,66,18),
  text('experiment-label','实验与资料',592,233,328,29,20,C.teal,true),
  text('experiment-summary','合成 1980行搜索、1080行恢复\nViking 1215例稳定性实验\nF3 270例冻结迁移',592,270,328,91,17.5),
  text('verification-label','软件与复现',592,378,328,29,20,C.teal,true),
  text('verification-summary','109项通过，4项旧资料测试跳过\n中文21页、英文15页初稿\n最终包完成五方法解压实测',592,416,328,77,17.5),
  ...foot('图为实际SPS读入界面。二维几何应用与一维波形恢复分别评价。',2)
],notes:sourceNotes(['figures/gui/02_survey_loaded.png','results/repair_20261026/INDEX.json','results/viking_robust_20261026/manifest.json','results/f3_transfer_20261027/manifest.json','checks/closure_20261027/project/checks.json'],
  'Python共113项测试，109通过，4项跳过均依赖未安装的可选历史RRAW资料。Release/Debug数值、投影、端到端、登记Viking及SPS集成通过。最终交付在同一Windows主机的新建虚拟环境、无Git/无外部Qt路径临时目录完成5个案例和62次原生调用，未重跑全部历史实验。SPS包含9072个检波点，不构成二维波形成像验证。')});

const summary=[['保留道数','固定冷曲波','固定f-k','内部选择'],...[36,60,84].map(m=>[String(m),...['cold','fk','selected'].map(k=>recovery(m,k).toFixed(3))])];
const gains=[36,60,84].map(m=>(recovery(m,'selected')-recovery(m,'cold')).toFixed(3));
slides.push({name:'Viking实验结果',elements:[title('Viking：恢复选择改善平均预测'),
  image('viking-recovery','figures/research/viking_robust_20261026/recovery.png',40,98,880,256,'Viking稳定性实验原图，固定曲波、独立f-k和观测内选择的三组结果'),
  text('snr-table-caption','共同隐藏道平均SNR（dB）',40,358,560,28,17.5,C.teal,true),
  table('editable-viking-means',summary,40,390,[104,150,144,162],[27,28,28,28],17),
  text('gain-label','相对固定冷曲波提高',638,371,282,27,17.5,C.teal,true),
  text('gain-values',gains.join(' / ')+' dB',638,408,282,31,19,C.navy,true),
  text('high-density-limit','84道时，固定f-k仍略高于\n内部选择',638,452,282,46,17.5,C.warn),
  ...foot('1215例，同测线3种子、双内层分组。表内五种布局等权，布局收益与恢复选择收益需区分。',3)
],notes:sourceNotes(['figures/research/viking_robust_20261026/recovery.png','results/viking_robust_20261026/analysis/overall_means.csv'],
  '三档为36、60、84道，对120候选道分别保留30%、50%、70%，包含12个强制先导道。内部选择用八个固定候选和[0,1]幅度系数，只读取观测道内部验证结果。固定曲波是冷启动40次，固定f-k为弱正则80次。五布局均值先按冻结条件汇总。恢复选择的增益不能全部归因于混合布局。最高密度下固定f-k为5.5544dB，内部选择为5.4869dB。')});

const rms=[36,60,84].map(m=>qual(m,'mean_rms_ratio').toFixed(3));
const phase=[36,60,84].map(m=>qual(m,'mean_band_phase_mae_degrees').toFixed(2));
slides.push({name:'外部验证与波形问题',elements:[title('F3迁移与波形质量：收益具有条件性'),
  text('external-label','新增F3叠后工区',40,98,430,28,20,C.teal,true),
  image('f3-transfer','figures/research/f3_transfer_20261027/recovery.png',40,143,430,218,'F3固定方案外部迁移运行原图，五种布局的平均共同隐藏SNR'),
  text('f3-findings','Jitter三档平均分均最高\n270例中215例选择线性插值',40,381,430,64,18.5,C.body,true),
  text('f3-limit','外部结果尚未证明混合方法普遍更优',40,461,430,31,18,C.warn),
  text('waveform-label','Viking隐藏道波形示例',515,98,405,28,20,C.teal,true),
  image('waveform-left-panel','figures/research/viking_robust_waveforms_20261026/waveform.png',515,143,405,264,'原始运行图左侧的固定隐藏道波形子图，完整源图作为图片嵌入，可取消裁切查看残差子图',0.50),
  text('waveform-rms','RMS比理想值1（36 / 60 / 84道）',515,420,405,26,17,C.teal,true),
  text('rms-values',rms.join(' / '),515,451,405,29,21,C.navy,true),
  text('phase-values','相位MAE：'+phase.join('° / ')+'°',515,481,405,25,17,C.body),
  ...foot('F3三个邻近剖面属于同一新增工区。波形误差针对带噪实测道，物理保幅与成像效果尚未验证。',4)
],notes:sourceNotes(['figures/research/f3_transfer_20261027/recovery.png','results/f3_transfer_20261027/means.csv','results/f3_transfer_20261027/recovery.csv','figures/research/viking_robust_waveforms_20261026/waveform.png','results/viking_quality_20261027/waveform_means.csv'],
  'F3方案在读取波形值前冻结，270案例、126搜索、2496次成功原生调用。三邻近剖面是一个新增工区的相关条件，未按外层成绩调参。215例选线性插值，55例选暖曲波。右图显示原波形图的左侧子图，示例为预先指定的FFID453、2.048至3.072秒窗口、随机H、m60、seed94217、index60；示例未按最高分挑选。下方RMS与相位数值来自全部1215案例的内部选择，不是该单个示例。RMS比理想值1。相位在0.02至0.24周期/采样点、去均值/Hann窗后按目标谱能量加权。两个资料源绝对分数不能相减解释算法增益。')});

slides.push({name:'问题与下一步',elements:[title('当前问题与下一步工作'),
  image('budget-control','figures/research/repair_20261026/budget.png',40,118,480,172,'合成实验预算机制对照原图，终点和相同实际评价次数的配对差异'),
  text('budget-label','预算机制已有实测支持',40,316,480,30,20,C.teal,true),
  text('budget-summary',`对旧Hybrid：终点${significant(search)}/6组显著\n相同实际评价次数下仅${significant(common)}/6组显著`,40,360,480,65,18.5),
  text('budget-conclusion','继续使用剩余预算是明确收益来源\n稳定超越SA的证据仍不足',40,446,480,55,18,C.body),
  table('editable-problem-plan',[
    ['当前问题','下一步工作'],
    ['算法优势\n尚未稳定优于SA','明确预算利用贡献\n分离布局与恢复作用'],
    ['外部证据\n新增工区仅F3','保持方案冻结\n扩大独立工区验证'],
    ['论文定稿\n总论与新增结果\n未完全同步','更新摘要与结论\n完善文献与学校格式']
  ],550,116,[153,217],[32,105,105,134],17),
  ...foot('现有数据支持预算利用、覆盖可行性与条件差异分析。全文需保持负结果与适用范围一致。',5)
],notes:sourceNotes(['figures/research/repair_20261026/budget.png','results/repair_20261026/analysis/search_pairs.csv','results/repair_20261026/analysis/common_evaluation_pairs.csv','manuscript_sources/chinese/thesis.md','manuscript_sources/english/manuscript.md'],
  '显著性按24项比较族Holm校正。Adaptive对旧Hybrid终点4/6组显著，同实际次数1/6；对NoRecycle终点6/6组显著，同实际次数六组差均为0。对SAOnly及SA_Budget未建立稳定显著优势。合成恢复以6个结构模板为单位，24项校正比较均不显著。第4章已加入F3与波形诊断，第6章、英文结论与附录仍需充分同步。优先使用现有数据解释贡献，不根据已见F3成绩反复选择最有利设置。')});

const data={title:'地震采样点筛选研究进展',author:'吴善州',width:960,height:540,font:'Microsoft YaHei',farEastFont:'微软雅黑',colors:C,slides,output:out,build};
if(slides.length!==5)throw Error('Exactly five slides required');
const hashes={};
for(const slide of slides)for(const element of slide.elements)if(element.type==='image')hashes[element.path]=createHash('sha256').update(await fs.readFile(element.path)).digest('hex');
await fs.writeFile(path.join(build,'deck_data.json'),JSON.stringify(data,null,2),'utf8');
await fs.writeFile(path.join(build,'source_images.json'),JSON.stringify(hashes,null,2),'utf8');
const execution=spawnSync('powershell.exe',['-NoProfile','-ExecutionPolicy','Bypass','-File',path.join(build,'native_author.ps1'),'-DataPath',path.join(build,'deck_data.json')],{encoding:'utf8',windowsHide:true,maxBuffer:1024*1024*4});
await fs.writeFile(path.join(build,'author.log'),execution.stdout+'\n'+execution.stderr,'utf8');
if(execution.status!==0)throw Error(execution.stdout+'\n'+execution.stderr);
console.log(execution.stdout.trim());
