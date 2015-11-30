//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "TemporalRandomizer.h"

/* constructor */
TemporalRandomizer::TemporalRandomizer(const ScanRunner& scanner, long lInitialSeed)
    : AbstractRandomizer(scanner.getParameters(), scanner.getMultiParentNodesExist(), lInitialSeed),
     _total_C(scanner.getTotalC()), _total_N(scanner.getTotalN()), _time_range_sets(scanner.getParameters().getDataTimeRangeSet()), _day_of_week_indexes(scanner.getDayOfWeekIndexes()) {
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    DataTimeRange min_max = _time_range_sets.getMinMax();
    _zero_translation_additive = (min_max.getStart() <= 0) ? std::abs(min_max.getStart()) : min_max.getStart() * -1;
}

int TemporalRandomizer::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    setSeed(iSimulation);
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    const DataTimeRange& range = _time_range_sets.getDataTimeRangeSets().front(); // TODO: for now, only take the first
    DataTimeRange zeroRange(range.getStart() + _zero_translation_additive, range.getEnd() + _zero_translation_additive);
    int TotalSimC = 0;

    if (_parameters.isPerformingDayOfWeekAdjustment()) {
        for (size_t i=0; i < treeNodes.size(); ++i) {
            const NodeStructure::CountContainer_t & counts = treeNodes.getIntC_C(i);
            for (size_t idx=0; idx < counts.size(); ++idx) {
                NodeStructure::count_t cases = counts[idx] - (idx + 1 == counts.size() ? 0 : counts[idx + 1]);
                for (NodeStructure::count_t c=0; c < cases; ++c) {
                    // For the associated day of week, by this idx, get the uniformly distributed time index along all of the same week day.
                    DataTimeRange::index_t idxDay = static_cast<DataTimeRange::index_t>(Equilikely(static_cast<long>(1), static_cast<long>(_day_of_week_indexes[idx % 7].size()), _random_number_generator));
                    ++(treeSimNodes[i].refIntC_C()[_day_of_week_indexes[idx % 7][idxDay - 1]]);
                    ++TotalSimC;
                }

            }
        }
    } else {
        for (size_t i=0; i < treeNodes.size(); ++i) {
            NodeStructure::count_t nodeC = treeNodes.getIntC(i);
            if (nodeC) {
                SimulationNode& simNode(treeSimNodes[i]);
                for (NodeStructure::count_t c=0; c < nodeC; ++c) {
                    DataTimeRange::index_t idx = static_cast<DataTimeRange::index_t>(Equilikely(static_cast<long>(zeroRange.getStart()), static_cast<long>(zeroRange.getEnd()), _random_number_generator));
                    ++(simNode.refIntC_C()[idx]);
                    ++TotalSimC;
                }
            }
        }
    }
    // sanity check
    if (TotalSimC != _total_C)
        throw prg_error("Number of simulated cases does not equal total cases: %d != %d.", "TemporalRandomizer::randomize(...)", TotalSimC, _total_C);
    return _total_C;
}

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
int TemporalRandomizer::RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes) {
    // clear simulation data
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    int TotalSimC = 0;
    if (_read_data) {
        boost::mutex::scoped_lock lock(mutex);
        TotalSimC = read(_read_filename, iSimulation, treeNodes, treeSimNodes);
    } else { // else standard randomization
        TotalSimC = randomize(iSimulation, NodesProxy(treeNodes), treeSimNodes);
    }
    // write simulation data to file if requested
    if (_write_data) {
        boost::mutex::scoped_lock lock(mutex);
        write(_write_filename, treeSimNodes);
    }
    // now set simulation data structures as cumulative
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::setCumulative));
    //------------------------ UPDATING THE TREE -----------------------------------
    for (size_t i=0; i < treeNodes.size(); i++)
        addSimC_C(i, i, treeSimNodes[i].getIntC_C(), treeSimNodes, treeNodes);
    return TotalSimC;
}

