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
        const RelativeRiskAdjustmentHandler&  _alternative_adjustments;
        RelativeRiskAdjustmentHandler::NodesExpectedContainer_t _nodes_IntN_C;
        boost::shared_ptr<AbstractNodesProxy> _nodes_proxy;

        virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);
        virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes, boost::mutex& mutex);
        virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes);

    public:
        AlternativeHypothesisRandomizater(const ScanRunner::NodeStructureContainer_t& treeNodes,
                                          boost::shared_ptr<AbstractRandomizer> randomizer,
                                          const RelativeRiskAdjustmentHandler& adjustments,
                                          const Parameters& parameters, 
                                          int totalC,
                                          bool multiparents,
                                          long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        AlternativeHypothesisRandomizater(const AlternativeHypothesisRandomizater& other) : AbstractRandomizer(other), _alternative_adjustments(other._alternative_adjustments) {
            _randomizer.reset(other._randomizer->clone());
            _nodes_IntN_C = other._nodes_IntN_C;
            _nodes_proxy.reset(other._nodes_proxy->clone());
        }
        virtual ~AlternativeHypothesisRandomizater() {}

        virtual AlternativeHypothesisRandomizater * clone() const {return new AlternativeHypothesisRandomizater(*this);}

        virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes);
};
#endif
//******************************************************************************
