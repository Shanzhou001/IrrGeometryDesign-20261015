#include "../plotting/figurepaths.h"
// add by wushanzhou in 2026/10/15
#include <QtGui/QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include "../gui/survey/tspsform.h"
#include "../experiments/geometryexperimentengine.h"
#include "../gui/survey/tspsdlg.h"
#include "../gui/survey/tcellgridform.h"
#include "../gui/dialogs/spssampledlg.h"
#include "../gui/dialogs/tchangeshowareadlg.h"
#include "../gui/dialogs/trealovtdlg.h"

#include "commandline.h"

int runCommandLine(int argc,char** argv,bool& handled)
{
    handled=true;
	for (int i = 1; i < argc; i++)
	{
        if(strcmp(argv[i],"--replay-reference")==0)
        {
            if(argc!=5 && argc!=6)return 2;
            bool valid=false;float ratio=QString::fromLocal8Bit(argv[4]).toFloat(&valid);
            if(!valid || ratio<=0 || ratio>=1)return 2;
            GeometryExperimentEngine::ReconstructionMetrics metrics;QString report;
            bool ok=GeometryExperimentEngine::runWavefieldReconstruction(
                QString::fromLocal8Bit(argv[3]),QString::fromLocal8Bit(argv[2]),ratio,metrics,report,
                argc==6?QString::fromLocal8Bit(argv[5]):QString(),true,false,true,false);
            fprintf(ok?stdout:stderr,"%s\n",report.toLocal8Bit().constData());return ok?0:1;
        }
        if(strcmp(argv[i],"--audit-queries")==0)
        {
            if(i+2>=argc || i+3!=argc)return 2;
            QString report;
            bool ok=GeometryExperimentEngine::runAuditQueries(QString::fromLocal8Bit(argv[i+1]),QString::fromLocal8Bit(argv[i+2]),report);
            fprintf(ok?stdout:stderr,"%s\n",report.toLocal8Bit().constData());return ok?0:1;
        }
        if(strcmp(argv[i],"--review-tests")==0)return GeometryExperimentEngine::runReviewUnitTests()?0:1; // add by wushanzhou in 2026/10/24
        if(strcmp(argv[i],"--review-search")==0) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            if(i+1>=argc)return 2; // add by wushanzhou in 2026/10/24
            QString output=QString::fromLocal8Bit(argv[i+1]),input,report; // add by wushanzhou in 2026/10/24
            int seeds=20,budget=200;unsigned long long seed=20261024ULL;bool tradeoff=false,valid=true; // add by wushanzhou in 2026/10/24
            for(int j=i+2;j<argc;j++) // add by wushanzhou in 2026/10/24
            { // add by wushanzhou in 2026/10/24
                if(strcmp(argv[j],"--tradeoff")==0)tradeoff=true; // add by wushanzhou in 2026/10/24
                else if(j+1>=argc){valid=false;break;} // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--input")==0)input=QString::fromLocal8Bit(argv[++j]); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--seeds")==0)seeds=QString::fromLocal8Bit(argv[++j]).toInt(&valid); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--budget")==0)budget=QString::fromLocal8Bit(argv[++j]).toInt(&valid); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--seed")==0)seed=QString::fromLocal8Bit(argv[++j]).toULongLong(&valid); // add by wushanzhou in 2026/10/24
                else valid=false; // add by wushanzhou in 2026/10/24
                if(!valid)break; // add by wushanzhou in 2026/10/24
            } // add by wushanzhou in 2026/10/24
            if(!valid)return 2; // add by wushanzhou in 2026/10/24
            bool ok=GeometryExperimentEngine::runReviewSearch(input,output,seeds,budget,seed,tradeoff,report); // add by wushanzhou in 2026/10/24
            fprintf(ok?stdout:stderr,"%s\n",report.toLocal8Bit().constData());return ok?0:1; // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
        if(strcmp(argv[i],"--review-reconstruct")==0) // add by wushanzhou in 2026/10/24
        { // add by wushanzhou in 2026/10/24
            if(i+1>=argc)return 2; // add by wushanzhou in 2026/10/24
            QString input=QString::fromLocal8Bit(argv[i+1]),output,mask,report; // add by wushanzhou in 2026/10/24
            int iterations=40,scales=4,angles=16;double start=.0265,end=.0005;bool warm=true,valid=true; // add by wushanzhou in 2026/10/24
            for(int j=i+2;j<argc;j++) // add by wushanzhou in 2026/10/24
            { // add by wushanzhou in 2026/10/24
                if(strcmp(argv[j],"--cold")==0)warm=false; // add by wushanzhou in 2026/10/24
                else if(j+1>=argc){valid=false;break;} // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--out")==0)output=QString::fromLocal8Bit(argv[++j]); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--mask")==0)mask=QString::fromLocal8Bit(argv[++j]); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--iter")==0)iterations=QString::fromLocal8Bit(argv[++j]).toInt(&valid); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--scales")==0)scales=QString::fromLocal8Bit(argv[++j]).toInt(&valid); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--angles")==0)angles=QString::fromLocal8Bit(argv[++j]).toInt(&valid); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--threshold-start")==0)start=QString::fromLocal8Bit(argv[++j]).toDouble(&valid); // add by wushanzhou in 2026/10/24
                else if(strcmp(argv[j],"--threshold-end")==0)end=QString::fromLocal8Bit(argv[++j]).toDouble(&valid); // add by wushanzhou in 2026/10/24
                else valid=false; // add by wushanzhou in 2026/10/24
                if(!valid)break; // add by wushanzhou in 2026/10/24
            } // add by wushanzhou in 2026/10/24
            if(!valid || output.isEmpty() || mask.isEmpty())return 2; // add by wushanzhou in 2026/10/24
            bool ok=GeometryExperimentEngine::runConfiguredReconstruction(input,mask,output,iterations,start,end,scales,angles,warm,report); // add by wushanzhou in 2026/10/24
            fprintf(ok?stdout:stderr,"%s\n",report.toLocal8Bit().constData());return ok?0:1; // add by wushanzhou in 2026/10/24
        } // add by wushanzhou in 2026/10/24
		if (strcmp(argv[i], "--test") == 0 || strcmp(argv[i], "--run-tests") == 0 || strcmp(argv[i], "-t") == 0)
		{
			bool ok = GeometryExperimentEngine::runUnitTests();
			return ok ? 0 : 1;
		}
		if (strcmp(argv[i], "--run-experiments") == 0 || strcmp(argv[i], "-e") == 0)
		{
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/synthetic"); // add by wushanzhou in 2026/10/23
			// 2026.10.20: 3 轮不足以支撑统计结论，默认提高到 10 轮独立种子
			int repeatRuns = 10;
			unsigned long long masterSeed = 42ULL;

			for (int j = 1; j < argc; j++)
			{
				if ((strcmp(argv[j], "--run-experiments") == 0 || strcmp(argv[j], "-e") == 0) && j + 1 < argc && argv[j + 1][0] != '-')
				{
					outDir = QString::fromLocal8Bit(argv[j + 1]);
				}
				else if ((strcmp(argv[j], "--repeat") == 0 || strcmp(argv[j], "-r") == 0) && j + 1 < argc)
				{
					repeatRuns = atoi(argv[j + 1]);
					if (repeatRuns < 1) repeatRuns = 1;
				}
				else if ((strcmp(argv[j], "--seed") == 0 || strcmp(argv[j], "-s") == 0) && j + 1 < argc)
				{
					masterSeed = (unsigned long long)_atoi64(argv[j + 1]);
				}
			}

			QString summary;
			printf("Starting Full Experiment Suite (Benchmark + Attributes + Wavefield CS Reconstruction)...\n");
			printf("Parameters: OutDir='%s', RepeatRuns=%d, MasterSeed=%llu\n",
				outDir.toLocal8Bit().constData(), repeatRuns, masterSeed);

			bool success = GeometryExperimentEngine::runFullExperimentSuite(NULL, outDir, summary, masterSeed, repeatRuns);
			printf("%s\n", summary.toLocal8Bit().constData());

			if (!success)
			{
				fprintf(stderr, "\n[ERROR]: Full experiment suite encountered failure in one or more required sub-experiments! Exiting with code 1.\n");
				return 1;
			}
			return 0;
		}
		// 2026.10.20: 真实野外资料重构验证专用入口
		// 用法: --run-real-reconstruction <input.csv> [--out <dir>] [--ratio <0..1>] [--mask <mask.txt>]
		if (strcmp(argv[i], "--run-real-reconstruction") == 0)
		{
			QString inCsv;
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/real_field"); // add by wushanzhou in 2026/10/23
			float ratio = 0.50f;
			QString maskPath = "";   // 留空则自动生成抖动掩码（固定种子，可复现）
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				inCsv = QString::fromLocal8Bit(argv[i + 1]);
			}
			for (int j = 1; j < argc; j++)
			{
				if (strcmp(argv[j], "--out") == 0 && j + 1 < argc)
				{
					outDir = QString::fromLocal8Bit(argv[j + 1]);
				}
				else if (strcmp(argv[j], "--ratio") == 0 && j + 1 < argc)
				{
					ratio = (float)atof(argv[j + 1]);
					if (ratio <= 0.0f || ratio >= 1.0f) ratio = 0.50f;
				}
				else if (strcmp(argv[j], "--mask") == 0 && j + 1 < argc)
				{
					maskPath = QString::fromLocal8Bit(argv[j + 1]);
				}
			}
			if (inCsv.isEmpty())
			{
				fprintf(stderr, "Usage: IrrGeometryDesign --run-real-reconstruction <input.csv> [--out <dir>] [--ratio <0..1>] [--mask <mask.txt>]\n");
				return 1;
			}
			printf("Real-field wavefield reconstruction\n");
			printf("  input : %s\n", inCsv.toLocal8Bit().constData());
			printf("  outDir: %s\n", outDir.toLocal8Bit().constData());
			printf("  ratio : %.2f\n", ratio);
			printf("  mask  : %s\n", maskPath.isEmpty() ? "(auto-generated jitter)" : maskPath.toLocal8Bit().constData());

			GeometryExperimentEngine::ReconstructionMetrics metrics;
			QString summary;
			bool ok = GeometryExperimentEngine::runWavefieldReconstruction(
				maskPath, outDir, ratio, metrics, summary, inCsv, true, false, true, true);
			printf("%s\n", summary.toLocal8Bit().constData());
			if (!ok)
			{
				fprintf(stderr, "\n[ERROR]: real-field reconstruction failed.\n");
				return 1;
			}
			printf("\nReal-field reconstruction succeeded.\n");
			return 0;
		}
		// 2026.10.20: 真实实测道留出验证（不依赖插值参考场）
		// 用法: --holdout-validate <gather.csv> [--traces m] [--keep k] [--iter n] [--seed s] [--out dir]
		if (strcmp(argv[i], "--holdout-validate") == 0)
		{
			QString inCsv;
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/real_field_holdout"); // add by wushanzhou in 2026/10/23
			int m = 59, k = 30, iters = 40;
			unsigned long long hs = 20261020ULL;
			QString extMask;   // 2026.10.20: 用外部掩码（抖动/SA）代替随机抽稀
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				inCsv = QString::fromLocal8Bit(argv[i + 1]);
			}
			for (int j = 1; j < argc; j++)
			{
				if (strcmp(argv[j], "--out") == 0 && j + 1 < argc) outDir = QString::fromLocal8Bit(argv[j + 1]);
				else if (strcmp(argv[j], "--traces") == 0 && j + 1 < argc) m = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--keep") == 0 && j + 1 < argc) k = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--iter") == 0 && j + 1 < argc) iters = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--seed") == 0 && j + 1 < argc) hs = (unsigned long long)_atoi64(argv[j + 1]);
				else if (strcmp(argv[j], "--mask") == 0 && j + 1 < argc) extMask = QString::fromLocal8Bit(argv[j + 1]);
			}
			if (inCsv.isEmpty())
			{
				fprintf(stderr, "Usage: IrrGeometryDesign --holdout-validate <gather.csv> [--traces m] [--keep k] [--iter n] [--seed s] [--out dir] [--mask <mask.txt>]\n");
				return 1;
			}
			printf("Hold-out validation on measured traces\n");
			printf("  input : %s\n", inCsv.toLocal8Bit().constData());
			printf("  traces=%d keep=%d iter=%d seed=%llu mask=%s\n", m, k, iters, hs,
				extMask.isEmpty() ? "(random)" : extMask.toLocal8Bit().constData());
			GeometryExperimentEngine::HoldoutMetrics hm;
			QString rep;
			bool hok = GeometryExperimentEngine::runHoldoutValidation(inCsv, m, k, iters, hs, outDir, hm, rep, extMask);
			printf("%s\n", rep.toLocal8Bit().constData());
			if (!hok)
			{
				fprintf(stderr, "\n[ERROR]: hold-out validation failed.\n");
				return 1;
			}
			return 0;
		}
		// 2026.10.20: 批量留出验证并归档（生成 holdout_runs.csv，供第三方复算）
		// 用法: --holdout-batch <gather.csv> [--traces m] [--iter n] [--out dir]
		if (strcmp(argv[i], "--holdout-batch") == 0)
		{
			QString inCsv;
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/real_field_holdout"); // add by wushanzhou in 2026/10/23
			int m = 59, iters = 40;
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				inCsv = QString::fromLocal8Bit(argv[i + 1]);
			}
			for (int j = 1; j < argc; j++)
			{
				if (strcmp(argv[j], "--out") == 0 && j + 1 < argc) outDir = QString::fromLocal8Bit(argv[j + 1]);
				else if (strcmp(argv[j], "--traces") == 0 && j + 1 < argc) m = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--iter") == 0 && j + 1 < argc) iters = atoi(argv[j + 1]);
			}
			if (inCsv.isEmpty())
			{
				fprintf(stderr, "Usage: IrrGeometryDesign --holdout-batch <gather.csv> [--traces m] [--iter n] [--out dir]\n");
				return 1;
			}
			QList<int> keeps;
			keeps << 20 << 30 << 40;
			QList<unsigned long long> seeds;
			seeds << 20261020ULL << 12345ULL;

			printf("Hold-out batch validation (6 configurations)\n");
			printf("  input : %s\n", inCsv.toLocal8Bit().constData());
			printf("  traces=%d iter=%d out=%s\n", m, iters, outDir.toLocal8Bit().constData());

			QString rep;
			bool bok = GeometryExperimentEngine::runHoldoutBatch(inCsv, m, keeps, seeds, iters, outDir, rep);
			printf("%s\n", rep.toLocal8Bit().constData());
			if (!bok)
			{
				fprintf(stderr, "\n[ERROR]: hold-out batch validation failed.\n");
				return 1;
			}
			return 0;
		}
		// 2026.10.20 新增：曲波变换参数鲁棒性实验（结果写入指定目录，可归档）
		// 用法: --miu-robustness [outDir, 默认 results/synthetic/01_sampling_benchmark]
		if (strcmp(argv[i], "--miu-robustness") == 0)
		{
			QString outDir = FigurePaths::artifactPath("results/synthetic/01_sampling_benchmark");
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				outDir = QString::fromLocal8Bit(argv[i + 1]);
			}
			printf("Curvelet parameter robustness experiment\n");
			printf("  outDir: %s\n", outDir.toLocal8Bit().constData());
			QString summary;
			bool ok = GeometryExperimentEngine::runMiuParamRobustnessExperiment(128, 0.50f, outDir, summary);
			printf("%s\n", summary.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		// 2026.10.20 新增：混合优化贡献分解消融（SA-only / CG-only / SA+CG / 等预算随机重启）
		// 用法: --ablation [outDir, 默认 results/ablation] [--seed S]
		if (strcmp(argv[i], "--ablation") == 0)
		{
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/ablation"); // add by wushanzhou in 2026/10/23
			unsigned long long abSeed = 42ULL;
			if (i + 1 < argc && argv[i + 1][0] != '-') outDir = QString::fromLocal8Bit(argv[i + 1]);
			for (int j = 1; j < argc; j++)
			{
				if ((strcmp(argv[j], "--seed") == 0 || strcmp(argv[j], "-s") == 0) && j + 1 < argc)
					abSeed = (unsigned long long)_atoi64(argv[j + 1]);
			}
			printf("Hybrid optimisation ablation (SA / CG / SA+CG / equal-budget restart)\n");
			printf("  outDir: %s, seed: %llu\n", outDir.toLocal8Bit().constData(), abSeed);
			std::vector<float> ablRates;
			ablRates.push_back(0.30f);
			ablRates.push_back(0.50f);
			ablRates.push_back(0.70f);
			QString summary;
			bool ok = GeometryExperimentEngine::runHybridAblationExperiment(128, ablRates, abSeed, outDir, summary);
			printf("%s\n", summary.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		// 2026.10.20 新增：2D 采样基准（N×N 接收网格上的二维相干度对比）
		// 用法: --sampling-2d [outDir, 默认 results/benchmark_2d] [--grid N] [--seed S]
		if (strcmp(argv[i], "--sampling-2d") == 0)
		{
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/benchmark_2d"); // add by wushanzhou in 2026/10/23
			int gridN = 32;
			int repeats2D = 1; // add by wushanzhou in 2026/10/23
			unsigned long long s2Seed = 42ULL;
			if (i + 1 < argc && argv[i + 1][0] != '-') outDir = QString::fromLocal8Bit(argv[i + 1]);
			for (int j = 1; j < argc; j++)
			{
				if (strcmp(argv[j], "--grid") == 0 && j + 1 < argc) gridN = atoi(argv[j + 1]);
				else if (strcmp(argv[j], "--seeds") == 0 && j + 1 < argc) repeats2D = atoi(argv[j + 1]); // add by wushanzhou in 2026/10/23
				else if ((strcmp(argv[j], "--seed") == 0 || strcmp(argv[j], "-s") == 0) && j + 1 < argc)
					s2Seed = (unsigned long long)_atoi64(argv[j + 1]);
			}
			printf("2D sampling benchmark (%dx%d grid, seed %llu)\n", gridN, gridN, s2Seed);
			std::vector<float> r2d;
			r2d.push_back(0.30f);
			r2d.push_back(0.50f);
			r2d.push_back(0.70f);
			QString summary;
			bool ok = GeometryExperimentEngine::runSampling2DBenchmark(gridN, r2d, s2Seed, outDir, summary, repeats2D); // add by wushanzhou in 2026/10/23
			printf("%s\n", summary.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		// 2026-10-21 新增：目标函数数学性质探针
		// 用法: --mu-probe <out.csv>
		if (strcmp(argv[i], "--mu-probe") == 0)
		{
			QString outCsv = FigurePaths::artifactPath("results/audit/mu_properties.csv");
			if (i + 1 < argc && argv[i + 1][0] != '-') outCsv = QString::fromLocal8Bit(argv[i + 1]);
			QDir().mkpath(QFileInfo(outCsv).absolutePath());
			QString rep;
			bool ok = GeometryExperimentEngine::runMiuProbe(outCsv, rep);
			printf("%s\n", rep.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		if (strcmp(argv[i], "--fft-probe") == 0)
		{
			QString outCsv = FigurePaths::artifactPath("results/audit/fft_structure.csv");
			if (i + 1 < argc && argv[i + 1][0] != '-') outCsv = QString::fromLocal8Bit(argv[i + 1]);
			QDir().mkpath(QFileInfo(outCsv).absolutePath());
			QString rep;
			bool ok = GeometryExperimentEngine::runFftProbe(outCsv, rep);
			printf("%s\n", rep.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		if (strcmp(argv[i], "--mu-discriminate") == 0)
		{
			QString outCsv = FigurePaths::artifactPath("results/audit/mu_discriminate.csv");
			if (i + 1 < argc && argv[i + 1][0] != '-') outCsv = QString::fromLocal8Bit(argv[i + 1]);
			QDir().mkpath(QFileInfo(outCsv).absolutePath());
			QString rep;
			bool ok = GeometryExperimentEngine::runMiuDiscriminateProbe(outCsv, rep);
			printf("%s\n", rep.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		if (strcmp(argv[i], "--mc-probe") == 0)
		{
			QString outCsv = FigurePaths::artifactPath("results/audit/mc_properties.csv");
			if (i + 1 < argc && argv[i + 1][0] != '-') outCsv = QString::fromLocal8Bit(argv[i + 1]);
			QDir().mkpath(QFileInfo(outCsv).absolutePath());
			QString rep;
			bool ok = GeometryExperimentEngine::runMcProbe(outCsv, rep);
			printf("%s\n", rep.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		// 2026-10-21 新增：空间约束优化 A–E 对比
		// 用法: --constrained [outDir] [--seeds N]
		if (strcmp(argv[i], "--constrained") == 0)
		{
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/constrained"); // add by wushanzhou in 2026/10/23
			int nSeeds = 5;
			if (i + 1 < argc && argv[i + 1][0] != '-') outDir = QString::fromLocal8Bit(argv[i + 1]);
			for (int j = 1; j < argc; j++)
				if (strcmp(argv[j], "--seeds") == 0 && j + 1 < argc) nSeeds = atoi(argv[j + 1]);
			if (nSeeds < 1) nSeeds = 1;
			std::vector<float> crates;
			crates.push_back(0.30f); crates.push_back(0.50f); crates.push_back(0.70f);
			std::vector<unsigned long long> cseeds;
			for (int k = 0; k < nSeeds; k++) cseeds.push_back(20261020ULL + (unsigned long long)(k * 179));
			printf("Constrained optimisation A-E comparison (%d seeds)\n", nSeeds);
			QString summary;
			bool ok = GeometryExperimentEngine::runConstrainedComparison(crates, cseeds, outDir, summary);
			printf("%s\n", summary.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		// 2026-10-21 新增：多种子等预算消融
		// 用法: --ablation-multiseed [outDir] [--seeds N] [--seed S]
		if (strcmp(argv[i], "--ablation-multiseed") == 0)
		{
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/ablation_multiseed"); // add by wushanzhou in 2026/10/23
			int nSeeds = 20;
			unsigned long long bs = 20261020ULL;
			if (i + 1 < argc && argv[i + 1][0] != '-') outDir = QString::fromLocal8Bit(argv[i + 1]);
			for (int j = 1; j < argc; j++)
			{
				if (strcmp(argv[j], "--seeds") == 0 && j + 1 < argc) nSeeds = atoi(argv[j + 1]);
				else if ((strcmp(argv[j], "--seed") == 0 || strcmp(argv[j], "-s") == 0) && j + 1 < argc)
					bs = (unsigned long long)_atoi64(argv[j + 1]);
			}
			if (nSeeds < 1) nSeeds = 1;
			printf("Multi-seed ablation: %d seeds, base=%llu\n", nSeeds, bs);
			std::vector<float> mr;
			mr.push_back(0.30f); mr.push_back(0.50f); mr.push_back(0.70f);
			QString summary;
			bool ok = GeometryExperimentEngine::runMultiSeedAblationExperiment(mr, nSeeds, bs, outDir, summary);
			printf("%s\n", summary.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
		// 2026-10-21 新增：掩码—重构对照
		// 用法: --mask-recon [outDir] [--masks N]
        if (strcmp(argv[i], "--holdout-study") == 0) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            QString input=FigurePaths::artifactPath("data/real_seismic/real_gather_59x256.csv"); // add by wushanzhou in 2026/10/23
            QString output=FigurePaths::artifactPath("results/enhanced_20261023/real_mask_study"); // add by wushanzhou in 2026/10/23
            int count=10,iterations=40;unsigned long long seed=20261023ULL; // add by wushanzhou in 2026/10/23
            if(i+1<argc && argv[i+1][0]!='-')input=QString::fromLocal8Bit(argv[i+1]); // add by wushanzhou in 2026/10/23
            for(int j=1;j<argc;j++) // add by wushanzhou in 2026/10/23
            { // add by wushanzhou in 2026/10/23
                if(strcmp(argv[j],"--out")==0 && j+1<argc)output=QString::fromLocal8Bit(argv[j+1]); // add by wushanzhou in 2026/10/23
                else if(strcmp(argv[j],"--seeds")==0 && j+1<argc)count=atoi(argv[j+1]); // add by wushanzhou in 2026/10/23
                else if(strcmp(argv[j],"--iter")==0 && j+1<argc)iterations=atoi(argv[j+1]); // add by wushanzhou in 2026/10/23
                else if(strcmp(argv[j],"--seed")==0 && j+1<argc)seed=(unsigned long long)_atoi64(argv[j+1]); // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
            QString report; // add by wushanzhou in 2026/10/23
            bool ok=GeometryExperimentEngine::runRealMaskStudy(input,count,iterations,seed,output,report); // add by wushanzhou in 2026/10/23
            printf("%s\n",report.toLocal8Bit().constData());return ok?0:1; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
		if (strcmp(argv[i], "--mask-recon") == 0)
		{
			QString outDir = FigurePaths::artifactPath("results/enhanced_20261023/mask_recon_builtin"); // add by wushanzhou in 2026/10/23
			int nMasks = 10;
            QString studyInput; // add by wushanzhou in 2026/10/23
			unsigned long long mrSeed = 20261020ULL;
			if (i + 1 < argc && argv[i + 1][0] != '-') outDir = QString::fromLocal8Bit(argv[i + 1]);
			for (int j = 1; j < argc; j++)
            { // add by wushanzhou in 2026/10/23
                if (strcmp(argv[j], "--masks") == 0 && j + 1 < argc) nMasks = atoi(argv[j + 1]); // add by wushanzhou in 2026/10/23
                else if (strcmp(argv[j], "--study-input") == 0 && j + 1 < argc) studyInput=QString::fromLocal8Bit(argv[j+1]); // add by wushanzhou in 2026/10/23
                else if (strcmp(argv[j], "--seed") == 0 && j + 1 < argc) mrSeed=(unsigned long long)_atoi64(argv[j+1]); // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
			printf("Mask-reconstruction study: %d masks per type\n", nMasks);
			QString summary;
			bool ok = GeometryExperimentEngine::runMaskReconStudy(outDir, nMasks, mrSeed, summary, studyInput); // add by wushanzhou in 2026/10/23
			printf("%s\n", summary.toLocal8Bit().constData());
			return ok ? 0 : 1;
		}
	}
    handled=false;return 0;
}
