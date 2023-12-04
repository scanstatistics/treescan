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
    int             _total_C;
    double          _total_N;

    int             BinomialGenerator(int n, double p);
    int             PoissonGenerator(double lambda);
    double          RandomUniform();

    virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

public:
    PoissonRandomizer(bool conditional, const ScanRunner& scanner, long lInitialSeed = RandomNumberGenerator::glDefaultSeed);
    virtual ~PoissonRandomizer() {}

    virtual PoissonRandomizer * clone() const {return new PoissonRandomizer(*this);}
};
//******************************************************************************
#endif

