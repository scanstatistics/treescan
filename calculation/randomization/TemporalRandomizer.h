//******************************************************************************
#ifndef __TemporalRandomizer_H
#define __TemporalRandomizer_H
//******************************************************************************
#include "Randomization.h"
#include "RandomDistribution.h"

/** Abstraction for Poisson data randomizers */
class TemporalRandomizer : public AbstractRandomizer {
protected:
    int                         _TotalC;
    double                      _TotalN;
    const DataTimeRangeSet    & _timeRangeSets;
    DataTimeRange::index_t      _zero_translation_additive;

public:
    TemporalRandomizer(int TotalC, double TotalN, const DataTimeRangeSet& timeRangeSets, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~TemporalRandomizer() {}

    virtual int     RandomizeData(unsigned int iSimulation,
                                  const ScanRunner::NodeStructureContainer_t& treeNodes,
                                  SimNodeContainer_t& treeSimNodes);
};
//******************************************************************************
#endif
