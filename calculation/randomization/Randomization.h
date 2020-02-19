//******************************************************************************
#ifndef __Randomization_H
#define __Randomization_H
//******************************************************************************
#include "RandomNumberGenerator.h"
#include "RandomDistribution.h"
#include "ScanRunner.h"
#include "RelativeRiskAdjustment.h"
#include "PrjException.h"
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
        TreeScan::cumulative_backward(_IntC_C);
        TreeScan::cumulative_backward(_BrC_C);
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

class AbstractNodesProxy {
    public:
        AbstractNodesProxy() {}
        virtual ~AbstractNodesProxy() {}

        virtual AbstractNodesProxy * clone() = 0;
        virtual size_t   size() const = 0;
        virtual double   getIntN(size_t i) const = 0;
        virtual const NodeStructure::ExpectedContainer_t & getIntN_C(size_t i) const = 0;
        virtual int      getIntC(size_t i) const = 0;
        virtual const NodeStructure::CountContainer_t & getIntC_C(size_t i) const = 0;
        virtual int      getBrC(size_t i) const = 0;
        virtual double   getProbability(size_t i) const = 0;

        virtual int getID(size_t i) const = 0;
};

class NodesProxy : public AbstractNodesProxy {
    protected:
        const ScanRunner::NodeStructureContainer_t & _treeNodes;
        const double _event_probability;

    public:
        NodesProxy(const ScanRunner::NodeStructureContainer_t& treeNodes, double event_probability=0) : _treeNodes(treeNodes), _event_probability(event_probability) {}
        virtual ~NodesProxy() {}

        virtual NodesProxy * clone() {return new NodesProxy(*this);}
        virtual size_t  size() const {return _treeNodes.size();}
        virtual double  getIntN(size_t i) const {return _treeNodes[i]->getIntN();}
        virtual const NodeStructure::ExpectedContainer_t & getIntN_C(size_t i) const {return _treeNodes[i]->getIntN_C();}
        virtual int     getIntC(size_t i) const {return _treeNodes[i]->getIntC();}
        virtual const NodeStructure::CountContainer_t & getIntC_C(size_t i) const {return _treeNodes[i]->getIntC_C();}
        virtual int     getBrC(size_t i) const {return _treeNodes[i]->getBrC();}
        virtual double  getProbability(size_t i) const {return _event_probability;}
        virtual int getID(size_t i) const {return _treeNodes[i]->getID();}
};

class SequentialNodesProxy : public AbstractNodesProxy {
protected:
    const ScanRunner::NodeStructureContainer_t & _treeNodes;
    const double _event_probability;

public:
    SequentialNodesProxy(const ScanRunner::NodeStructureContainer_t& treeNodes, double event_probability=0) : _treeNodes(treeNodes), _event_probability(event_probability) {}
    virtual ~SequentialNodesProxy() {}

    virtual SequentialNodesProxy * clone() { return new SequentialNodesProxy(*this); }
    virtual size_t  size() const { return _treeNodes.size(); }
    virtual double  getIntN(size_t i) const { return _treeNodes[i]->getIntN_Seq_New(); }
    virtual const NodeStructure::ExpectedContainer_t & getIntN_C(size_t i) const { throw prg_error("NodesProxy::getIntN_C(size_t) not implemented.", "getIntN_C(size_t)"); }
    virtual int     getIntC(size_t i) const { throw prg_error("SequentialNodesProxy::getIntC(size_t) not implemented.", "getIntC(size_t)"); }
    virtual const NodeStructure::CountContainer_t & getIntC_C(size_t i) const { throw prg_error("SequentialNodesProxy::getIntC_C(size_t) not implemented.", "getIntC_C(size_t)"); }
    virtual int     getBrC(size_t i) const { throw prg_error("SequentialNodesProxy::getBrC(size_t) not implemented.", "getBrC(size_t)"); }
    virtual double  getProbability(size_t i) const { return _event_probability; }
    virtual int getID(size_t i) const { return _treeNodes[i]->getID(); }
};


class AlternativeExpectedNodesProxy : public NodesProxy {
    protected:
        const RelativeRiskAdjustmentHandler::NodesExpectedContainer_t & _tree_nodes_expected;

