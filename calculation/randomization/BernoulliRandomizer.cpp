//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"

/* constructor */
BernoulliRandomizer::BernoulliRandomizer(double probability, bool conditional, int TotalC, int TotalControls, double TotalN, const Parameters& parameters, long lInitialSeed)
                    :AbstractDenominatorDataRandomizer(parameters, lInitialSeed), _probability(probability), _conditional(conditional), _TotalC(TotalC), _TotalN(TotalN), _TotalControls(TotalControls) {}


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
     x = _randomNumberGenerator.GetRandomDouble();
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
    SetSeed(iSimulation);
    // reset simData
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    std::vector<int> randCounts;
    int nCumCounts = _TotalC < _TotalControls ? _TotalC :_TotalControls;
    MakeDataB(nCumCounts, _TotalC + _TotalControls, randCounts);
    int nCumMeasure = _TotalC + _TotalControls - 1;
    int TotalSimC=0;

    if (_conditional) {
        TotalSimC = _TotalC;
        for (size_t i=0; i < treeNodes.size(); ++i) {
            nCumMeasure -= static_cast<int>(treeNodes.getIntN(i));
            while (nCumCounts > 0 && randCounts[nCumCounts-1] > nCumMeasure) {
                treeSimNodes.at(i).refIntC()++; //treeNodes.at(i)->_SimIntC += 1;
                nCumCounts--;
            }
            treeSimNodes.at(i).refBrC() = 0; //treeNodes.at(i)->_SimBrC = 0;  // Initilazing the branch cases with zero
        }
        //now reverse everything if Controls < Cases
        if (_TotalC >= _TotalControls) {
            for  (size_t i=0; i < treeNodes.size(); ++i)
                treeSimNodes.at(i).refIntC() = static_cast<int>(treeNodes.getIntN(i)) - treeSimNodes.at(i).refIntC();
        }
    } else {
        for (size_t i=0; i < treeNodes.size(); ++i) {
            int cases = gBinomialGenerator.GetBinomialDistributedVariable(static_cast<int>(treeNodes.getIntN(i)), static_cast<float>(treeNodes.getProbability(i)), _randomNumberGenerator);
            treeSimNodes.at(i).refIntC() = cases; //treeNodes.at(i)->_SimIntC = cases;
            TotalSimC += cases;
            treeSimNodes.at(i).refBrC() = 0; //treeNodes.at(i)->_SimBrC = 0;  // Initilazing the branch cases with zero
        } // for i
    }
    return TotalSimC;
}
