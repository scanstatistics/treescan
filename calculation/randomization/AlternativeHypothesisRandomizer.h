//******************************************************************************
#ifndef __AlternativeHypothesisRandomization_H
#define __AlternativeHypothesisRandomization_H
//******************************************************************************
#include "Randomization.h"
#include "RelativeRiskAdjustment.h"

/** Abstraction for denominator data randomizer. */
class AlternativeHypothesisRandomizater : public AbstractRandomizer {
    protected:
        boost::shared_ptr<AbstractRandomizer> _randomizer;
        boost::shared_ptr<RelativeRiskAdjustmentHandler> _riskAdjustments;
        RelativeRiskAdjustmentHandler::NodesExpectedContainer_t _nodes_IntN_C;
        boost::shared_ptr<AbstractNodesProxy> _nodes_proxy;

        virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);
        virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes);
        virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes);

    public:
        AlternativeHypothesisRandomizater(const ScanRunner::NodeStructureContainer_t& treeNodes,
                                          boost::shared_ptr<AbstractRandomizer> randomizer,
                                          boost::shared_ptr<RelativeRiskAdjustmentHandler> riskAdjustments,
                                          const Parameters& parameters, 
                                          long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~AlternativeHypothesisRandomizater() {}

        virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes);
};
#endif
//******************************************************************************
