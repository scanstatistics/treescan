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

/* Writes simulation data to text file for given simulation while ensuring that simulations written are ordered by ordinal. */
void OrderedSimulationDataWriter::write(unsigned int simulation, const SimNodeContainer_t& treeSimNodes) {
    if (_sim_position == simulation) { // If simulation position equals this simulation, write it now.
        for (auto& node : treeSimNodes)
            *_sim_stream << node.getIntC() << " ";
        *_sim_stream << std::endl;
        ++_sim_position;
    } else { // Otherwise write this simulation data to cache.
        CacheStream_t _cache_stream(new std::stringstream());
        for (auto& node : treeSimNodes)
            *_cache_stream << node.getIntC() << " ";
        *_cache_stream << std::endl;
        _sim_stream_cache.push_back(std::make_pair(simulation, _cache_stream));
        std::sort(_sim_stream_cache.begin(), _sim_stream_cache.end()); // Sort by simulation number ascending.
    }
    // Now write any cached stream(s) which are next in the write sequence.
    size_t numWritten = 0;
    for (auto& cacheStream: _sim_stream_cache) {
        if (_sim_position != cacheStream.first) break;
        *_sim_stream << cacheStream.second->str();
        ++_sim_position;
        ++numWritten;
    }
    if (numWritten) _sim_stream_cache.erase(_sim_stream_cache.begin(), _sim_stream_cache.begin() + numWritten);
    if (_sim_position > _num_simulations)
        _sim_stream->close();
}

/* Writes sequential simulation data to text file for given simulation while ensuring that simulations written are ordered by ordinal. */
void OrderedSimulationDataWriter::writeSequenceData(unsigned int simulation, const SimNodeContainer_t& treeSimNodes, const boost::dynamic_bitset<>& _restricted_levels) {
    if (_sim_position == simulation) { // If simulation position equals this simulation, write it now.
        for (auto node: treeSimNodes) {
            if (!_restricted_levels.test(node.getLevel())) // skip writing data from nodes that are not evaluated - we're writing branch data
                *_sim_stream << node.getBrC() << " ";
        }
        *_sim_stream << std::endl;
        ++_sim_position;
    } else { // Otherwise write this simulation data to cache.
        CacheStream_t _cache_stream(new std::stringstream());
        for (auto& node : treeSimNodes) {
            if (!_restricted_levels.test(node.getLevel())) // skip writing data from nodes that are not evaluated - we're writing branch data
                *_cache_stream << node.getBrC() << " ";
        }
        *_cache_stream << std::endl;
        _sim_stream_cache.push_back(std::make_pair(simulation, _cache_stream));
        std::sort(_sim_stream_cache.begin(), _sim_stream_cache.end()); // Sort by simulation number ascending.
    }
    // Now write any cached stream(s) which are next in the write sequence.
    size_t numWritten = 0;
    for (auto& cacheStream : _sim_stream_cache) {
        if (_sim_position != cacheStream.first) break;
        *_sim_stream << cacheStream.second->str();
        ++_sim_position;
        ++numWritten;
    }
    if (numWritten) _sim_stream_cache.erase(_sim_stream_cache.begin(), _sim_stream_cache.begin() + numWritten);
    if (_sim_position > _num_simulations)
        _sim_stream->close();
}

FileStreamReadManager::SharedStream_t FileStreamReadManager::getStream(const AbstractRandomizer * r, boost::mutex& mutex) {
    auto it = std::find_if(_managed_streams.begin(), _managed_streams.end(), [r](const ManagedStream_t& element) { return element.first == r; });
    if (it == _managed_streams.end()) {
        boost::mutex::scoped_lock lock(mutex);
        _managed_streams.push_back(std::make_pair(r, boost::shared_ptr<std::ifstream>(new std::ifstream())));
        _managed_streams.back().second->open(_filename.c_str());
        return _managed_streams.back().second;
    }
    return it->second;
}

////////////////////////////////////////////////// AbstractDenominatorDataRandomizer ///////////////////////////////////

AbstractDenominatorDataRandomizer::AbstractDenominatorDataRandomizer(const ScanRunner& scanner, long lInitialSeed) :
    AbstractRandomizer(scanner.getParameters(), scanner.getMultiParentNodesExist(), lInitialSeed), _scanner(scanner), _sim_position(1) {
    // Store the un-evaluated levelsin tree for in bitsetfor tree sequential analyses.
    if (_parameters.isSequentialScanTreeOnly()) {
        _restricted_levels.resize(scanner.getTreeStatistics()._nodes_per_level.size() + 1);
        if (_parameters.getRestrictTreeLevels()) {
            for (auto level : _parameters.getRestrictedTreeLevels())
                _restricted_levels.set(level);
        }
    }
}

AbstractDenominatorDataRandomizer::~AbstractDenominatorDataRandomizer() {}