/** Reads simulation data from file. */
int TemporalRandomizer::read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) {
    std::ifstream stream;
    if (!stream.is_open()) stream.open(filename.c_str());
    if (!stream) throw resolvable_error("Error: Could not open file '%s' to read the simulated data.\n", filename.c_str());

    int count, total=0;

    //seek line offset for reading iSimulation'th simulation data
    unsigned int skip = static_cast<unsigned int>((simulation - 1) * treeSimNodes.size());
    for (unsigned int i=0; i < skip; ++i)
        stream.ignore(std::numeric_limits<int>::max(), '\n');

    size_t checkNodes = treeSimNodes.size();
    for (size_t i=0; i < treeSimNodes.size(); ++i) {
        SimulationNode& simNode(treeSimNodes[i]);
        int branch = 0;
        size_t checkC = simNode.getIntC_C().size();
        for (size_t s=0; s < simNode.getIntC_C().size(); ++s) {
            // check for end of file yet we should have more to read
            if (!(stream >> count) && i < checkNodes && s < checkC) {
                if (stream.eof())
                    throw resolvable_error("Error: Simulated data file does not contain enough data in simulation %d. Expecting %d nodes with %d datum but could only read %d nodes with %d datum.\n", simulation, checkNodes, checkC, i+1, s);
                throw resolvable_error("Error: Simulated data file appears to contain invalid data in simulation %d. Datum could not be read as integer for element %d of node %d.\n", simulation, s, i+1);
            }

            simNode.refIntC_C()[s] = count;
            branch += count;
        }
        total += branch;
        // check that the total read count equals node branch count?
        if (branch != treeNodes[i]->getBrC())
            throw resolvable_error("Error: Simulation count (%d) for node %s in simulated data file does not equal branch count (%d).\n",
                                   branch, treeNodes[i]->getIdentifier().c_str(), treeNodes[i]->getBrC());
        treeSimNodes[i].refBrC() = 0;
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
        const SimulationNode& simNode(treeSimNodes[i]);
        for (size_t s=0; s < simNode.getIntC_C().size(); ++s) {
            stream << simNode.getIntC_C()[s] << " ";
        }
        stream << std::endl;
    }
    stream.close();
}

//********** ConditionalTemporalRandomizer **********

ConditionalTemporalRandomizer::ConditionalTemporalRandomizer(const ScanRunner& scanner, long lInitialSeed)
            : TemporalRandomizer(scanner, lInitialSeed), AbstractPermutedDataRandomizer(scanner.getParameters(), lInitialSeed) {

    StationaryContainerCollection_t::iterator stationaryItr;
    PermutedContainerCollection_t::iterator permutedItr;
    // determine which collection of attributes to add these patients.
    if (!_dayOfWeekAdjustment) {
        stationaryItr = gvStationaryAttributeCollections.begin();
        permutedItr = gvOriginalPermutedAttributeCollections.begin();
    }
    // assign stationary and permuted data from scanner nodes
    const ScanRunner::NodeStructureContainer_t & nodes = scanner.getNodes();
    ScanRunner::NodeStructureContainer_t::const_iterator itr=nodes.begin(), itr_end=nodes.end();
    for (;itr != itr_end; ++itr) {
        const NodeStructure::CountContainer_t& counts = (*itr)->getIntC_C();
        for (size_t timeIdx=0; timeIdx < counts.size(); ++timeIdx) {
            if (_dayOfWeekAdjustment) {
                stationaryItr = gvStationaryAttributeCollections.begin() + static_cast<size_t>(timeIdx % 7);
                permutedItr = gvOriginalPermutedAttributeCollections.begin() + static_cast<size_t>(timeIdx % 7);
            }
            NodeStructure::count_t num_cases = counts[timeIdx] -  (timeIdx + 1 >= counts.size() ? 0 : counts[timeIdx + 1]);
            for (NodeStructure::count_t c=0; c < num_cases; ++c) {
                //add stationary values
                stationaryItr->push_back(ConditionalTemporalStationary_t((*itr)->getID()));
                //add permutated value
                permutedItr->push_back(ConditionalTemporalPermuted_t(timeIdx));
            }
        }
    }
}

/** Assigns randomized case data to tree simulation node data structures. */
void ConditionalTemporalRandomizer::AssignRandomizedData(const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    int TotalSimC = 0;
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    // iterate through each separate collection of stationary/permuted collections
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        // for each stationary/permutation pair, updating the number of cases for node/time
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ++(treeSimNodes[itrS->GetStationaryVariable()].refIntC_C()[(*itrP).GetPermutedVariable()]);
            ++TotalSimC;
        }
    }
    if (TotalSimC != _total_C) // sanity check
        throw prg_error("Number of simulated cases does not equal total cases: %d != %d.", "ConditionalTemporalRandomizer::AssignRandomizedData(...)", TotalSimC, _total_C);
}
