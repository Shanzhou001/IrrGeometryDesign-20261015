#ifndef RECONSTRUCTIONOPTIONS_H // add by wushanzhou in 2026/10/24
#define RECONSTRUCTIONOPTIONS_H // add by wushanzhou in 2026/10/24
// One tolerance in original amplitude units, shared by the solver and reports.
inline double reconstructionConsistencyTolerance(double observedScale)
{
    return 1e-5 * (observedScale > 1.0 ? observedScale : 1.0);
}
// Explicit options for the separately versioned research interface. add by wushanzhou in 2026/10/24
struct ReconstructionOptions // add by wushanzhou in 2026/10/24
{ // add by wushanzhou in 2026/10/24
    double thresholdStart,thresholdEnd; // add by wushanzhou in 2026/10/24
    int scales,angles; // add by wushanzhou in 2026/10/24
    bool warmStart; // add by wushanzhou in 2026/10/24
    ReconstructionOptions():thresholdStart(0.0265),thresholdEnd(0.0005),scales(4),angles(16),warmStart(true){} // add by wushanzhou in 2026/10/24
}; // add by wushanzhou in 2026/10/24
#endif // add by wushanzhou in 2026/10/24
