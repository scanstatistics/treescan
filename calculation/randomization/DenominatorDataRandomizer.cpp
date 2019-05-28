//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include <iostream>
#include <fstream>

int AbstractDenominatorDataRandomizer::RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes) {
    int TotalSimC;
    if (_read_data) {
        std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));
        boost::mutex::scoped_lock lock(mutex);
        TotalSimC = read(_read_filename, iSimulation, treeNodes, treeSimNodes);
    } else if (_parameters.isSequentialScanBernoulli()) {
        TotalSimC = randomize(iSimulation, SequentialNodesProxy(treeNodes, _parameters.getProbability()), treeSimNodes);
        // Now read stored data set
        if (_read_filename.size()) {
            boost::mutex::scoped_lock readlock(mutex);
            TotalSimC += read(_read_filename, iSimulation, treeNodes, treeSimNodes);
        }
        boost::mutex::scoped_lock writelock(mutex);
        write(_write_filename, treeSimNodes);
    } else { // else standard randomization
        TotalSimC = randomize(iSimulation, NodesProxy(treeNodes, _parameters.getProbability()), treeSimNodes);
    }
    // write simulation data to file if requested
    if (_write_data) {
        boost::mutex::scoped_lock lock(mutex);
        write(_write_filename, treeSimNodes);
    }
    //------------------------ UPDATING THE TREE -----------------------------------
    for (size_t i=0; i < treeNodes.size(); i++)
        addSimC_C(i, i, treeSimNodes[i].getIntC_C(), treeSimNodes, treeNodes);
    return TotalSimC;
}

/** Reads simulation data from file. */
int AbstractDenominatorDataRandomizer::read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) {
    std::ifstream stream;
    if (!stream.is_open()) stream.open(filename.c_str());
    if (!stream) throw resolvable_error("Error: Could not open file '%s' to read the simulated data.\n", filename.c_str());

    int count, total_sim=0;

    //seek line offset for reading simulation'th simulation data
    unsigned int skip = simulation - 1;
    for (unsigned int i=0; i < skip; ++i)
        stream.ignore(std::numeric_limits<int>::max(), '\n');

    size_t checkNodes = treeSimNodes.size();
    for (size_t i=0; i < treeSimNodes.size(); ++i) {
        // check for end of file yet we should have more to read
        if (!(stream >> count) && i < checkNodes) {
            if (stream.eof())
                throw resolvable_error("Error: Simulated data file does not contain enough data for simulation %d. Expecting %d datum but could only read %d.\n", simulation, checkNodes, i);
            throw resolvable_error("Error: Simulated data file appears to contain invalid data in simulation %d. Datum could not be read as integer for %d element.\n", simulation, i+1);
        }
        treeSimNodes[i].refIntC() += count;
        treeSimNodes[i].refBrC() = 0;
        total_sim += count;
    }
    stream.close();

    return total_sim;
}

/** Writes simulation data to file. */
void AbstractDenominatorDataRandomizer::write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) {
    std::ofstream stream;

    //open output file
    stream.open(filename.c_str(), std::ios::ate|std::ios::app);
    if (!stream) throw resolvable_error("Error: Could not open the simulated data output file '%s'.\n", filename.c_str());
    for (size_t i=0; i < treeSimNodes.size(); ++i) {
        stream << treeSimNodes[i].getIntC() << " ";
    }
    stream << std::endl;
    stream.close();
}