int AbstractDenominatorDataRandomizer::RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes) {
    int totalSimC = 0;
    auto updateTree = [&treeNodes, &treeSimNodes, this]() { // update tree structure by adding counts up the tree
        for (size_t i = 0; i < treeNodes.size(); i++)
            addSimC_C(i, i, treeSimNodes[i].getIntC_C(), treeSimNodes, treeNodes);
    };
    auto standardWrite = [&treeSimNodes, &mutex, this]() { // standard simulation data write
        boost::mutex::scoped_lock lock(mutex);
        write(_write_filename, treeSimNodes);
    };
    if (_read_data) {
        std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));
        boost::mutex::scoped_lock lock(mutex);
        totalSimC = read(_read_filename, iSimulation, treeNodes, treeSimNodes, mutex);
        if (_write_data) standardWrite(); // write simulation data to file if requested
        updateTree();
    } else if (_parameters.isSequentialScanTreeOnly()) {
        totalSimC = randomize(iSimulation, SequentialNodesProxy(treeNodes, _parameters.getDataOnlyOnLeaves(), _parameters.getProbability()), treeSimNodes);
        if (_write_data) standardWrite(); // write simulation data to file if requested
        // update tree now since we will be reading and writing data that has been added up the tree
        updateTree();
        if (_read_filename.size()) // add cumulated simulation data from previous look(s)
            totalSimC += readSequentialData(_read_filename, iSimulation, treeNodes, treeSimNodes, mutex);
        boost::mutex::scoped_lock writelock(mutex);
        _sim_stream_writer->writeSequenceData(iSimulation, treeSimNodes, _restricted_levels);
    } else { // else standard randomization
        totalSimC = randomize(iSimulation, NodesProxy(treeNodes, _parameters.getDataOnlyOnLeaves(), _parameters.getProbability()), treeSimNodes);
        if (_write_data) standardWrite(); // write simulation data to file if requested
        updateTree();
    }
    return totalSimC;
}

/** Reads simulation data from file. */
int AbstractDenominatorDataRandomizer::read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes, boost::mutex& mutex) {
    FileStreamReadManager::SharedStream_t _sim_stream = _sim_stream_reader->getStream(this, mutex);
    // seek line offset from current simulation row to this simulation
    while (_sim_position < simulation) {
        _sim_stream->ignore(std::numeric_limits<int>::max(), _sim_stream->widen('\n'));
        ++_sim_position;
    }
    size_t checkNodes = treeSimNodes.size();
    int count, total_sim = 0;
    for (size_t i=0; i < checkNodes; ++i) {
        auto& node = treeSimNodes[i];
        // check for end of file yet we should have more to read
        *_sim_stream >> count;
        if (_sim_stream->fail()) throw resolvable_error("Error: Simulated data file appears to contain invalid data in simulation %d. Datum could not be read as integer for %d element.\n", simulation, i + 1);
        node.refIntC() += count;
        node.refBrC() = 0;
        total_sim += count;
    }
    return total_sim;
}

/** Reads sequential simulation data from file. */
int AbstractDenominatorDataRandomizer::readSequentialData(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes, boost::mutex& mutex) {
    FileStreamReadManager::SharedStream_t _sim_stream = _sim_stream_reader->getStream(this, mutex);
    // seek line offset from current simulation row to this simulation

    while (_sim_position < simulation) {
        _sim_stream->ignore(std::numeric_limits<int>::max(), _sim_stream->widen('\n'));
        ++_sim_position;
    }
    size_t checkNodes = treeSimNodes.size();
    int count, total_sim = 0;
    for (size_t i = 0; i < checkNodes; ++i) {
        auto& node = treeSimNodes[i];
        if (_restricted_levels.test(node.getLevel()))
            continue; // these nodes were skipped during write process, so skip on read
                      // check for end of file yet we should have more to read
        *_sim_stream >> count;
        if (_sim_stream->fail()) throw resolvable_error("Error: Simulated data file appears to contain invalid data in simulation %d. Datum could not be read as integer for %d element.\n", simulation, i + 1);
        node.refBrC() += count; // We're reading branch data, not node data.
        total_sim += count;
    }
    return total_sim;
}

void AbstractDenominatorDataRandomizer::sequentialSetup(const ScanRunner& scanner) {
	if (scanner.getParameters().isSequentialScanTreeOnly()) {
		if (!scanner.getSequentialStatistic().isFirstLook())
			_read_filename = scanner.getSequentialStatistic().getSimulationDataFilename();
		_write_filename = scanner.getSequentialStatistic().getWriteSimulationDataFilename();
		// Create new OrderedSimulationDataWriter in explicit constructor. Since it's a shared pointer, clones will share this class.
		_sim_stream_writer.reset(new OrderedSimulationDataWriter(_write_filename, scanner.getParameters().getNumReplicationsRequested()));
		// Create new FileStreamReadManager in explicit constructor. Since it's a shared pointer, clones will share this class.
		_sim_stream_reader.reset(new FileStreamReadManager(_read_filename));
	}
}

/** Writes simulation data to file. */
void AbstractDenominatorDataRandomizer::write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) {
    std::ofstream stream;

    //open output file
    stream.open(filename.c_str(), std::ios::ate|std::ios::app);
    if (!stream) throw resolvable_error("Error: Could not open the simulated data output file '%s'.\n", filename.c_str());
    for (auto& node: treeSimNodes)
        stream << node.getIntC() << " ";
    stream << std::endl;
    stream.close();
}
