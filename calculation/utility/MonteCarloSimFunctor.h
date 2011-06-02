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
    
private:
  boost::mutex                                & gMutex;
  SimDataContainer_t                            _simData;
  ScanRunner::Loglikelihood_t                   _loglikelihood;
  boost::shared_ptr<AbstractRandomizer>         _randomizer;
  const ScanRunner                            & _scanRunner;

public:
  MCSimSuccessiveFunctor(boost::mutex& Mutex, 
                         boost::shared_ptr<AbstractRandomizer> randomizer,
                         const ScanRunner& scanRunner);
  //~MCSimSuccessiveFunctor() {}

  result_type operator() (param_type const & param);

};
//******************************************************************************
#endif

