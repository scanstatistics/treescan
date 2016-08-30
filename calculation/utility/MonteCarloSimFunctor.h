//******************************************************************************
#ifndef __MonteCarloSimFunctor_H
#define __MonteCarloSimFunctor_H
//******************************************************************************
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "ScanRunner.h"
#include "MCSimJobSource.h"
#include "Randomization.h"
#include "WindowLength.h"

class AbstractMeasureList {
    protected:
        typedef std::vector<double> list_container_t;
        typedef boost::shared_ptr<list_container_t> list_t;

        const ScanRunner& _scanRunner;
        ScanRunner::Loglikelihood_t _loglikelihood;

    public:
        AbstractMeasureList(const ScanRunner & scanRunner, ScanRunner::Loglikelihood_t loglikelihood) : _scanRunner(scanRunner), _loglikelihood(loglikelihood) {}
        virtual ~AbstractMeasureList() {}

        virtual void add(int c, double n) = 0;
        virtual void initialize() = 0;
        virtual double loglikelihood() = 0;

        static AbstractMeasureList * getNewMeasureList(const ScanRunner& scanner, ScanRunner::Loglikelihood_t loglikelihood);
};

/** Non Measurelist class -- simply stores greatest LLR. */
class NonMeasureList : public AbstractMeasureList {
    protected:
        double _max_loglikelihood;

    public:
        NonMeasureList(const ScanRunner & scanRunner, ScanRunner::Loglikelihood_t loglikelihood) : AbstractMeasureList(scanRunner, loglikelihood), _max_loglikelihood(-std::numeric_limits<double>::max()) {}
        virtual ~NonMeasureList() {}

        virtual void add(int c, double n) { _max_loglikelihood = std::max(_max_loglikelihood, _loglikelihood->LogLikelihood(c, n)); }
        virtual void initialize() {_max_loglikelihood = -std::numeric_limits<double>::max();}
        virtual double loglikelihood() { return _max_loglikelihood; }
};

/** Scan for areas with more than expected cases. */
class MinimumMeasureList : public AbstractMeasureList {
    protected:
        list_t _min_measure;

    public:
        MinimumMeasureList(const ScanRunner & scanRunner, ScanRunner::Loglikelihood_t loglikelihood) : AbstractMeasureList(scanRunner, loglikelihood) {
            _min_measure.reset(new list_container_t(_scanRunner.getTotalC() + 1));
        }
        virtual ~MinimumMeasureList() {}

        virtual void add(int c, double n) {
            assert(c >= 0);
            if (c < 0) exit(1);
            if ((*_min_measure)[c] > n)
                (*_min_measure)[c] = n;
        }
        virtual void initialize() {
            list_container_t& measure = (*_min_measure);
            if (_scanRunner.getParameters().getModelType() == Parameters::BERNOULLI) {
                double totalC = static_cast<double>(_scanRunner.getTotalC());
                double totalN = _scanRunner.getTotalN();
                for (list_container_t::size_type i=0; i < _min_measure->size(); ++i)
                    measure[i] = (totalN *  static_cast<double>(i)) / totalC;
            } else {
                for (list_container_t::size_type i=0; i < _min_measure->size(); ++i)
                    measure[i] = static_cast<double>(i);
            }
        }
        virtual double loglikelihood() {
            double simLogLikelihood = -std::numeric_limits<double>::max(),  max_excess(0);
            list_container_t::size_type iListSize = static_cast<list_container_t::size_type>(_scanRunner.getTotalC()),
                                        iHalfListSize = static_cast<list_container_t::size_type>(iListSize/2);
            /* Start case index at two -- don't want to consider simulations with one case as this could indicate a false high loglikelihood. */
            list_container_t::size_type i = 2;

            list_container_t& measure = (*_min_measure);
            if (_scanRunner.getParameters().getModelType() == Parameters::BERNOULLI) {
                double total_measure(_scanRunner.getTotalN()), risk(static_cast<double>(_scanRunner.getTotalC())/_scanRunner.getTotalN());
                /* Calculating the LLR for less than half the cases can use a trick where the calculation is performed only if the excess exceeds any previous excess. */
                for (; i < iHalfListSize; ++i) {
                    if (static_cast<double>(i) - measure[i] * risk > max_excess) {
                        max_excess = static_cast<double>(i) - measure[i] * risk;
                        simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(static_cast<int>(i), measure[i]));
                    }
                }
                /* Calculate LLR for remaining half - trick not valid when number of cases is greater than or equal half. */
                list_container_t::size_type i=std::max(iHalfListSize, static_cast<list_container_t::size_type>(2));
                for (; i <= iListSize; ++i) {
                    if (measure[i] != 0.0 && static_cast<double>(i) * total_measure > measure[i] * static_cast<double>(iListSize)) {
                        simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(static_cast<int>(i), measure[i]));
                    }
                }
            } else {
                /* Calculating the LLR for less than half the cases can use a trick where the calculation is performed only if the excess exceeds any previous excess. */
                for (; i < iHalfListSize; ++i) {
                    if (static_cast<double>(i) - measure[i] > max_excess) {
                        max_excess = static_cast<double>(i) - measure[i];
                        simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(static_cast<int>(i), measure[i]));
                    }
                }
                /* Calculate LLR for remaining half - trick not valid when number of cases is greater than or equal half. */
                list_container_t::size_type i=std::max(iHalfListSize, static_cast<list_container_t::size_type>(2));
                for (; i <= iListSize; ++i) {
                    if (measure[i] != 0.0 && static_cast<double>(i) > measure[i]) {
                        simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(static_cast<int>(i), measure[i]));
                    }
                }
            }
            return simLogLikelihood;
        }
};

