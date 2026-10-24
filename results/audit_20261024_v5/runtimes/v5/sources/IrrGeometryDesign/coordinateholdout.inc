// Strict measured-data loading and a physical-coordinate observation operator. // add by wushanzhou in 2026/10/23
static bool loadHoldoutGather(const QString& path, std::vector<std::vector<double> >& data, // add by wushanzhou in 2026/10/23
                              std::vector<double>& coordinates, QString& error) // add by wushanzhou in 2026/10/23
{ // add by wushanzhou in 2026/10/23
    data.clear(); coordinates.clear(); QFile file(path); // add by wushanzhou in 2026/10/23
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) { error="cannot open holdout input"; return false; } // add by wushanzhou in 2026/10/23
    QTextStream in(&file); in.setCodec("UTF-8"); int lineNumber=0, columns=0; // add by wushanzhou in 2026/10/23
    while (!in.atEnd()) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        QString line=in.readLine().trimmed(); lineNumber++; // add by wushanzhou in 2026/10/23
        if (line.startsWith("# offsets_m=")) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            if (!coordinates.empty()) { error="duplicate coordinate metadata"; return false; } // add by wushanzhou in 2026/10/23
            QStringList fields=line.mid(12).split(',',QString::KeepEmptyParts); // add by wushanzhou in 2026/10/23
            for (int i=0;i<fields.size();i++) // add by wushanzhou in 2026/10/23
            { // add by wushanzhou in 2026/10/23
                bool ok=false; double value=fields[i].toDouble(&ok); // add by wushanzhou in 2026/10/23
                if (!ok || value!=value || fabs(value)>1e20 || (!coordinates.empty() && value<=coordinates.back())) // add by wushanzhou in 2026/10/23
                { error="coordinates must be finite and strictly increasing"; return false; } // add by wushanzhou in 2026/10/23
                coordinates.push_back(value); // add by wushanzhou in 2026/10/23
            } // add by wushanzhou in 2026/10/23
            continue; // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        if (line.isEmpty() || line.startsWith("#")) continue; // add by wushanzhou in 2026/10/23
        QStringList fields=line.contains(',')||line.contains(';') ? // add by wushanzhou in 2026/10/23
            line.split(QRegExp("[,;]"),QString::KeepEmptyParts) : line.split(QRegExp("\\s+"),QString::SkipEmptyParts); // add by wushanzhou in 2026/10/23
        if (!columns) columns=fields.size(); // add by wushanzhou in 2026/10/23
        if (fields.size()!=columns || columns<4) // add by wushanzhou in 2026/10/23
        { error=QString("ragged/short holdout row at line %1").arg(lineNumber); return false; } // add by wushanzhou in 2026/10/23
        std::vector<double> row; // add by wushanzhou in 2026/10/23
        for (int i=0;i<columns;i++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            bool ok=false; double value=fields[i].toDouble(&ok); // add by wushanzhou in 2026/10/23
            if (!ok || value!=value || fabs(value)>1e30) // add by wushanzhou in 2026/10/23
            { error=QString("invalid holdout value at line %1 column %2").arg(lineNumber).arg(i+1); return false; } // add by wushanzhou in 2026/10/23
            row.push_back(value); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        data.push_back(row); // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    if (data.size()<32 || (int)coordinates.size()!=columns) // add by wushanzhou in 2026/10/23
    { error="holdout requires >=32 time samples and # offsets_m= metadata matching every column"; return false; } // add by wushanzhou in 2026/10/23
    return true; // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23

static float interpolateKnown(const std::vector<float>& values,const std::vector<int>& known, // add by wushanzhou in 2026/10/23
                               const std::vector<double>& x,double position) // add by wushanzhou in 2026/10/23
{ // add by wushanzhou in 2026/10/23
    if (position<=x[known.front()]) return values[known.front()]; // add by wushanzhou in 2026/10/23
    if (position>=x[known.back()]) return values[known.back()]; // add by wushanzhou in 2026/10/23
    for (size_t k=1;k<known.size();k++) if (x[known[k]]>=position) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        int a=known[k-1],b=known[k]; double weight=(position-x[a])/(x[b]-x[a]); // add by wushanzhou in 2026/10/23
        return (float)((1-weight)*values[a]+weight*values[b]); // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    return 0; // add by wushanzhou in 2026/10/23
} // add by wushanzhou in 2026/10/23

static bool coordinateHoldoutReconstruct(const std::vector<std::vector<float> >& truth, // add by wushanzhou in 2026/10/23
    const std::vector<int>& mask,const std::vector<double>& x,int iterations, // add by wushanzhou in 2026/10/23
    std::vector<std::vector<float> >& linear,std::vector<std::vector<float> >& reconstructed, // add by wushanzhou in 2026/10/23
    double& consistencyError,double* outputScale=NULL,const ReconstructionOptions* options=NULL, // add by wushanzhou in 2026/10/24
    std::vector<float>* convergence=NULL) // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/23
    const int nt=(int)truth.size(),m=(int)x.size(); // add by wushanzhou in 2026/10/23
    linear.clear();reconstructed.clear();consistencyError=0; // add by wushanzhou in 2026/10/24
    if(nt<32 || m<2 || (int)mask.size()!=m || iterations<1)return false; // add by wushanzhou in 2026/10/24
    for(int j=0;j<m;j++)if(!reconstructionFinite(x[j]) || (j && x[j]<=x[j-1]) || (mask[j]!=0 && mask[j]!=1))return false; // add by wushanzhou in 2026/10/24
    for(int i=0;i<nt;i++)if((int)truth[i].size()!=m)return false; // add by wushanzhou in 2026/10/24
    double spacing=x.back()-x.front(); // add by wushanzhou in 2026/10/23
    for (int j=1;j<m;j++) spacing=std::min(spacing,x[j]-x[j-1]); // add by wushanzhou in 2026/10/23
    int nx=32; // add by wushanzhou in 2026/10/23
    // Distinct left nodes give a full-rank, tridiagonal observation Gram. // add by wushanzhou in 2026/10/23
    while (nx<4096 && (x.back()-x.front())/(nx-1)>spacing) nx*=2; // add by wushanzhou in 2026/10/23
    if ((x.back()-x.front())/(nx-1)>spacing) return false; // add by wushanzhou in 2026/10/23
    const double dx=(x.back()-x.front())/(nx-1); // add by wushanzhou in 2026/10/23
    std::vector<double> positions(m); // add by wushanzhou in 2026/10/23
    std::vector<int> known; // add by wushanzhou in 2026/10/23
    for(int j=0;j<m;j++) // add by wushanzhou in 2026/10/24
    { // add by wushanzhou in 2026/10/24
        positions[j]=j==m-1?(double)(nx-1):std::max(0.0,std::min((double)(nx-1),(x[j]-x.front())/dx)); // add by wushanzhou in 2026/10/24
        if(mask[j])known.push_back(j); // add by wushanzhou in 2026/10/24
    } // add by wushanzhou in 2026/10/24
    if (known.size()<2) return false; // add by wushanzhou in 2026/10/23
    double knownScale=0; // add by wushanzhou in 2026/10/23
    for(int i=0;i<nt;i++)for(size_t k=0;k<known.size();k++) // add by wushanzhou in 2026/10/23
        knownScale=std::max(knownScale,fabs((double)truth[i][known[k]])); // add by wushanzhou in 2026/10/23
    if(knownScale<=1e-20)knownScale=1.0; // add by wushanzhou in 2026/10/23
    if(outputScale)*outputScale=knownScale; // add by wushanzhou in 2026/10/23
    linear.assign(nt,std::vector<float>(m,0)); // add by wushanzhou in 2026/10/23
    std::vector<std::vector<float> > initial(nt,std::vector<float>(nx,0)); // add by wushanzhou in 2026/10/23
    std::vector<std::vector<float> > observed(nt,std::vector<float>(m,0)); // add by wushanzhou in 2026/10/23
    for (int i=0;i<nt;i++) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        for (int j=0;j<m;j++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            if(mask[j]) observed[i][j]=(float)(truth[i][j]/knownScale); // add by wushanzhou in 2026/10/23
            linear[i][j]=interpolateKnown(truth[i],known,x,x[j]); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
        for (int j=0;j<nx;j++) initial[i][j]=interpolateKnown(observed[i],known,x,x.front()+j*dx); // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    std::vector<std::vector<float> > unused(nt,std::vector<float>(nx,0)),grid,residual; // add by wushanzhou in 2026/10/23
    std::vector<int> gridMask(nx,0);std::vector<float> trace; // add by wushanzhou in 2026/10/23
    if(options && !options->warmStart)for(int i=0;i<nt;i++)std::fill(initial[i].begin(),initial[i].end(),0.0f); // add by wushanzhou in 2026/10/24
    curveletPocsReconstruct(initial,unused,gridMask,iterations,grid,residual,trace,&positions,&observed,&mask,options); // add by wushanzhou in 2026/10/24
    if(convergence)*convergence=trace; // add by wushanzhou in 2026/10/24
    if(grid.size()!=(size_t)nt || grid.empty() || grid[0].size()!=(size_t)nx)return false; // add by wushanzhou in 2026/10/23
    reconstructed.assign(nt,std::vector<float>(m,0));consistencyError=0; // add by wushanzhou in 2026/10/23
    for (int j=0;j<m;j++) // add by wushanzhou in 2026/10/23
    { // add by wushanzhou in 2026/10/23
        int a=(int)floor(positions[j]);if(a>=nx-1)a=nx-2; // add by wushanzhou in 2026/10/23
        const double weight=positions[j]-a; // add by wushanzhou in 2026/10/23
        for(int i=0;i<nt;i++) // add by wushanzhou in 2026/10/23
        { // add by wushanzhou in 2026/10/23
            reconstructed[i][j]=(float)(knownScale*((1-weight)*grid[i][a]+weight*grid[i][a+1])); // add by wushanzhou in 2026/10/23
            if(!reconstructionFinite(reconstructed[i][j]))return false; // add by wushanzhou in 2026/10/24
            if(mask[j]) consistencyError=std::max(consistencyError,fabs((double)reconstructed[i][j]-truth[i][j])); // add by wushanzhou in 2026/10/23
        } // add by wushanzhou in 2026/10/23
    } // add by wushanzhou in 2026/10/23
    // Projection works in knownScale-normalized units. Float output roundoff in
    // raw amplitude units must be compared with the same normalized tolerance.
    return consistencyError<1e-5*std::max(1.0,knownScale); // add by wushanzhou in 2026/10/24
} // add by wushanzhou in 2026/10/23
