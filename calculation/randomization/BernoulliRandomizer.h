//******************************************************************************
#ifndef __BernoulliRandomizer_H
#define __BernoulliRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include <boost/cast.hpp>

/** Unconditional Bernoulli randomizer for null hypothesis. */
class UnconditionalBernoulliRandomizer : public AbstractDenominatorDataRandomizer {
    protected:
        int             _total_C;
        int             _total_Controls;

        virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

    public:
        UnconditionalBernoulliRandomizer(int TotalC, int TotalControls, const Parameters& parameters, bool multiparents, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~UnconditionalBernoulliRandomizer() {}

        virtual UnconditionalBernoulliRandomizer * clone() const {return new UnconditionalBernoulliRandomizer(*this);}
};

/** abstract conditional Bernoulli randomizer for null hypothesis. */
class AbstractConditionalBernoulliRandomizer : public AbstractDenominatorDataRandomizer {
    protected:
        int             _total_C;
        int             _total_Controls;

        void MakeDataB(int tTotalCounts, double tTotalMeasure, std::vector<int>& RandCounts);

        virtual int _randomize(int cases, int controls, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

    public:
        AbstractConditionalBernoulliRandomizer(int TotalC, int TotalControls, const Parameters& parameters, bool multiparents, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~AbstractConditionalBernoulliRandomizer() {}
};

/** conditional Bernoulli randomizer for null hypothesis. */
class ConditionalBernoulliRandomizer : public AbstractConditionalBernoulliRandomizer {
    protected:
        virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

    public:
        ConditionalBernoulliRandomizer(int TotalC, int TotalControls, const Parameters& parameters, bool multiparents, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~ConditionalBernoulliRandomizer() {}

        virtual ConditionalBernoulliRandomizer * clone() const {return new ConditionalBernoulliRandomizer(*this);}
};

/** conditional Bernoulli randomizer for alternative hypothesis. */
class ConditionalBernoulliAlternativeHypothesisRandomizer : public AbstractConditionalBernoulliRandomizer {
    protected:
        typedef std::vector<double> Probabilities_t;

        unsigned int _n1; // number of nodes with excess risk
        double _p0; // default event probability
        double _p1; // event probability for excess risk nodes
        Probabilities_t _A;

        boost::shared_ptr<ScanRunner::NodeStructureContainer_t> _alternative_hypothesis_nodes;
        boost::shared_ptr<AbstractNodesProxy> _alternative_hypothesis_nodes_proxy;
        boost::shared_ptr<ScanRunner::NodeStructureContainer_t> _null_hypothesis_nodes;
        boost::shared_ptr<AbstractNodesProxy> _null_hypothesis_nodes_proxy;

        virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

    public:
        ConditionalBernoulliAlternativeHypothesisRandomizer(const ScanRunner::NodeStructureContainer_t& treeNodes,
                                                            const RelativeRiskAdjustmentHandler& adjustments,
                                                            int TotalC,
                                                            int TotalControls,
                                                            double p0,
                                                            double p1,
                                                            unsigned int n1,
                                                            const Parameters& parameters,
                                                            bool multiparents, 
                                                            long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~ConditionalBernoulliAlternativeHypothesisRandomizer();

        virtual ConditionalBernoulliAlternativeHypothesisRandomizer * clone() const {return new ConditionalBernoulliAlternativeHypothesisRandomizer(*this);}
};
//******************************************************************************
#endif
