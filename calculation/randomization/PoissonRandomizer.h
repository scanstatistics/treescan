//******************************************************************************
#ifndef __PoissonRandomizer_H
#define __PoissonRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include <boost/cast.hpp>

/** Abstraction for Poisson data randomizers */
class PoissonRandomizer : public AbstractDenominatorDataRandomizer {
protected:
    bool            _conditional;
    int             _TotalC;
    double          _TotalN;

    int             BinomialGenerator(int n, double p, bool classic=false);
    int             PoissonGenerator(double lambda);
    double          RandomUniform(bool classic=false);

public:
    PoissonRandomizer(bool conditional, int TotalC, double TotalN, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~PoissonRandomizer() {}

    virtual int     RandomizeData(unsigned int iSimulation,
                                  const ScanRunner::NodeStructureContainer_t& treeNodes,
                                  SimNodeContainer_t& treeSimNodes);

};
//******************************************************************************
#endif

