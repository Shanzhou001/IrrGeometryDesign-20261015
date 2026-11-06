# 实测波形与炮检映射检索结果

本轮已找到、下载并核验两套配套公开资料；原5151-5171测区的对应波形尚未找到可确认的公开匹配。没有将其他工区拼接或改名为原测区，也未把新增资料计入已封存实验。

## 原测区定位结果

本机原始SPS包含349个炮点、9072个检波点（18线×504点）、6282段炮检关系。源线号为5151、5156、5161、5166、5171，源点号1001—1139；接收点号6355—6858。

按[SEG SPS 2.1标准](https://seg.org/wp-content/uploads/2025/11/seg_sps_rev2.1.pdf)，X记录的野外记录号位于第8—15列，原文件为2—350，共349个FFID。首条关系的FFID是350，记录增量1、仪器码1；现有程序中的35011是组合编号，不能直接用它对接SEG-Y的FFID。

原文件只有H00/H26头记录，没有测区名、采集年份、业主、承包商、坐标基准/投影/分区或分量约定。坐标数值不能唯一确定测区。源记录含年积日和时刻，但缺少年份。通道增量为2，需按多分量/通道组约定核对；9072个接收位置不等同于已经确认9072条单分量波形。

已按文件名、精确坐标、源/接收线号及项目名进行网页与GitHub检索，未找到能核验属于同一测区的波形下载位置。这是本次检索未确认，不代表互联网绝对不存在。用户也无法确认原始工区或提供单位，因此此项仍需资料提供方补充身份信息。

标准化原始几何位于output/waveform_mapping_search：

- original_sps_fingerprint.json：源文件SHA-256、范围及缺失标识。
- original_ffid_source_map.csv：349个标准FFID与炮点对应。
- original_relation_segments_sps21.csv：6282段原始炮检关系，保留通道/仪器/索引字段。
- original_sources.csv、original_receivers.csv：完整炮点及检波点表。

这些文件只验证几何记录及字段解析，不冒充已完成实测波形的道头匹配。

## 已下载的相关公开数据

| 数据 | 下载与核验范围 | 本地位置 |
|---|---|---|
| Poland Line 001陆上叠前炮集 | 完整445100896字节SEG-Y及SPS/RPS/XPS；251炮点、782检波点；70782条有效道按FFID+道号唯一关联，502条辅助道排除 | data/public/poland_line001_field |
| Soda Lake 2010 3D/3C | 一炮7897520字节SEG-Y；完整发布的8321炮点、2978检波点、669212段关系文件；样例957条有效道对应319个检波位置，另有1条辅助道 | data/public/soda_lake_2010_field |

### Poland来源与使用

[SEG数据介绍](https://wiki.seg.org/wiki/Poland_2D_Vibroseis_Line_001)列出了完整SPS/RPS/XPS配套文件；[Geomage官方教程](https://doc.geomage.com/seismic-processing/3_-geometry-assigment-and-qc.html)说明其几何赋值方法。已从[Geofizyka Torun发布者下载地址](https://ftps.geofizyka.pl/main.html?download&weblink=973286ef2a219064126f69fd6b8f77c6&realfilename=2D_Land_vibro_data_2ms.tgz)获取完整压缩包及五个原文件，包内TXT声明public domain。

本地trace_source_receiver_mapping.csv逐道给出文件偏移、FFID、道号、源/检站号和坐标。verification.json记录70782条有效道全部唯一匹配，无缺失或重复。原SEG-Y坐标字段为零；导出的坐标来自配套几何文件，不能声称原道头已有坐标并与SPS独立一致。

二进制头和全部道头均为2ms、1501点，文件长度严格一致；文本头仍写4ms/750点，属于需保留的旧说明。原TXT实际为841字节，与SEG目录历史列示的866字节不同，未用历史尺寸冒充发布者校验和；当前发布者包的实际文件大小与本地SHA-256均记录在source.json。

### Soda Lake来源与使用

[DOE/GDR原始资料页](https://gdr.openei.org/submissions/1655)提供原始3D/3C炮集、现场日志及项目报告；[美国政府数据目录](https://catalog.data.gov/dataset/soda-lake-geothermal-raw-3d-and-3c-seismic-reflection-data-from-2010-survey)确认坐标嵌入SEG-Y、CC BY 4.0及发布单位。全体原始波形约171GB，本轮仅下载一个完整炮记录，没有宣称下载全工区。

geometry子目录中的SPS/RPS/XPS为从官方Field Logs ZIP按严格HTTP 206范围取得的完整文件，逐成员CRC和本地SHA-256通过；完整1GB日志ZIP没有下载或校验全卷。首次XPS传输TLS中断，随后采用有界网络重试完成；没有据此重试或替换科学实验。

样例的F7733R1_trace_source_receiver_mapping.csv关联957条有效道及319个位置，原道号减去1排除前导辅助道，与XPS三组分量块的全部957个位置关系相符。三个分量的物理方向尚未确认。

需保留的差异：原道头坐标乘0.1才与SPS声明的米制坐标一致，原道头的比例/单位字段不能直接照用；检波位置误差小于1e-8米，炮点位置最大坐标差为0.2米，FFID和最近测量炮点身份相符，但不是坐标完全相等。XPS有3条单点段的通道增量为0，因起止道号与点号相同而逐点识别，没有为多点段猜测间距。XPS接收索引0与测站文件索引1不同，导出表中分别保留。

公开现场Master日志另有ShotNo等标识，不将其直接等同于重排后的导出FFID。原始值和差异均保留在本地文件与verification.json中。

## 对原实验闭环的影响

新资料可用于后续陆上炮集导入、几何与波形联合验证，以及三维非规则接收点研究；本轮仅完成资料获取和映射核验，没有扩充原7740例实验或制造新的未见验证身份。后续使用须先确定分量、坐标解释、固定掩码和恢复规则，并明确资料已被查看。

原9072点测区的缺失输入仍未补齐。原测区的物理保幅、下游成像或实际现场成本仍不能据此宣布验证完成。已封存源码、程序和当前results/latest保持原身份，没有Git提交。
