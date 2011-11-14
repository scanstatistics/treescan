//******************************************************************************
#ifndef __BernoulliRandomizer_H
#define __BernoulliRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include <boost/cast.hpp>

/** Bernoulli randomizer for null hypothesis. */
class BernoulliRandomizer : public AbstractDenominatorDataRandomizer {
  protected:
    bool            _conditional;
    int             _TotalC;
    int             _TotalControls;
    double          _TotalN;
    double          _probability;

    void MakeDataB(int tTotalCounts, double tTotalMeasure, std::vector<int>& RandCounts);

  public:
    BernoulliRandomizer(double probability, bool conditional, int TotalC, int TotalControls, double TotalN, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~BernoulliRandomizer() {}

    virtual BernoulliRandomizer * Clone() const {return new BernoulliRandomizer(*this);}

    virtual int RandomizeData(unsigned int iSimulation,
                              const ScanRunner::NodeStructureContainer_t& treeNodes,
                              SimDataContainer_t& simData);
};
//******************************************************************************
#endif

