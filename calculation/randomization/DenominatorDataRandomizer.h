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

/** Abstraction for denominator data randomizer. */
class AbstractDenominatorDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   _binomial_generator;
    boost::shared_ptr<std::ifstream> _sim_stream;
    unsigned int        _sim_position;
    boost::shared_ptr<OrderedSimulationDataWriter> _sim_stream_writer;

    //virtual int randomize(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) = 0;

    virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes);
    virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes);

  public:
      AbstractDenominatorDataRandomizer(const Parameters& parameters, bool multiparents, long lInitialSeed = RandomNumberGenerator::glDefaultSeed);
    virtual ~AbstractDenominatorDataRandomizer() {}

    virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes);
};
//******************************************************************************
#endif

