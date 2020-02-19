//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"
#include <boost/random.hpp>

using boost::mt19937;
using boost::uniform_01;

//////////////////// UnconditionalBernoulliRandomizer //////////////////////////

/* constructor */
UnconditionalBernoulliRandomizer::UnconditionalBernoulliRandomizer(const ScanRunner& scanner, long lInitialSeed)
                    :AbstractDenominatorDataRandomizer(scanner.getParameters(), scanner.getMultiParentNodesExist(), lInitialSeed), 
                    _scanner(scanner), _total_C(scanner.getTotalC()), _total_Controls(scanner.getTotalControls()) {

    // TODO -- evetually replace this?
    if (scanner.getParameters().isSequentialScanBernoulli()) {
        if (!scanner.getSequentialStatistic().isFirstLook())
            _read_filename = _scanner.getSequentialStatistic().getSimulationDataFilename();
        _write_filename = _scanner.getSequentialStatistic().getWriteSimulationDataFilename();
    }

}

/** Distributes cases into simulation case array. Caller is responsible for ensuring that
    passed array pointers are allocated and dimensions match that of passed tract and locations variables. */
int UnconditionalBernoulliRandomizer::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    //reset seed of random number generator
    setSeed(iSimulation);
    // reset simData
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    int TotalSimC=0;
    for (size_t i=0; i < treeNodes.size(); ++i) {
        int cases = _binomial_generator.GetBinomialDistributedVariable(static_cast<int>(treeNodes.getIntN(i)), static_cast<float>(treeNodes.getProbability(i)), _random_number_generator);
        treeSimNodes[i].refIntC() = cases;
        TotalSimC += cases;
        treeSimNodes[i].refBrC() = 0; // Initilazing the branch cases with zero
    } // for i
    return TotalSimC;
}

//////////////////// AbstractConditionalBernoulliRandomizer ////////////////////////////

/* constructor */
AbstractConditionalBernoulliRandomizer::AbstractConditionalBernoulliRandomizer(int TotalC, int TotalControls, const Parameters& parameters, bool multiparents, long lInitialSeed)
                    :AbstractDenominatorDataRandomizer(parameters, multiparents, lInitialSeed), _total_C(TotalC), _total_Controls(TotalControls) {}

/** Each of the totalMeasure number of individuals (sum of cases and controls),
    are randomized to either be a case or a control. The output is an array with
    the indices of the TotalCounts number of cases. For example, if there are
    20 cases and 80 controls, the output is an array the the indices between
    0 and 99 that correspond to the randomized cases. (MK Oct 27, 2003)         */
void AbstractConditionalBernoulliRandomizer::MakeDataB(int tTotalCounts, double tTotalMeasure, std::vector<int>& RandCounts) {
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
int AbstractConditionalBernoulliRandomizer::_randomize(int cases, int controls, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    std::vector<int> randCounts;
    int nCumCounts = cases < controls ? cases : controls;
    MakeDataB(nCumCounts, cases + controls, randCounts);
    int nCumMeasure = cases + controls - 1;
    int TotalSimC=0;

    TotalSimC = cases;
    for (size_t i=0; i < treeNodes.size(); ++i) {
        nCumMeasure -= static_cast<int>(treeNodes.getIntN(i));
        while (nCumCounts > 0 && randCounts[nCumCounts-1] > nCumMeasure) {
            treeSimNodes[treeNodes.getID(i)/*i*/].refIntC()++;
            nCumCounts--;
        }
        treeSimNodes[treeNodes.getID(i)/*i*/].refBrC() = 0; // Initilazing the branch cases with zero
    }
    //now reverse everything if Controls < Cases
    if (cases >= controls) {
        for  (size_t i=0; i < treeNodes.size(); ++i)
            treeSimNodes[treeNodes.getID(i)/*i*/].refIntC() = static_cast<int>(treeNodes.getIntN(i)) - treeSimNodes[treeNodes.getID(i)/*i*/].refIntC();
    }
    return TotalSimC;
}

//////////////////// BernoulliTimeRandomizer ////////////////////////////

/* constructor */
BernoulliTimeRandomizer::BernoulliTimeRandomizer(int TotalC, int TotalControls, const Parameters& parameters, bool multiparents, long lInitialSeed)
	:AbstractConditionalBernoulliRandomizer(TotalC, TotalControls, parameters, multiparents, lInitialSeed) {}

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
controls then each randomly assigned to be a case or a control. Caller is responsible for ensuring that
passed array pointers are allocated and dimensions match that of passed tract and locations variables. */
int BernoulliTimeRandomizer::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
	//reset seed of random number generator
	setSeed(iSimulation);
	// reset simData
	std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

	int TotalSimC = 0;
	for (size_t i = 0; i < treeNodes.size(); ++i) {
		SimulationNode& simNode = treeSimNodes[treeNodes.getID(i)];
		std::vector<int> randCounts;
		int node_cases = treeNodes.getIntC(i), node_controls = static_cast<int>(treeNodes.getIntN(i));
		TotalSimC += node_cases;
		int nCumCounts = node_cases < node_controls ? node_cases : node_controls;
		MakeDataB(nCumCounts, node_cases + node_controls, randCounts);
		int nCumMeasure = node_cases + node_controls - 1;
		int intervals = static_cast<int>(simNode.refIntC_C().size());
		for (int idx = intervals-1; idx >= 0; --idx) {
			if (idx == intervals - 1) {
				nCumMeasure -= treeNodes.getIntC_C(i)[idx] + static_cast<int>(treeNodes.getIntN_C(i)[idx]);
			} else {
				nCumMeasure -= (treeNodes.getIntC_C(i)[idx] - treeNodes.getIntC_C(i)[idx + 1]) + static_cast<int>(treeNodes.getIntN_C(i)[idx] - treeNodes.getIntN_C(i)[idx + 1]);
			}
			while (nCumCounts > 0 && randCounts[nCumCounts - 1] > nCumMeasure) {
				simNode.refIntC_C()[idx]++;
				--nCumCounts;
			}
		}
		simNode.setCumulative();
		//now reverse everything if Controls < Cases
		if (node_cases >= node_controls)
			for (int idx = 0; idx < intervals; ++idx)
				simNode.refIntC_C()[idx] = (treeNodes.getIntC_C(i)[idx] + static_cast<int>(treeNodes.getIntN_C(i)[idx])) - simNode.refIntC_C()[idx];
	}
	return TotalSimC;
}

