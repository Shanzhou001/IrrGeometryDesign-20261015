// Strict measured-data loading and a physical-coordinate observation operator.
static bool loadHoldoutGather(const QString& path, std::vector<std::vector<double> >& data,
                              std::vector<double>& coordinates, QString& error)
{
    data.clear(); coordinates.clear(); QFile file(path);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) { error="cannot open holdout input"; return false; }
    QTextStream in(&file); in.setCodec("UTF-8"); int lineNumber=0, columns=0;
    while (!in.atEnd())
    {
        QString line=in.readLine().trimmed(); lineNumber++;
        if (line.startsWith("# offsets_m="))
        {
            if (!coordinates.empty()) { error="duplicate coordinate metadata"; return false; }
            QStringList fields=line.mid(12).split(',',QString::KeepEmptyParts);
            for (int i=0;i<fields.size();i++)
            {
                bool ok=false; double value=fields[i].toDouble(&ok);
                if (!ok || value!=value || fabs(value)>1e20 || (!coordinates.empty() && value<=coordinates.back()))
                { error="coordinates must be finite and strictly increasing"; return false; }
                coordinates.push_back(value);
            }
            continue;
        }
        if (line.isEmpty() || line.startsWith("#")) continue;
        QStringList fields=line.contains(',')||line.contains(';') ?
            line.split(QRegExp("[,;]"),QString::KeepEmptyParts) : line.split(QRegExp("\\s+"),QString::SkipEmptyParts);
        if (!columns) columns=fields.size();
        if (fields.size()!=columns || columns<4)
        { error=QString("ragged/short holdout row at line %1").arg(lineNumber); return false; }
        std::vector<double> row;
        for (int i=0;i<columns;i++)
        {
            bool ok=false; double value=fields[i].toDouble(&ok);
            if (!ok || value!=value || fabs(value)>1e30)
            { error=QString("invalid holdout value at line %1 column %2").arg(lineNumber).arg(i+1); return false; }
            row.push_back(value);
        }
        data.push_back(row);
    }
    if (data.size()<32 || (int)coordinates.size()!=columns)
    { error="holdout requires >=32 time samples and # offsets_m= metadata matching every column"; return false; }
    return true;
}

static float interpolateKnown(const std::vector<float>& values,const std::vector<int>& known,
                               const std::vector<double>& x,double position)
{
    if (position<=x[known.front()]) return values[known.front()];
    if (position>=x[known.back()]) return values[known.back()];
    for (size_t k=1;k<known.size();k++) if (x[known[k]]>=position)
    {
        int a=known[k-1],b=known[k]; double weight=(position-x[a])/(x[b]-x[a]);
        return (float)((1-weight)*values[a]+weight*values[b]);
    }
    return 0;
}

static bool coordinateHoldoutReconstruct(const std::vector<std::vector<float> >& truth,
    const std::vector<int>& mask,const std::vector<double>& x,int iterations,
    std::vector<std::vector<float> >& linear,std::vector<std::vector<float> >& reconstructed,
    double& consistencyError,double* outputScale=NULL)
{
    const int nt=(int)truth.size(),m=(int)x.size();
    double spacing=x.back()-x.front();
    for (int j=1;j<m;j++) spacing=std::min(spacing,x[j]-x[j-1]);
    int nx=32;
    // Distinct left nodes give a full-rank, tridiagonal observation Gram.
    while (nx<4096 && (x.back()-x.front())/(nx-1)>spacing) nx*=2;
    if ((x.back()-x.front())/(nx-1)>spacing) return false;
    const double dx=(x.back()-x.front())/(nx-1);
    std::vector<double> positions(m);
    std::vector<int> known;
    for (int j=0;j<m;j++) { positions[j]=(x[j]-x.front())/dx; if(mask[j])known.push_back(j); }
    if (known.size()<2) return false;
    double knownScale=0;
    for(int i=0;i<nt;i++)for(size_t k=0;k<known.size();k++)
        knownScale=std::max(knownScale,fabs((double)truth[i][known[k]]));
    if(knownScale<=1e-20)knownScale=1.0;
    if(outputScale)*outputScale=knownScale;
    linear.assign(nt,std::vector<float>(m,0));
    std::vector<std::vector<float> > initial(nt,std::vector<float>(nx,0));
    std::vector<std::vector<float> > observed(nt,std::vector<float>(m,0));
    for (int i=0;i<nt;i++)
    {
        for (int j=0;j<m;j++)
        {
            if(mask[j]) observed[i][j]=(float)(truth[i][j]/knownScale);
            linear[i][j]=interpolateKnown(truth[i],known,x,x[j]);
        }
        for (int j=0;j<nx;j++) initial[i][j]=interpolateKnown(observed[i],known,x,x.front()+j*dx);
    }
    std::vector<std::vector<float> > unused(nt,std::vector<float>(nx,0)),grid,residual;
    std::vector<int> gridMask(nx,0);std::vector<float> trace;
    curveletPocsReconstruct(initial,unused,gridMask,iterations,grid,residual,trace,&positions,&observed,&mask);
    if(grid.size()!=(size_t)nt || grid.empty() || grid[0].size()!=(size_t)nx)return false;
    reconstructed.assign(nt,std::vector<float>(m,0));consistencyError=0;
    for (int j=0;j<m;j++)
    {
        int a=(int)floor(positions[j]);if(a>=nx-1)a=nx-2;
        const double weight=positions[j]-a;
        for(int i=0;i<nt;i++)
        {
            reconstructed[i][j]=(float)(knownScale*((1-weight)*grid[i][a]+weight*grid[i][a+1]));
            if(mask[j]) consistencyError=std::max(consistencyError,fabs((double)reconstructed[i][j]-truth[i][j]));
        }
    }
    return consistencyError<1e-5;
}
