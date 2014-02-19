//******************************************************************************
#ifndef __DenominatorDataRandomizer_H
#define __DenominatorDataRandomizer_H
//******************************************************************************
#include "Randomization.h"

/** Abstraction for denominator data randomizer. */
class AbstractDenominatorDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   gBinomialGenerator;

    virtual int randomize(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) = 0;

    virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes);
    virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes);

  public:
    AbstractDenominatorDataRandomizer(const Parameters& parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractRandomizer(parameters, lInitialSeed) {}
    virtual ~AbstractDenominatorDataRandomizer() {}

    virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes);
};
//******************************************************************************
#endif

