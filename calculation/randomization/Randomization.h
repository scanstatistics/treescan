//******************************************************************************
#ifndef __Randomization_H
#define __Randomization_H
//******************************************************************************
#include "RandomNumberGenerator.h"
#include "RandomDistribution.h"
#include "ScanRunner.h"

typedef std::pair<int,int>     SimData_t;
typedef std::vector<SimData_t> SimDataContainer_t;

/* abstract data randomizer base class */
class AbstractRandomizer {
protected:
    RandomNumberGenerator   _randomNumberGenerator;  /** generates random numbers */

    void            SetSeed(unsigned int iSimulationIndex);

public:
    AbstractRandomizer::AbstractRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : _randomNumberGenerator(lInitialSeed) {}
    virtual ~AbstractRandomizer() {}

    virtual int     RandomizeData(unsigned int iSimulation,
                                  const ScanRunner::NodeStructureContainer_t& treeNodes,
                                  SimDataContainer_t& simData) = 0;
};

/** Abstraction for denominator data randomizer. */
class AbstractDenominatorDataRandomizer : public AbstractRandomizer {
protected:
    BinomialGenerator   gBinomialGenerator;

public:
    AbstractDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractRandomizer(lInitialSeed) {}
    virtual ~AbstractDenominatorDataRandomizer() {}
};

/** Abstraction for Poisson data randomizers */
class PoissonRandomizer : public AbstractDenominatorDataRandomizer {
protected:
    bool            _conditional;
    int             _TotalC;
    double          _TotalN;

    int             BinomialGenerator(int n, double p, bool classic=true);
    int             PoissonGenerator(double lambda);
    double          RandomUniform(bool classic=true);

    void            addSimC(size_t id, int c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimDataContainer_t& simData);
    void            addSimCAnforlust(size_t id, int c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimDataContainer_t& simData);

public:
    PoissonRandomizer(bool conditional, int TotalC, double TotalN, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~PoissonRandomizer() {}

    virtual int     RandomizeData(unsigned int iSimulation,
                                  const ScanRunner::NodeStructureContainer_t& treeNodes,
                                  SimDataContainer_t& simData);

};
//******************************************************************************
#endif