//////////////////// ConditionalBernoulliRandomizer ////////////////////////////

/* constructor */
ConditionalBernoulliRandomizer::ConditionalBernoulliRandomizer(int TotalC, int TotalControls, const Parameters& parameters, bool multiparents, long lInitialSeed)
                    :AbstractConditionalBernoulliRandomizer(TotalC, TotalControls, parameters, multiparents, lInitialSeed) {}

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. Caller is responsible for ensuring that
    passed array pointers are allocated and dimensions match that of passed tract and locations variables. */
int ConditionalBernoulliRandomizer::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    //reset seed of random number generator
    setSeed(iSimulation);
    // reset simData
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));
    return _randomize(_total_C, _total_Controls, treeNodes, treeSimNodes);
}

//////////////////// ConditionalBernoulliAlternativeHypothesisRandomizer ////////////////////////////

/* constructor */
ConditionalBernoulliAlternativeHypothesisRandomizer::ConditionalBernoulliAlternativeHypothesisRandomizer(const ScanRunner::NodeStructureContainer_t& treeNodes,
                                                                                                         const RelativeRiskAdjustmentHandler& adjustments,
                                                                                                         int TotalC, int TotalControls,
                                                                                                         double p0, double p1, unsigned int n1,
                                                                                                         const Parameters& parameters, bool multiparents, long lInitialSeed)
                    :AbstractConditionalBernoulliRandomizer(TotalC, TotalControls, parameters, multiparents, lInitialSeed), _p0(p0), _p1(p1), _n1(n1) {

    BinomialGenerator bg;
    _A.resize(std::min(static_cast<unsigned int>(TotalC), n1) + 1/* correct, so that we go 0 to total? */, 0.0);
    unsigned int N = static_cast<unsigned int>(TotalC + TotalControls);
    double sum=0.0;
    size_t k = static_cast<size_t>(std::max((int)0, TotalC - static_cast<int>(N - n1)));
    for (; k < _A.size(); ++k) {
        double d1 = bg.getBinomialDistributionProbability(k, n1, p1);
        double d2 = 0.0;
        /* We need to guard around the situation where the number of trials is less than the number of expected successes. */
        if (N - n1 >= static_cast<unsigned int>(TotalC) - k)
            d2 = bg.getBinomialDistributionProbability(static_cast<unsigned int>(TotalC) - k, N - n1, p0);
        _A[k] = d1 * d2;
        sum += _A[k];
    }
    for (size_t k=0; k < _A.size(); ++k) {
        _A[k] = (k == 0 ? 0.0 : _A[k - 1]) + _A[k]/sum;
    }

    // We need to create 2 groups of  tree nodes: those nodes in the alternative hypothesis and those still under the null hypothesis.
    boost::dynamic_bitset<> node_set(treeNodes.size());
    _alternative_hypothesis_nodes.reset(new ScanRunner::NodeStructureContainer_t());
    for (AdjustmentsContainer_t::const_iterator itr = adjustments.get().begin(); itr != adjustments.get().end(); ++itr) {
        node_set.set(itr->first);
        _alternative_hypothesis_nodes->push_back(treeNodes[itr->first]);
    }
    _alternative_hypothesis_nodes_proxy.reset(new NodesProxy(*_alternative_hypothesis_nodes));

    _null_hypothesis_nodes.reset(new ScanRunner::NodeStructureContainer_t());
    node_set.flip();
    boost::dynamic_bitset<>::size_type p = node_set.find_first();
    for (; p != boost::dynamic_bitset<>::npos; p = node_set.find_next(p))
        _null_hypothesis_nodes->push_back(treeNodes[p]);
    _null_hypothesis_nodes_proxy.reset(new NodesProxy(*_null_hypothesis_nodes));
}

ConditionalBernoulliAlternativeHypothesisRandomizer::~ConditionalBernoulliAlternativeHypothesisRandomizer() {
    _alternative_hypothesis_nodes->clear(); // Clear all elements -- otherwise we're deleting pointers held somewhere else.
    _null_hypothesis_nodes->clear(); // Clear all elements -- otherwise we're deleting pointers held somewhere else.
}

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. Caller is responsible for ensuring that
    passed array pointers are allocated and dimensions match that of passed tract and locations variables. */
int ConditionalBernoulliAlternativeHypothesisRandomizer::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    //reset seed of random number generator
    setSeed(iSimulation);
    // reset simData
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    mt19937 generator(iSimulation);
    Probabilities_t::iterator itr=std::lower_bound(_A.begin(), _A.end(), uniform_01<mt19937>(generator)());
    // random number of cases among n1 indivduals in the nodes w/ excess risk
    unsigned int X1 = (itr == _A.end() ? static_cast<unsigned int>(0) : static_cast<unsigned int>(std::distance(_A.begin(), itr)));
    return _randomize(X1, _n1 - X1, *_alternative_hypothesis_nodes_proxy, treeSimNodes) +
           _randomize(_total_C - X1, (_total_C + _total_Controls) - _n1 - (_total_C - X1), *_null_hypothesis_nodes_proxy, treeSimNodes);
}
