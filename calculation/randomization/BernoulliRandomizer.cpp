//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"

/* constructor */
BernoulliRandomizer::BernoulliRandomizer(bool conditional, int TotalC, int TotalControls, double TotalN, const Parameters& parameters, bool multiparents, long lInitialSeed)
                    :AbstractDenominatorDataRandomizer(parameters, multiparents, lInitialSeed), _conditional(conditional), _total_C(TotalC), _total_N(TotalN), _total_Controls(TotalControls) {}


/** Each of the totalMeasure number of individuals (sum of cases and controls),
    are randomized to either be a case or a control. The output is an array with
    the indices of the TotalCounts number of cases. For example, if there are
    20 cases and 80 controls, the output is an array the the indices between
    0 and 99 that correspond to the randomized cases. (MK Oct 27, 2003)         */
void BernoulliRandomizer::MakeDataB(int tTotalCounts, double tTotalMeasure, std::vector<int>& RandCounts) {
  int nCumCounts=0;
  double x, ratio;

  RandCounts.resize(tTotalCounts);
  for (int i=0; i < tTotalMeasure; ++i) {
     x = _random_number_generator.GetRandomDouble();
     ratio = (double) (tTotalCounts - nCumCounts) / (tTotalMeasure - i);
     if (x <= ratio) {
       RandCounts[nCumCounts] = i;
       nCumCounts++;
     }
  }
}

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. Caller is responsible for ensuring that
    passed array pointers are allocated and dimensions match that of passed tract and locations variables. */
int BernoulliRandomizer::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    //reset seed of random number generator
    setSeed(iSimulation);
    // reset simData
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    std::vector<int> randCounts;
    int nCumCounts = _total_C < _total_Controls ? _total_C :_total_Controls;
    MakeDataB(nCumCounts, _total_C + _total_Controls, randCounts);
    int nCumMeasure = _total_C + _total_Controls - 1;
    int TotalSimC=0;

    if (_conditional) {
        TotalSimC = _total_C;
        for (size_t i=0; i < treeNodes.size(); ++i) {
            nCumMeasure -= static_cast<int>(treeNodes.getIntN(i));
            while (nCumCounts > 0 && randCounts[nCumCounts-1] > nCumMeasure) {
                treeSimNodes[i].refIntC()++;
                nCumCounts--;
            }
            treeSimNodes[i].refBrC() = 0; // Initilazing the branch cases with zero
        }
        //now reverse everything if Controls < Cases
        if (_total_C >= _total_Controls) {
            for  (size_t i=0; i < treeNodes.size(); ++i)
                treeSimNodes[i].refIntC() = static_cast<int>(treeNodes.getIntN(i)) - treeSimNodes[i].refIntC();
        }
    } else {
        for (size_t i=0; i < treeNodes.size(); ++i) {
            int cases = _binomial_generator.GetBinomialDistributedVariable(static_cast<int>(treeNodes.getIntN(i)), static_cast<float>(treeNodes.getProbability(i)), _random_number_generator);
            treeSimNodes[i].refIntC() = cases;
            TotalSimC += cases;
            treeSimNodes[i].refBrC() = 0; // Initilazing the branch cases with zero
        } // for i
    }
    return TotalSimC;
}
