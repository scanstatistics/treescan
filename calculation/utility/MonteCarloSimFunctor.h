//******************************************************************************
#ifndef __MonteCarloSimFunctor_H
#define __MonteCarloSimFunctor_H
//******************************************************************************
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "ScanRunner.h"
#include "MCSimJobSource.h"
#include "Randomization.h"

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

    successful_result_type scanTree(param_type const & param);
    successful_result_type scanTreeTemporal(param_type const & param);
    successful_result_type scanTreeTemporalConditional(param_type const & param);

public:
    MCSimSuccessiveFunctor(boost::mutex& mutex, boost::shared_ptr<AbstractRandomizer> randomizer, const ScanRunner& scanRunner);
    //~MCSimSuccessiveFunctor() {}
    result_type operator() (param_type const & param);
};
//******************************************************************************
#endif
