//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "TemporalRandomizer.h"

/* constructor */
TemporalRandomizer::TemporalRandomizer(int TotalC, double TotalN, const DataTimeRangeSet& timeRangeSets, long lInitialSeed)
                  : AbstractRandomizer(lInitialSeed), _TotalC(TotalC), _TotalN(TotalN), _timeRangeSets(timeRangeSets) {
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    _zero_translation_additive = std::abs(std::min(0, timeRangeSets.getMinMax().getStart()));
}

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
int TemporalRandomizer::RandomizeData(unsigned int iSimulation,
                                      const ScanRunner::NodeStructureContainer_t& treeNodes,
                                      SimNodeContainer_t& treeSimNodes) {
    SetSeed(iSimulation);
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    const DataTimeRange& range = _timeRangeSets.getDataTimeRangeSets().front(); // TODO: for now, only take the first
    DataTimeRange zeroRange(range.getStart() + _zero_translation_additive, range.getEnd() + _zero_translation_additive);

    int TotalSimC = 0;
    for(size_t i=0; i < treeNodes.size(); ++i) {
        NodeStructure::count_t branchC = treeNodes.at(i)->getBrC();
        if (branchC) {
            SimulationNode& simNode(treeSimNodes.at(i));
            for (NodeStructure::count_t c=0; c < branchC; ++c) {
                DataTimeRange::index_t idx = static_cast<DataTimeRange::index_t>(Equilikely(static_cast<long>(zeroRange.getStart()), static_cast<long>(zeroRange.getEnd()), _randomNumberGenerator));
                ++(simNode.refIntC_C().at(idx));
                ++TotalSimC;
            }
            simNode.setCumulative();
        }
    }
    TotalSimC = _TotalC;

    //------------------------ UPDATING THE TREE -----------------------------------
    for (size_t i=0; i < treeNodes.size(); i++) {
        if (treeNodes.at(i)->getAnforlust()==false) 
            addSimC_C(i, treeSimNodes.at(i).refIntC_C(), treeNodes, treeSimNodes);
        else 
            addSimC_CAnforlust(i, treeSimNodes.at(i).refIntC_C(), treeNodes, treeSimNodes);
    }
    return TotalSimC;
}
