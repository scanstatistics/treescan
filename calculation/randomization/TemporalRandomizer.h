//******************************************************************************
#ifndef __TemporalRandomizer_H
#define __TemporalRandomizer_H
//******************************************************************************
#include "Randomization.h"
#include "RandomDistribution.h"
#include "PermutationDataRandomizer.h"

/** Abstraction for Poisson data randomizers */
class TemporalRandomizer : public AbstractRandomizer {
protected:
    typedef std::list<std::pair<int, NodeStructure::count_t> > censor_distribution_t;
    typedef std::vector<censor_distribution_t> node_censor_container_t;

    int                         _total_C;
    double                      _total_N;
    const DataTimeRangeSet    & _time_range_sets;
    DataTimeRange::index_t      _zero_translation_additive;
    const ScanRunner::DayOfWeekIndexes_t  & _day_of_week_indexes;
    bool                        _censored_data;
    node_censor_container_t     _node_censors;

    virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);
    virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes, boost::mutex& mutex);
    virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes);

public:
    TemporalRandomizer(const ScanRunner& scanner, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~TemporalRandomizer() {}

    virtual TemporalRandomizer * clone() const {return new TemporalRandomizer(*this);}
    virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes);
};

typedef StationaryAttribute<int> ConditionalTemporalStationary_t; // node Id
typedef PermutedAttribute<size_t> ConditionalTemporalPermuted_t;  // time index

/** Tree temporal scan that conditions on the number of cases on each branch and on the total number of cases on each day summed over all the leaves */
class ConditionalTemporalRandomizer : public TemporalRandomizer, public AbstractPermutedDataRandomizer<ConditionalTemporalStationary_t, ConditionalTemporalPermuted_t> {
    protected:
        virtual void AssignRandomizedData(const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);
        virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
            setSeed(iSimulation);
            //assign random numbers to permuted attribute and sort
            SortPermutedAttribute(_random_number_generator);
            //re-assign dataset's simulation data
            AssignRandomizedData(treeNodes, treeSimNodes);
            return _total_C;
        }

    public:
        ConditionalTemporalRandomizer(const ScanRunner& scanner, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~ConditionalTemporalRandomizer() {}
        virtual ConditionalTemporalRandomizer * clone() const {return new ConditionalTemporalRandomizer(*this);}
};

/** Tree temporal randomizer for alternative hypothesis in power estimations. */
class TemporalAlternativeHypothesisRandomizer : public TemporalRandomizer {
protected:
    virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

public:
    TemporalAlternativeHypothesisRandomizer(const ScanRunner& scanner, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~TemporalAlternativeHypothesisRandomizer() {}

    virtual TemporalAlternativeHypothesisRandomizer * clone() const {return new TemporalAlternativeHypothesisRandomizer(*this);}
};
//******************************************************************************
#endif
