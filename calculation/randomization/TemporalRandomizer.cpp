//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "TemporalRandomizer.h"

/* constructor */
TemporalRandomizer::TemporalRandomizer(int TotalC, double TotalN, const DataTimeRangeSet& timeRangeSets, const Parameters& parameters, long lInitialSeed)
                  : AbstractRandomizer(parameters, lInitialSeed), _TotalC(TotalC), _TotalN(TotalN), _timeRangeSets(timeRangeSets) {
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    DataTimeRange min_max = timeRangeSets.getMinMax();
    _zero_translation_additive = (min_max.getStart() <= 0) ? std::abs(min_max.getStart()) : min_max.getStart() * -1;
}

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
int TemporalRandomizer::RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes) {
    // clear simulation data
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    int TotalSimC = 0;
    if (_parameters.isReadingSimulationData()) {
        boost::mutex::scoped_lock lock(mutex);
        TotalSimC = read(_parameters.getInputSimulationsFilename(), iSimulation, treeNodes, treeSimNodes);
    } else { // else standard randomization
        SetSeed(iSimulation);
        // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
        const DataTimeRange& range = _timeRangeSets.getDataTimeRangeSets().front(); // TODO: for now, only take the first
        DataTimeRange zeroRange(range.getStart() + _zero_translation_additive, range.getEnd() + _zero_translation_additive);
        for(size_t i=0; i < treeNodes.size(); ++i) {
            NodeStructure::count_t branchC = treeNodes.at(i)->getBrC();
            if (branchC) {
                SimulationNode& simNode(treeSimNodes.at(i));
                for (NodeStructure::count_t c=0; c < branchC; ++c) {
                    DataTimeRange::index_t idx = static_cast<DataTimeRange::index_t>(Equilikely(static_cast<long>(zeroRange.getStart()), static_cast<long>(zeroRange.getEnd()), _randomNumberGenerator));
                    ++(simNode.refIntC_C().at(idx));
                    ++TotalSimC;
                }
            }
        }
        TotalSimC = _TotalC;
    }
    // write simulation data to file if requested
    if (_parameters.isWritingSimulationData()) {
        boost::mutex::scoped_lock lock(mutex);
        write(_parameters.getOutputSimulationsFilename(), treeSimNodes);
    }
    // now set simulation data structures as cumulative
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::setCumulative));
    //------------------------ UPDATING THE TREE -----------------------------------
    for (size_t i=0; i < treeNodes.size(); i++) {
        if (treeNodes.at(i)->getAnforlust()==false) 
            addSimC_C(i, treeSimNodes.at(i).refIntC_C(), treeNodes, treeSimNodes);
        else 
            addSimC_CAnforlust(i, treeSimNodes.at(i).refIntC_C(), treeNodes, treeSimNodes);
    }
    return TotalSimC;
}

/** Reads simulation data from file. */
int TemporalRandomizer::read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) {
    std::ifstream stream;
    if (!stream.is_open()) stream.open(filename.c_str());
    if (!stream) throw resolvable_error("Error: Could not open file '%s' to read the simulated data.\n", filename.c_str());

    int count, total=0;

    //seek line offset for reading iSimulation'th simulation data
    unsigned int skip = (simulation - 1) * treeSimNodes.size();
    for (unsigned int i=0; i < skip; ++i)
        stream.ignore(std::numeric_limits<int>::max(), '\n');

    size_t checkNodes = treeSimNodes.size();
    for (size_t i=0; i < treeSimNodes.size(); ++i) {
        SimulationNode& simNode(treeSimNodes.at(i));
        int branch = 0;
        size_t checkC = simNode.getIntC_C().size();
        for (size_t s=0; s < simNode.getIntC_C().size(); ++s) {
            // check for end of file yet we should have more to read
            if (!(stream >> count) && i < checkNodes && s < checkC) {
                if (stream.eof())
                    throw resolvable_error("Error: Simulated data file does not contain enough data in simulation %d. Expecting %d nodes with %d datum but could only read %d nodes with %d datum.\n", simulation, checkNodes, checkC, i+1, s);
                throw resolvable_error("Error: Simulated data file appears to contain invalid data in simulation %d. Datum could not be read as integer for element %d of node %d.\n", simulation, s, i+1);
            }

            simNode.refIntC_C().at(s) = count;
            branch += count;
        }
        total += branch;
        // check that the total read count equals node branch count?
        if (branch != treeNodes.at(i)->getBrC())
            throw resolvable_error("Error: Simulation count (%d) for node %s in simulated data file does not equal branch count (%d).\n",
                                   branch, treeNodes.at(i)->getIdentifier().c_str(), treeNodes.at(i)->getBrC());
        treeSimNodes.at(i).refBrC() = 0;
    }
    stream.close();
    return total;
}

/** Writes simulation data to file. */
void TemporalRandomizer::write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) {
    std::ofstream stream;

    //open output file
    stream.open(filename.c_str(), std::ios::ate|std::ios::app);
    if (!stream) throw resolvable_error("Error: Could not open the simulated data output file '%s'.\n", filename.c_str());
    for (size_t i=0; i < treeSimNodes.size(); ++i) {
        const SimulationNode& simNode(treeSimNodes.at(i));
        for (size_t s=0; s < simNode.getIntC_C().size(); ++s) {
            stream << simNode.getIntC_C().at(s) << " ";
        }
        stream << std::endl;
    }
    stream.close();
}