    public:
        AlternativeExpectedNodesProxy(const ScanRunner::NodeStructureContainer_t& treeNodes,
                                      const RelativeRiskAdjustmentHandler::NodesExpectedContainer_t& tree_nodes_expected)
          : NodesProxy(treeNodes),  _tree_nodes_expected(tree_nodes_expected) {}
        virtual ~AlternativeExpectedNodesProxy() {}

        virtual AlternativeExpectedNodesProxy * clone() {return new AlternativeExpectedNodesProxy(*this);}
        virtual double  getIntN(size_t i) const {return _tree_nodes_expected[i].front();}
        virtual const NodeStructure::ExpectedContainer_t & getIntN_C(size_t i) const {return _tree_nodes_expected[i];}
        virtual double  getProbability(size_t i) const {throw prg_error("AlternativeExpectedNodesProxy::getProbability(size_t) not implemented.","getProbability(size_t)");}
};

class AlternativeProbabilityNodesProxy : public NodesProxy {
    protected:
        RelativeRiskAdjustmentHandler::NodesAdjustmentsContainer_t _treeNodeProbability;

    public:
        AlternativeProbabilityNodesProxy(const ScanRunner::NodeStructureContainer_t& treeNodes,
                                         const RelativeRiskAdjustmentHandler& handler,
                                         double event_probability) : NodesProxy(treeNodes, event_probability) {
            _treeNodeProbability.resize(treeNodes.size(), event_probability);
            handler.getAsProbabilities(_treeNodeProbability, event_probability);
        }
        virtual ~AlternativeProbabilityNodesProxy() {}

        virtual AlternativeProbabilityNodesProxy * clone() {return new AlternativeProbabilityNodesProxy(*this);}
        virtual size_t  size() const {return _treeNodes.size();}
        virtual double  getIntN(size_t i) const {return _treeNodes[i]->getIntN();}
        virtual const NodeStructure::ExpectedContainer_t & getIntN_C(size_t i) const {throw prg_error("AlternativeProbabilityNodesProxy::getIntN_C(size_t) not implemented.","getIntN_C(size_t)");}
        virtual int     getIntC(size_t i) const {return _treeNodes[i]->getIntC();}
        virtual const NodeStructure::CountContainer_t & getIntC_C(size_t i) const {throw prg_error("AlternativeProbabilityNodesProxy::getIntC_C(size_t) not implemented.","getIntC_C(size_t)");}
        virtual int     getBrC(size_t i) const {return _treeNodes[i]->getBrC();}
        virtual double  getProbability(size_t i) const {return _treeNodeProbability[i];}

        virtual int getID(size_t i) const {throw prg_error("AlternativeProbabilityNodesProxy::getID(size_t) not implemented.","getID(size_t)");}
};

typedef std::vector<SimulationNode> SimNodeContainer_t;

/* abstract data randomizer base class */
class AbstractRandomizer {
    friend class AlternativeHypothesisRandomizater;

    private:
        void _addSimC_C_ancestor_list(size_t source_id, const NodeStructure::CountContainer_t& c, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes);
        void _addSimC_C_recursive(size_t id, const NodeStructure::CountContainer_t& c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes);

    protected:
        RandomNumberGenerator _random_number_generator;  /** generates random numbers */
        const Parameters& _parameters;
        bool _read_data;
        std::string _read_filename;
        bool _write_data;
        std::string _write_filename;
        bool _multiparents;

        void addSimC_C(size_t source_id, size_t target_id, const NodeStructure::CountContainer_t& c, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes);
        void setSeed(unsigned int iSimulationIndex);
        virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) = 0;
        virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) = 0;
        virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) = 0;

    public:
        AbstractRandomizer(const Parameters& parameters, bool multiparents, long lInitialSeed=RandomNumberGenerator::glDefaultSeed) 
            : _parameters(parameters), _random_number_generator(lInitialSeed), _read_data(false), _write_data(false), _multiparents(multiparents) {}
        virtual ~AbstractRandomizer() {}

        virtual AbstractRandomizer * clone() const = 0;
        static AbstractRandomizer * getNewRandomizer(const ScanRunner& scanner);
        virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes) = 0;
        void setReading(const std::string& s) {_read_filename = s; _read_data = true;}
        void setWriting(const std::string& s) {_write_filename = s; _write_data = true;}
};
//******************************************************************************
#endif
