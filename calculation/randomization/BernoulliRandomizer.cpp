//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"


/* constructor */
BernoulliRandomizer::BernoulliRandomizer(double probability, bool conditional, int TotalC, int TotalControls, double TotalN, long lInitialSeed)
                    :AbstractDenominatorDataRandomizer(lInitialSeed), _probability(probability), _conditional(conditional), _TotalC(TotalC), _TotalN(TotalN), _TotalControls(TotalControls) {}


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
int BernoulliRandomizer::RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimDataContainer_t& simData) {
  //reset seed of random number generator
  SetSeed(iSimulation);

  // reset simData
  std::fill(simData.begin(), simData.end(), SimData_t(0,0));

  std::vector<int> randCounts;
  int nCumCounts = _TotalC < _TotalControls ? _TotalC :_TotalControls;
  MakeDataB(nCumCounts, _TotalC + _TotalControls, randCounts);
  int nCumMeasure = _TotalC + _TotalControls - 1;
  int TotalSimC=0;

  if (_conditional) {
      TotalSimC = _TotalC;
      for (size_t i=0; i < treeNodes.size(); ++i) {
          nCumMeasure -= static_cast<int>(treeNodes.at(i)->getIntN());
          while (nCumCounts > 0 && randCounts[nCumCounts-1] > nCumMeasure) {
              simData.at(i).first++; //treeNodes.at(i)->_SimIntC += 1;
              nCumCounts--;
          }
          simData.at(i).second = 0; //treeNodes.at(i)->_SimBrC = 0;  // Initilazing the branch cases with zero
      }
      //now reverse everything if Controls < Cases
      if (_TotalC >= _TotalControls) {
        for  (size_t i=0; i < treeNodes.size(); ++i)
           simData.at(i).first = static_cast<int>(treeNodes.at(i)->getIntN()) - simData.at(i).first;
      }
  } else {
    for (size_t i=0; i < treeNodes.size(); ++i) {
        int cases = gBinomialGenerator.GetBinomialDistributedVariable(static_cast<int>(treeNodes.at(i)->getIntN()), static_cast<float>(_probability), _randomNumberGenerator);
        simData.at(i).first = cases; //treeNodes.at(i)->_SimIntC = cases;
        TotalSimC += cases;
        simData.at(i).second = 0; //treeNodes.at(i)->_SimBrC = 0;  // Initilazing the branch cases with zero
    } // for i
  }    

  //------------------------ UPDATING THE TREE -----------------------------------
  for (size_t i=0; i < treeNodes.size(); i++) {
      if (treeNodes.at(i)->getAnforlust()==false) addSimC(i, simData.at(i).first/*_Nodes.at(i)->_SimIntC*/, treeNodes, simData);
      else addSimCAnforlust(i, simData.at(i).first/*_Nodes.at(i)->_SimIntC*/, treeNodes, simData);
  }
  return TotalSimC;
}