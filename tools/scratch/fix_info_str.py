# -*- coding: utf-8 -*-
import os

def main():
    target_path = os.path.join("IrrGeometryDesign", "spsshow", "tspsform.cpp")
    with open(target_path, "rb") as f:
        raw = f.read()

    if raw.startswith(b"\xef\xbb\xbf"):
        raw = raw[3:]
    content = raw.decode("utf-8")

    # Fix line around 1562 in on_actionRSample_triggered
    bad_block = """			QString info_str = QString::fromLocal8Bit(
		"观测系统相干度计算完成：\\n"
		"● 检波点互相干度 (μ_r): %1\\n"
		"● 激发点互相干度 (μ_s): %2\\n\\n"
		"【科学结论与图件支撑】：\\n"
		"互相干度 μ 越小，观测感知矩阵在 Curvelet 稀疏基下的去相干性越好，越有利于消除相干假频，支撑高信噪比波场恢复。").arg(miu_r).arg(miu_s);
	QMessageBox::information(this, QString::fromLocal8Bit("观测系统相干度计算结果"), info_str);"""

    correct_rsample_msg = """			QString info_str = QString::fromLocal8Bit(
				"检波线空间抽稀完成：\\n"
				"● 抽稀后互相干度 (μ): %1\\n\\n"
				"【后续优化建议】：\\n"
				"建议继续点击【退火优化】按钮，采用模拟退火算法优化扰动非规则排列，破坏周期性假频。").arg(miu_xj_before);
			QMessageBox::information(this, QString::fromLocal8Bit("检波线抽稀计算完成"), info_str);"""

    if bad_block in content:
        content = content.replace(bad_block, correct_rsample_msg, 1)

    # Now update line around 2205 in on_actionCalculate_the_Miu_for_Loaded_Geometry_triggered
    old_calc_miu = """	// 将计算得到的μ值输出 [Liu Wei 6/14/2023]
	QString info_str=tr("Incoherent parameter")+QString::fromLocal8Bit("(μ_r): ")+QString::number(miu_r)+"; "+QString::fromLocal8Bit("(μ_s): ")+QString::number(miu_s);
	QMessageBox::information(NULL,tr("Computation Information"),info_str);//输出信息可以根据需要进行调整"""

    new_calc_miu = """	// 将计算得到的μ值输出 [Liu Wei 6/14/2023]
	QString info_str = QString::fromLocal8Bit(
		"观测系统互相干度计算完成：\\n"
		"● 检波点互相干度 (μ_r): %1\\n"
		"● 激发点互相干度 (μ_s): %2\\n\\n"
		"【科学结论与图件支撑】：\\n"
		"互相干度 μ 越小，观测感知矩阵在 Curvelet 稀疏基下的去相干性越好，相干假频栅瓣越小，越有利于高保真度波场恢复。").arg(miu_r).arg(miu_s);
	QMessageBox::information(this, QString::fromLocal8Bit("观测系统相干度计算结果"), info_str);"""

    if old_calc_miu in content:
        content = content.replace(old_calc_miu, new_calc_miu, 1)

    out_bytes = b"\xef\xbb\xbf" + content.encode("utf-8")
    with open(target_path, "wb") as f:
        f.write(out_bytes)
    print("Fix applied successfully. Total bytes:", len(out_bytes))

if __name__ == "__main__":
    main()
