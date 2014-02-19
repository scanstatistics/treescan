//******************************************************************************
#ifndef __Randomization_H
#define __Randomization_H
//******************************************************************************
#include "RandomNumberGenerator.h"
#include "RandomDistribution.h"
#include "ScanRunner.h"
#include "boost/thread/mutex.hpp"

class SimulationNode {
private:
    NodeStructure::CountContainer_t _IntC_C;
    NodeStructure::CountContainer_t _BrC_C;

public:
    SimulationNode(size_t container_size=1) {
        _IntC_C.resize(container_size, 0);
        _BrC_C.resize(container_size, 0);
    }

    void clear() {
        std::fill(_IntC_C.begin(), _IntC_C.end(), 0);
        std::fill(_BrC_C.begin(), _BrC_C.end(), 0);
    }
    void setCumulative() {
        TreeScan::cumulative(_IntC_C);
        TreeScan::cumulative(_BrC_C);
    }

    NodeStructure::count_t                     getIntC() const {return _IntC_C.front();}
    const NodeStructure::CountContainer_t    & getIntC_C() const {return _IntC_C;}
    NodeStructure::count_t                     getBrC() const {return _BrC_C.front();}
    const NodeStructure::CountContainer_t    & getBrC_C() const {return _BrC_C;}

    NodeStructure::count_t                   & refIntC() {return _IntC_C.front();}
    NodeStructure::CountContainer_t          & refIntC_C() {return _IntC_C;}
    NodeStructure::count_t                   & refBrC() {return _BrC_C.front();}
    NodeStructure::CountContainer_t          & refBrC_C() {return _BrC_C;}
};

//typedef std::pair<int,int> SimData_t;
//typedef std::vector<SimData_t> SimDataContainer_t;

typedef std::vector<SimulationNode> SimNodeContainer_t;

/* abstract data randomizer base class */
class AbstractRandomizer {
protected:
    RandomNumberGenerator _randomNumberGenerator;  /** generates random numbers */
    const Parameters& _parameters;

    void addSimC_C(size_t id, NodeStructure::CountContainer_t& c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes);
    void addSimC_CAnforlust(size_t id, NodeStructure::CountContainer_t& c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes);
    void SetSeed(unsigned int iSimulationIndex);

    virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) = 0;
    virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) = 0;

public:
    AbstractRandomizer(const Parameters& parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : _parameters(parameters), _randomNumberGenerator(lInitialSeed) {}
    virtual ~AbstractRandomizer() {}

    virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes) = 0;
    static AbstractRandomizer * getNewRandomizer(const Parameters& parameters, int TotalC, int TotalControls, double TotalN);
};
//******************************************************************************
#endif
