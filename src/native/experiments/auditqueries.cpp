#include "experiment_internal.h"

// v5 input has no waveforms: id;method;seed;B;minSA;window;gap;nn;Gindex;Gphysical;wallMs;x;mask;eligible.
#include <QSet>
bool GeometryExperimentEngine::runAuditQueries(const QString& input,const QString& output,QString& report)
{
    QFile source(input);QDir dir(output);
    if(QFileInfo(output).exists() || !source.open(QIODevice::ReadOnly|QIODevice::Text) || !QDir().mkpath(output))
    {report="cannot read queries or output already exists";return false;}
    QFile table(dir.filePath("queries.csv"));
    if(!table.open(QIODevice::WriteOnly|QIODevice::Text))return false;
    QTextStream os(&table),in(&source);os.setRealNumberPrecision(12);
    os << "id,method,n,m,seed,budget,ok,feasible,mu,objective,max_gap,max_gap_m,nn_cv,evals,attempts,wall_ms,native_clock_ms,status,mask\n";
    bool allOk=true;int count=0;QSet<QString> identifiers;
    while(!in.atEnd())
    {
        QString line=in.readLine().trimmed();if(line.isEmpty() || line.startsWith("#"))continue;
        QStringList f=line.split(';',QString::KeepEmptyParts);
        if(f.size()!=14 || !QRegExp("^[A-Za-z0-9_]+$").exactMatch(f[0]) || identifiers.contains(f[0]))
        {report="malformed query or duplicate identifier";return false;}
        identifiers.insert(f[0]);
        bool valid=true,ok=false;
        unsigned long long seed=f[2].toULongLong(&ok);valid=valid&&ok;
        int budget=f[3].toInt(&ok);valid=valid&&ok;
        AdaptiveSearchOptions options;
        options.minimumSa=f[4].toInt(&ok);valid=valid&&ok;
        options.stagnationWindow=f[5].toInt(&ok);valid=valid&&ok;
        ConstraintWeights w;
        w.lambdaGap=f[6].toDouble(&ok);valid=valid&&ok;
        w.lambdaNN=f[7].toDouble(&ok);valid=valid&&ok;
        w.hardMaxGap=f[8].toInt(&ok);valid=valid&&ok;
        w.hardMaxGapM=f[9].toDouble(&ok);valid=valid&&ok;
        options.wallLimitMs=f[10].toDouble(&ok);valid=valid&&ok;
        std::vector<double> x;
        if(!f[11].isEmpty())
        {
            QStringList positions=f[11].split(',',QString::KeepEmptyParts);
            for(int j=0;j<positions.size();j++){double v=positions[j].toDouble(&ok);valid=valid&&ok;x.push_back(v);}
        }
        const int n=f[12].size();std::vector<int> initial(n);int m=0;
        for(int j=0;j<n;j++){initial[j]=f[12][j]=='1'?1:0;m+=initial[j];if(f[12][j]!='1' && f[12][j]!='0')valid=false;}
        if(!f[13].isEmpty())
        {
            if(f[13].size()!=n)valid=false;
            for(int j=0;j<f[13].size();j++)
            {options.eligible.push_back(f[13][j]=='1'?1:0);if(f[13][j]!='1' && f[13][j]!='0')valid=false;}
        }
        if(n<2 || n>2048 || (!x.empty() && (int)x.size()!=n) || !valid)
        {report="invalid query parameters";return false;}
        ConstrainedResult result;bool feasible=false;
        if(f[1]=="dp")
        {
            feasible=SamplingPhysicalFeasibleMask(x,m,w.hardMaxGapM,result.mask);
            ok=true;result.status=feasible?"feasible":"infeasible";
        }
        else if(f[1]=="measure" || f[1]=="SAOnly" || f[1]=="Hybrid" || f[1]=="Local")
        {
            SamplingSearchMethod method=f[1]=="SAOnly"?SEARCH_SA:f[1]=="Local"?SEARCH_LOCAL:SEARCH_HYBRID;
            ok=RunSamplingSearch(n,m,initial,w,seed,f[1]=="measure"?1:budget,method,result,64,1,x.empty()?NULL:&x,
                options.eligible.empty()?NULL:&options.eligible);feasible=result.hardConstraintSatisfied;
        }
        else
        {
            if(f[1]=="SA_Budget")options.localSearch=false;
            else if(f[1]=="FixedSwitch")options.adaptiveSwitch=false;
            else if(f[1]=="NoRecycle")options.recycleBudget=false;
            else if(f[1]=="AdjacentOnly")options.mixedNeighborhood=false;
            else if(f[1]=="NoPerturb")options.perturbation=false;
            else if(f[1]!="Adaptive"){report="unknown query method";return false;}
            ok=RunAdaptiveSamplingSearch(n,m,initial,w,seed,budget,options,result,x.empty()?NULL:&x);
            feasible=result.hardConstraintSatisfied;
        }
        allOk=allOk&&ok;count++;
        os << f[0] << "," << f[1] << "," << n << "," << m << "," << QString::number(seed) << "," << budget
           << "," << (ok?1:0) << "," << (feasible?1:0) << "," << result.mu << "," << result.objective
           << "," << result.maxGap << "," << result.maxGapM << "," << result.nnCv << "," << result.evals
           << "," << QString::number(result.proposalAttempts) << "," << result.runtimeWallMs << "," << result.runtimeMs
           << "," << QString::fromStdString(result.status) << ",";
        for(size_t j=0;j<result.mask.size();j++)os << result.mask[j];os << "\n";
        if(!result.bestTrace.empty())
        {
            QFile trace(dir.filePath(f[0]+".trace.csv"));if(!trace.open(QIODevice::WriteOnly|QIODevice::Text))return false;
            QTextStream ts(&trace);ts.setRealNumberPrecision(12);
            ts << "evaluation,best,candidate_or_legacy_current,accepted_current,phase,temperature,wall_ms\n";
            for(size_t k=0;k<result.bestTrace.size();k++)
            {
                ts << k+1 << "," << result.bestTrace[k] << "," << result.currentTrace[k] << ",";
                if(k<result.acceptedTrace.size())ts << result.acceptedTrace[k];ts << ",";
                if(k<result.phaseTrace.size())ts << result.phaseTrace[k];ts << ",";
                if(k<result.temperatureTrace.size())ts << result.temperatureTrace[k];ts << ",";
                if(k<result.wallTraceMs.size())ts << result.wallTraceMs[k];ts << "\n";
            }
            ts.flush();if(trace.error()!=QFile::NoError)return false;
        }
    }
    os.flush();report=QString("audit_v5 processed %1 queries; success=%2").arg(count).arg(allOk);
    return count>0 && allOk && source.error()==QFile::NoError && table.error()==QFile::NoError;
}
