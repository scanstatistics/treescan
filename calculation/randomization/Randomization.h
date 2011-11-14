//******************************************************************************
#ifndef __Randomization_H
#define __Randomization_H
//******************************************************************************
#include "RandomNumberGenerator.h"
#include "RandomDistribution.h"
#include "ScanRunner.h"

typedef std::pair<int,int> SimData_t;
typedef std::vector<SimData_t> SimDataContainer_t;

/* abstract data randomizer base class */
class AbstractRandomizer {
protected:
    RandomNumberGenerator _randomNumberGenerator;  /** generates random numbers */

    void addSimC(size_t id, int c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimDataContainer_t& simData);
    void addSimCAnforlust(size_t id, int c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimDataContainer_t& simData);
    void SetSeed(unsigned int iSimulationIndex);

public:
    AbstractRandomizer::AbstractRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : _randomNumberGenerator(lInitialSeed) {}
    virtual ~AbstractRandomizer() {}

    virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimDataContainer_t& simData) = 0;
    static AbstractRandomizer * getNewRandomizer(const Parameters& parameters, int TotalC, int TotalControls, double TotalN);
};
//******************************************************************************
#endif
