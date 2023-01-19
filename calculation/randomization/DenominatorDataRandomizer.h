//******************************************************************************
#ifndef __DenominatorDataRandomizer_H
#define __DenominatorDataRandomizer_H
//******************************************************************************
#include "Randomization.h"

/** Class which ensures that simulation data is written to file in ordinal sequence (sim 1, sim 2, sim 3, etc.). 
    Class objects which are stream derivatives are allocated with shared pointers to facilitate randomizer clone. */
class OrderedSimulationDataWriter {
    protected:
        typedef boost::shared_ptr<std::stringstream> CacheStream_t;
        typedef std::pair<unsigned int, CacheStream_t> CacheStreamPair_t;
        typedef std::vector<CacheStreamPair_t> CacheStreamContainer_t;

        unsigned int _sim_position;
        unsigned int _num_simulations;
        boost::shared_ptr<std::ofstream> _sim_stream;
        CacheStreamContainer_t _sim_stream_cache;

    public:
        OrderedSimulationDataWriter(const std::string& filename, unsigned int num_simulations);

        void write(unsigned int simulation, const SimNodeContainer_t& treeSimNodes);
};

/** Class which manages a std::ifstream associated with a randomizer object. This object can be shared by randomizer objects. */
class FileStreamReadManager {
    public:
        typedef boost::shared_ptr<std::ifstream> SharedStream_t;
        typedef std::pair<const AbstractRandomizer*, SharedStream_t> ManagedStream_t;

    protected:
        std::vector<ManagedStream_t> _managed_streams;
        const std::string _filename;

    public:
        FileStreamReadManager(const std::string& filename) : _filename(filename) {}
        virtual ~FileStreamReadManager() {
            try {
                for (auto it=_managed_streams.begin(); it != _managed_streams.end(); ++it)
                    it->second->close();
            } catch (...) {}
        }

        SharedStream_t getStream(const AbstractRandomizer * r, boost::mutex& mutex);
};

/** Abstraction for denominator data randomizer. */
class AbstractDenominatorDataRandomizer : public AbstractRandomizer {
  protected:
    typedef boost::shared_ptr<OrderedSimulationDataWriter> SharedSimWriter_t;
    typedef boost::shared_ptr<FileStreamReadManager> SharedSimReader_t;

    BinomialGenerator   _binomial_generator;
    unsigned int        _sim_position;
    SharedSimWriter_t   _sim_stream_writer;
    SharedSimReader_t   _sim_stream_reader;

    //virtual int randomize(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) = 0;

    virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes, boost::mutex& mutex);
	void sequentialSetup(const ScanRunner& scanner);
    virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes);

  public:
      AbstractDenominatorDataRandomizer(const Parameters& parameters, bool multiparents, long lInitialSeed = RandomNumberGenerator::glDefaultSeed);
      virtual ~AbstractDenominatorDataRandomizer();

    virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes);
};
//******************************************************************************
#endif

