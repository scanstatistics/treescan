//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include <iostream>
#include <fstream>

////////////////////////////////////////////////// OrderedSimulationDataWriter /////////////////////////////////////////

OrderedSimulationDataWriter::OrderedSimulationDataWriter(const std::string& filename, unsigned int num_simulations) : _sim_position(1), _num_simulations(num_simulations){
    _sim_stream.reset(new std::ofstream());
    _sim_stream->open(filename.c_str(), std::ios::ate | std::ios::app);
}

/* Writes simulation data to text file for gioven simulation while ensuring that simulations written are ordered by ordinal. */
void OrderedSimulationDataWriter::write(unsigned int simulation, const SimNodeContainer_t& treeSimNodes) {
    if (_sim_position == simulation) { // If simulation position equals this simulation, write it now.
        for (size_t i = 0; i < treeSimNodes.size(); ++i)
            *_sim_stream << treeSimNodes[i].getIntC() << " ";
        *_sim_stream << std::endl;
        ++_sim_position;
    } else { // Otherwise write this simulation data to cache.
        CacheStream_t _cache_stream(new std::stringstream());
        for (size_t i = 0; i < treeSimNodes.size(); ++i)
            *_cache_stream << treeSimNodes[i].getIntC() << " ";
        *_cache_stream << std::endl;
        _sim_stream_cache.push_back(std::make_pair(simulation, _cache_stream));
        std::sort(_sim_stream_cache.begin(), _sim_stream_cache.end()); // Sort by simulation number ascending.
    }
    // Now write any cached stream(s) which are next in the write sequence.
    size_t numWritten = 0;
    for(CacheStreamContainer_t::iterator itr= _sim_stream_cache.begin(); itr != _sim_stream_cache.end(); ++itr) {
        if (_sim_position != itr->first) break;
        *_sim_stream << itr->second->str();
        ++_sim_position;
        ++numWritten;
    }
    if (numWritten) _sim_stream_cache.erase(_sim_stream_cache.begin(), _sim_stream_cache.begin() + numWritten);
    if (_sim_position > _num_simulations)
        _sim_stream->close();
}

////////////////////////////////////////////////// AbstractDenominatorDataRandomizer ///////////////////////////////////

AbstractDenominatorDataRandomizer::AbstractDenominatorDataRandomizer(const Parameters& parameters, bool multiparents, long lInitialSeed) :
    AbstractRandomizer(parameters, multiparents, lInitialSeed), _sim_position(0) {}

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
            TotalSimC += read(_read_filename, iSimulation, treeNodes, treeSimNodes);
        }
        boost::mutex::scoped_lock writelock(mutex);
        _sim_stream_writer->write(iSimulation, treeSimNodes);
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
    //std::ifstream stream;
    if (!_sim_stream.get()) _sim_stream.reset(new std::ifstream());
    if (!_sim_stream->is_open()) _sim_stream->open(filename.c_str());

    int count, total_sim=0;
    //seek line offset for reading simulation'th simulation data
    unsigned int skip = simulation - 1;
    for (; _sim_position < skip; ++_sim_position)
        _sim_stream->ignore(std::numeric_limits<int>::max(), '\n');

    size_t checkNodes = treeSimNodes.size();
    for (size_t i=0; i < treeSimNodes.size(); ++i) {
        // check for end of file yet we should have more to read
        if (!(*_sim_stream >> count) && i < checkNodes) {
            if (_sim_stream->eof())
                throw resolvable_error("Error: Simulated data file does not contain enough data for simulation %d. Expecting %d datum but could only read %d.\n", simulation, checkNodes, i);
            throw resolvable_error("Error: Simulated data file appears to contain invalid data in simulation %d. Datum could not be read as integer for %d element.\n", simulation, i+1);
        }
        treeSimNodes[i].refIntC() += count;
        treeSimNodes[i].refBrC() = 0;
        total_sim += count;
    }
    _sim_stream->ignore(1, '\n'); // read possible trailing newline from current data set line
    ++_sim_position; // advance simulation position

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