//runs jobs for the "successive" algorithm
class MCSimSuccessiveFunctor {
public:
    typedef unsigned int param_type;
    typedef MCSimJobSource::result_type result_type;
    typedef MCSimJobSource::successful_result_type successful_result_type;
    
private:
    boost::mutex                            & _mutex;
    SimNodeContainer_t                        _treeSimNodes;
    ScanRunner::Loglikelihood_t               _loglikelihood;
    boost::shared_ptr<AbstractRandomizer>     _randomizer;
    const ScanRunner                        & _scanRunner;

    //boost::shared_ptr<std::vector<double> >     _measure;
    boost::shared_ptr<AbstractMeasureList>     _measure_list;

	bool isEvaluated(const NodeStructure& node, const SimulationNode& simNode) const;
    successful_result_type scanTree(param_type const & param);
    successful_result_type scanTreeTemporalConditionNode(param_type const & param);
    successful_result_type scanTreeTemporalConditionNodeTime(param_type const & param);

public:
    MCSimSuccessiveFunctor(boost::mutex& mutex, boost::shared_ptr<AbstractRandomizer> randomizer, const ScanRunner& scanRunner);
    //~MCSimSuccessiveFunctor() {}
    result_type operator() (param_type const & param);
};

//runs jobs for the "successive" algorithm for the sequential purely temporal scan
class SequentialMCSimSuccessiveFunctor {
public:
    typedef unsigned int param_type;
    typedef MCSimJobSource::result_type result_type;
    typedef MCSimJobSource::successful_result_type successful_result_type;
    
private:
    boost::mutex & _mutex;
    boost::shared_ptr<SimulationNode> _treeSimNode;
    ScanRunner::Loglikelihood_t _loglikelihood;
    const ScanRunner & _scanRunner;
    RandomNumberGenerator _random_number_generator;  /** generates random numbers */
    DataTimeRange _range;
    DataTimeRange _startWindow;
    DataTimeRange _endWindow;
    WindowLength _window;
    boost::shared_ptr<SequentialScanLoglikelihoodRatioWriter> _sequential_writer;

public:
    SequentialMCSimSuccessiveFunctor(boost::mutex& mutex, const ScanRunner& scanner, boost::shared_ptr<SequentialScanLoglikelihoodRatioWriter> writer);
    result_type operator() (param_type const & param);
};

class SequentialFileDataSource;

//runs jobs for the "successive" algorithm for the sequential purely temporal scan
class SequentialReadMCSimSuccessiveFunctor {
public:
    typedef unsigned int param_type;
    typedef MCSimJobSource::result_type result_type;
    typedef MCSimJobSource::successful_result_type successful_result_type;
    
private:
    boost::mutex & _mutex;
    const ScanRunner & _scanRunner;
    boost::shared_ptr<SequentialFileDataSource> _source;

public:
    SequentialReadMCSimSuccessiveFunctor(boost::mutex& mutex, const ScanRunner& scanner, boost::shared_ptr<SequentialFileDataSource> source);
    result_type operator() (param_type const & param);
};
//******************************************************************************
#endif
