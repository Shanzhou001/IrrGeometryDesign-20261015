import fs from 'node:fs/promises';
import path from 'node:path';
import crypto from 'node:crypto';
import {pathToFileURL} from 'node:url';
import {createRequire} from 'node:module';
const opts={};for(let i=2;i<process.argv.length;i+=2)opts[process.argv[i].replace(/^--/,'')]=process.argv[i+1];
if(process.argv.includes('--help')){console.log('Rebuild the frozen seven-slide reference: --root PROJECT --template PPTX --output PPTX --build DIR --skill-dir DIR --runtime-modules DIR --python EXE');process.exit(0);}
for(const key of ['root','template','output','build','skill-dir','runtime-modules','python'])if(!opts[key])throw new Error('Missing --'+key);
const root=path.resolve(opts.root).replaceAll('\\','/');
await fs.mkdir(opts.build,{recursive:true});
const require=createRequire(path.join(opts['runtime-modules'],'_runtime_loader.cjs'));
const {FileBlob,PresentationFile}=await import(pathToFileURL(require.resolve('@oai/artifact-tool')).href);
const build=path.resolve(opts.build);
const source=path.resolve(opts.template);
const deck=await PresentationFile.importPptx(await FileBlob.load(source));
const text=(id,value)=>{deck.resolve(id).text=value;};
text('sh/547294r6','研究进展与实验核验');
text('sh/9072xkry','研究进展与实验核验');
text('sh/fu94fe98','地震采样布局的预算约束优化与缺失道恢复适用条件研究');
text('sh/ozy1ofad','地震采样布局的预算约束优化与缺失道恢复适用条件研究');
text('sh/ts7md4r2',"本研究在已采集地震数据上模拟保留部分道，评价预算约束下的布局搜索与缺失道恢复。采用模拟退火与离散局部交换搜索，包含自适应切换和预算回收。旧CG名称仅作接口兼容。\n\n历史5148例结果保留。20结构主要比较有1项显著正差、3项显著负差。两工区空间复验的收益依赖保留率，互干度降低不能保证SNR提高。\n\n本轮完成2592例，包含同预算随机搜索和Parihaka前瞻验证。128道时Hybrid相对Jitter、SA六块均正，对随机搜索五块为正。\n\n当前报告条件性收益。原9072点SPS缺对应波形，物理保幅、成像与现场成本尚待验证。");
text('sh/b29kza94','Viking测线的三个炮集，每个窗口包含120道、256个原生采样点。图中显示空间连续的波形结构。显示采用各面板的99%分位截幅，原始数字振幅保持不变；这些炮集来自同一工区。');
text('sh/n6ls3alk','五种布局分别配合固定冷曲波、固定f-k与内层选择恢复。表中为五种布局等权平均SNR，单位为dB。保留道数增加时，平均重建质量提高。\n\n布局排名随恢复方式变化；恢复配置选择的收益单独归因。84道时，内层选择平均值略低于固定f-k，负结果继续保留。');
text('sh/n2l4fq98','3 / 7');
deck.resolve('sh/n2l4fq98').position={left:1181,top:677,width:70,height:25};
text('sh/0ba143al','F3已见工区重复迁移');
text('sh/i94r6xgz','沿用历史冻结方案，在已访问的F3叠后资料上重复验证。36、60、84道条件下，Jitter的内层选择平均SNR均最高。\n\n270例中215例选择线性插值。当前结果未建立自适应布局的稳定优势；本轮重算不能恢复未见测试身份。');
text('sh/w72947yt','历史预设隐藏道的数字振幅对照。黑线为实测目标，彩线为不同布局的预测。预测存在峰谷幅度与出现时间偏差。\n\n波形评价同时保留RMS比、相关性和相位误差。数字波形相似度不能建立物理保幅或下游成像结论。');
text('sh/doj29oba','布局收益与恢复器收益分别评价');
text('sh/yhg7epsj',"预算贡献与结论边界");
text('sh/pc76hkr2','6 / 7');
text('sh/oryp8fah',"旧Hybrid的部分最终优势在匹配实际评价次数后减弱。NoRecycle共同阶段差值为零，预算回收作用单列。\n\n新增随机搜索只按互干度选择布局，全部候选保留。新工区的主要比较结果见上一页。");
const table=deck.resolve('tb/9cvmlsby');
for(const [r,c,value] of [[0, 0, "前瞻验证"], [0, 1, "Parihaka，128道"], [1, 0, "Hybrid－Jitter\nHybrid－SA"], [1, 1, "+0.374 / +0.283 dB\n均为6/6块正差"], [2, 0, "Hybrid 对\n同预算随机搜索"], [2, 1, "+0.169 dB，5/6块正差\n未满足六块均正"]])table.cells.set(r,c,value);
const table2=deck.resolve('tb/it83epwj');
for(const [r,c,value] of [[0, 0, "已有证据"], [0, 1, "结论边界"], [1, 0, "SA、Hybrid\n及随机搜索\n各300次目标评价"], [1, 1, "评价次数相同\n运行时间单独解释"], [2, 0, "9072点SPS\n仅有几何资料"], [2, 1, "物理保幅、成像与\n实际采集降本待验证"]])table2.cells.set(r,c,value);
const title=deck.slides.items[4].shapes.add({geometry:'textbox',name:'validation-scope-title',position:{left:53.33,top:41.33,width:1173.33,height:66.67},fill:'none',line:{fill:'none',width:0}});
title.text='前瞻外部验证与公平预算';title.text.style={fontFamily:'微软雅黑',fontSize:42.67,bold:true,color:'#102D3D'};
const scope=deck.slides.items[4].shapes.add({geometry:'textbox',name:'prospective-scope',position:{left:53,top:190,width:295,height:350},fill:'none',line:{fill:'none',width:0}});
scope.text="Parihaka首次用于本项目。\n\n读取波形前冻结128道主条件。\n\n六块一致性是描述性标准，未作总体显著性检验。\n\n对随机搜索未满足预定标准的结果继续保留。";scope.text.style={fontFamily:'微软雅黑',fontSize:22,color:'#233C4A'};
for(const i of [1,3,4,6]){
 const footer=deck.slides.items[i].shapes.add({geometry:'textbox',name:'updated-page-number',position:{left:1181,top:677,width:70,height:25},fill:'none',line:{fill:'none',width:0}});
 footer.text=`${i+1} / 7`;footer.text.style={fontFamily:'微软雅黑',fontSize:14,color:'#667A86'};
}
const pictures=deck.slides.items[3].images.items;
const left=pictures.find(p=>p.position.left<600);
if(!left)throw new Error('F3 source image not found');
const bytes=await fs.readFile(root+'/data/templates/f3_recovery.png');
left.replace({blob:bytes.buffer.slice(bytes.byteOffset,bytes.byteOffset+bytes.byteLength),contentType:'image/png',fit:'contain',alt:'F3 repeated transfer; historical frozen results'});
for(const slide of deck.slides.items)slide.speakerNotes.text="方法为SA与离散局部交换搜索，CG为兼容名。当前题目与研究范围见RESEARCH_SCOPE.md。历史数据与图表来自results/completion_20261103及results/current，原始5148例中部分阶段按身份复用。新增results/closure_20261104为2592例重算，包括1296例既有条件复验、432例新增随机对照和864例新工区。源协议closure_protocol.json在Parihaka波形访问前冻结，传输修订仅改用同一文件预定块的字节范围，完整卷MD5未本地验证。数据来源https://zenodo.org/records/18988527，原始来源https://public.3.basecamp.com/p/JyT276MM7krjYrMoLqLQ6xST，署名Jorge Leonardo、Chevron U.S.A. Inc.和NZPM，编译数据许可CC BY 4.0。主要比较为128道Hybrid对Jitter、SA、300次随机搜索。六块一致性为描述性标准，不是总体显著性检验。对随机搜索平均正差但5/6块为正，未满足预定一致性标准。恢复规则80次FISTA、0.01阈值比例固定，实际归一化和阈值由观测计算。原9072点SPS无对应波形，未验证物理保幅、成像和实际采集成本。完整结果见results/closure_20261104/report/report.html。";
for(let i=0;i<deck.slides.items.length;i++){
 const png=await deck.export({slide:deck.slides.items[i],format:'png',scale:1});
 await fs.writeFile(`${build}/closure-${i+1}.png`,new Uint8Array(await png.arrayBuffer()));
}
const candidate=build+'/candidate_closure.pptx';await (await PresentationFile.exportPptx(deck)).save(candidate);
const skill=opts['skill-dir'];
process.env.RUNTIME_NODE_MODULES=opts['runtime-modules'];
const {finalizePresentation}=await import(pathToFileURL(path.join(skill,'container_tools/artifact_tool_utils.mjs')).href);
await fs.mkdir(path.dirname(opts.output),{recursive:true});
const result=await finalizePresentation({workspaceDir:root,candidatePath:candidate,
 finalPath:opts.output,
 pythonExecutable:opts.python,
 integrityValidatorPath:path.join(skill,'container_tools/inspect_presentation_package_integrity.py'),
 layoutValidatorPath:path.join(skill,'container_tools/inspect_presentation_layout_geometry.py'),
 layoutArgs:['--expected-slide-size-emu','12192000,6858000','--validate-bullet-geometry','--validate-heading-fit',
  ...[3,5,6,7].flatMap(n=>['--require-native-table-slide',String(n)])],
 explicitTotalSlideCount:7,requiredNativeTableOwnerSlides:[3,5,6,7],requiredNativeChartOwnerSlides:[],
 fontPolicy:{basis:'reference',families:['微软雅黑','Arial'],referencePath:source,referenceSha256:crypto.createHash('sha256').update(await fs.readFile(source)).digest('hex')},
 verifyArtifactToolImport:true,receiptPath:build+'/validation_closure_v2.json'});
console.log(JSON.stringify(result));
