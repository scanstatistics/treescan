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
        int             _total_C;
        int             _total_Controls;
        double          _total_N;

        void MakeDataB(int tTotalCounts, double tTotalMeasure, std::vector<int>& RandCounts);

        virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

    public:
        BernoulliRandomizer(bool conditional, int TotalC, int TotalControls, double TotalN, const Parameters& parameters, bool multiparents, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~BernoulliRandomizer() {}

        virtual BernoulliRandomizer * clone() const {return new BernoulliRandomizer(*this);}
};
//******************************************************************************
#endif
