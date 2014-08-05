//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "MCSimJobSource.h"
#include "UtilityFunctions.h"

//constructor
MCSimJobSource::MCSimJobSource(boost::posix_time::ptime CurrentTime, PrintQueue & rPrintDirection, const char * szReplicationFormatString, ScanRunner & rRunner, unsigned int num_replica, bool isPowerStep)
 : guiNextJobParam(1)
 , guiUnregisteredJobLowerBound(1)
 , gfnRegisterResult(&MCSimJobSource::RegisterResult_AutoAbort)//initialize to the most feature-laden
 , gConstructionTime(CurrentTime)
 , grPrintDirection(rPrintDirection)
 , gszReplicationFormatString(szReplicationFormatString)
 , grRunner(rRunner)
 , guiJobCount(num_replica)
 , guiNextProcessingJobId(1)
 , guiJobsReported(0)
 , _isPowerStep(isPowerStep)
{
  if (false/*rParameters.GetTerminateSimulationsEarly()*/) {
    gfnRegisterResult = &MCSimJobSource::RegisterResult_AutoAbort;
  }
  else {
    gfnRegisterResult = &MCSimJobSource::RegisterResult_NoAutoAbort;
  }

  grLoglikelihood.reset((AbstractLoglikelihood::getNewLoglikelihood(grRunner.getParameters(), grRunner.getTotalC(), grRunner.getTotalN())));
  if (grRunner.getParameters().isGeneratingLLRResults())
    _ratio_writer.reset(new LoglikelihoodRatioWriter(grRunner, _isPowerStep));
}


void MCSimJobSource::acquire(job_id_type & dst_job_id, param_type & dst_param)
{
  if (is_exhausted())
    throw std::runtime_error("can't acquire a job from an exhausted source.");

  gbsUnregisteredJobs.push_back(true);

  //all exception-unsafe ops have been executed, so do updates:
  dst_param = dst_job_id = guiNextJobParam;
  ++guiNextJobParam;
}

void MCSimJobSource::Assert_NoExceptionsCaught() const
{
  static const char * szExceptionIntroFormatString = "An exception was thrown from simulation #%d.";
  static const char * szExceptionMessageTitle = "\nException message: ";
  static const char * szExceptionCallPathTitle = "\nException call path:\n";

  if (GetExceptionCount() > 0) {
    //scan collection of exceptions for Memory exception type, this type trumps all -- take first
    std::deque<exception_type>::const_iterator itr = gvExceptions.begin();
    for (; itr != gvExceptions.end(); ++itr) {
       if (itr->second.second.eException_type == job_result::memory) {
         std::string sTemp;
         printString(sTemp, szExceptionIntroFormatString, itr->first);
         sTemp += szExceptionMessageTitle;
         sTemp += itr->second.second.Exception.what();
         sTemp += szExceptionCallPathTitle;
         sTemp += itr->second.second.Exception.trace();
         memory_exception MemoryException(sTemp.c_str());
         throw MemoryException;
       }
    }

    CarrierException<exception_sequence_type> lclException(gvExceptions, "", "MCSimJobSource");
    exception_type const & rFirstException(lclException->front());

    // reconstitute resolvable_error and throw
    if (rFirstException.second.second.eException_type == job_result::resolvable)
        throw resolvable_error(rFirstException.second.second.Exception.what());

    std::string sTemp;
    printString(sTemp, szExceptionIntroFormatString, rFirstException.first);
    lclException.addWhat(sTemp.c_str());
    lclException.addWhat(szExceptionMessageTitle);
    lclException.addWhat(rFirstException.second.second.Exception.what());
    lclException.addWhat(szExceptionCallPathTitle);
    lclException.addWhat(rFirstException.second.second.Exception.trace());

    throw lclException;
  }
}

bool MCSimJobSource::CancelRequested() const
{
  return grPrintDirection.GetIsCanceled();
}

//How many jobs have registered a successful result?
//This is all jobs that:
//1. completed without an exception and
//2. were not discarded in the event of an auto-abort condition.
unsigned int MCSimJobSource::GetSuccessfullyCompletedJobCount() const
{
  unsigned int uiResult = guiUnregisteredJobLowerBound-1;
  if (AutoAbortConditionExists())
     uiResult = grRunner.getSimulationVariables().get_sim_count();
  else
    uiResult += (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) - gvExceptions.size();
  return uiResult;
}

//How many jobs are there that have been acquired but whose results have not
//been registered?
unsigned int MCSimJobSource::GetUnregisteredJobCount() const
{
  return gbsUnregisteredJobs.count();
}

std::deque<unsigned int> MCSimJobSource::GetUnregisteredJobs() const
{
  std::deque<unsigned int> seqResult;
  for (unsigned int ui=guiUnregisteredJobLowerBound, uiCurr=0, uiEnd=gbsUnregisteredJobs.size(); uiCurr < uiEnd; ++ui,++uiCurr)
    if (gbsUnregisteredJobs.test(uiCurr))
      seqResult.push_back(ui);
  return seqResult;
}

//From how many jobs were exceptions caught?
unsigned int MCSimJobSource::GetExceptionCount() const
{
  return gvExceptions.size();
}

MCSimJobSource::exception_sequence_type MCSimJobSource::GetExceptions() const
{
  return gvExceptions;
}

bool MCSimJobSource::is_exhausted() const
{
  return
    CancelConditionExists()
   || ExceptionConditionExists()
   || AutoAbortConditionExists()
   || (guiNextJobParam > guiJobCount);
}

//Remove the first N bits from operand.
void MCSimJobSource::DynamicBitsetPopFrontN(boost::dynamic_bitset<> & operand, unsigned long N)
{
  operand >>= N;//shift all bits down
  operand.resize(N > operand.size() ? 0 : operand.size()-N);//pop the back bits off
}

void MCSimJobSource::register_result(job_id_type const & job_id, param_type const & param, result_type const & result)
{
  try
  {
    //the job_id must be one of the unfinished jobs:
    assert(job_id >= guiUnregisteredJobLowerBound);
    assert(job_id < guiNextJobParam);

    (this->*gfnRegisterResult)(job_id, param, result);

    //after everything else is done, update gbsUnfinishedJobs:
    if (job_id != guiUnregisteredJobLowerBound)
      gbsUnregisteredJobs.reset(job_id - guiUnregisteredJobLowerBound);
    else
    {//remove leading bit plus the block of zero bits that follows:
      unsigned long ulN=1;
      for (; (ulN < gbsUnregisteredJobs.size()) && !gbsUnregisteredJobs.test(ulN); ++ulN);//count the number of zero bits that follow (until the first 1 bit)
      DynamicBitsetPopFrontN(gbsUnregisteredJobs, ulN);
      guiUnregisteredJobLowerBound += ulN;
    }
  }
  catch (prg_exception & e)
  {
    e.addTrace("register_result()", "MCSimJobSource");
    throw;
  }
}

//register a result when analysis has been canceled.  This will be called for
//all subsequent job registrations (that were already running when cancel
//got triggered), which are ignored.
void MCSimJobSource::RegisterResult_CancelConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult)
{
//  try
//  {
//  }
//  catch (prg_exception & e)
//  {
//    e.addTrace("RegisterResult_CancelConditionExists()", "MCSimJobSource");
//    throw;
//  }
}

//register a result when AutoAbort (early termination) isn't active
void MCSimJobSource::RegisterResult_AutoAbort(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    //check exception condition first.  Want to report an exception even if
    //cancel is requested.
    if (!rResult.bUnExceptional)
    {
      //populate stored exceptions:
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
      gfnRegisterResult = &MCSimJobSource::RegisterResult_ExceptionConditionExists;
      return;
    }
    else if (CancelRequested())
    {
      gfnRegisterResult = &MCSimJobSource::RegisterResult_CancelConditionExists;
      ReleaseAutoAbortCheckResources();
      return;
    }

    // Add this job to the cache of results.
    gmapOverflowResults.insert(std::make_pair(rJobID,std::make_pair(rParam,rResult)));
    // process cached completed jobs sequencely
    while (!gmapOverflowResults.empty() && guiNextProcessingJobId == gmapOverflowResults.begin()->first) {
         //gAutoAbortResultsRegistered.set(gmapOverflowResults.begin()->first - guPreviousAutoAbortCheckPoint - 1);
         RegisterResult_NoAutoAbort(gmapOverflowResults.begin()->first, gmapOverflowResults.begin()->second.first, gmapOverflowResults.begin()->second.second);
         gmapOverflowResults.erase(gmapOverflowResults.begin());
         ++guiNextProcessingJobId;
         if (grRunner.getSimulationVariables().get_llr_counters().front().second >= 0/* TODO grRunner.gParameters.GetExecuteEarlyTermThreshold()*/) {
            //auto-abort is triggered
            gfnRegisterResult = &MCSimJobSource::RegisterResult_AutoAbortConditionExists;
            ReleaseAutoAbortCheckResources();
            return;
         }
    }
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_AutoAbort()", "MCSimJobSource");
    throw;
  }
}

//register a result when AutoAbort has been triggered.  This will be called for
//all subsequent job registrations (the ones that were already running when auto-abort
//got triggered).  Their results are ignored.
void MCSimJobSource::RegisterResult_AutoAbortConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult)
{
//  try
//  {
//  }
//  catch (prg_exception & e)
//  {
//    e.AddCallpath("RegisterResult_AutoAbortConditionExists()", "MCSimJobSource");
//    throw;
//  }
}

//register a result when a previously registered result indicated an exception.
void MCSimJobSource::RegisterResult_ExceptionConditionExists(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    if (!rResult.bUnExceptional)
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_ExceptionConditionExists()", "MCSimJobSource");
    throw;
  }
}

//register a result when no extended conditions (AutoAbort[early termination],
//thrown exceptions, cancelation) are active.
void MCSimJobSource::RegisterResult_NoAutoAbort(job_id_type const & rJobID, param_type const & rParam, result_type const & rResult)
{
  try
  {
    //check exception condition first.  Want to report an exception even if
    //cancel is requested.
    if (!rResult.bUnExceptional)
    {
      //populate stored exceptions:
      gvExceptions.push_back(std::make_pair(rJobID, std::make_pair(rParam,rResult)));
      gfnRegisterResult = &MCSimJobSource::RegisterResult_ExceptionConditionExists;
      return;
    }
    else if (CancelRequested())
    {
      gfnRegisterResult = &MCSimJobSource::RegisterResult_CancelConditionExists;
      return;
    }

    //update ratios, significance, etc.
    double result = grLoglikelihood->LogLikelihoodRatio(rResult.dSuccessfulResult.first);        
    for (unsigned int k=0; k < grRunner.getCuts().size(); k++)
        if (rResult.dSuccessfulResult.first > grRunner.getCuts().at(k)->getLogLikelihood()) grRunner.getCuts().at(k)->incrementRank();

    if (_ratio_writer.get()) _ratio_writer->write(result);
    if (!_isPowerStep) grRunner.updateCriticalValuesList(result);
    grRunner.getSimulationVariables().add_llr(result);
    grRunner.getSimulationVariables().increment_sim_count();

    ++guiJobsReported;

    //if appropriate, estimate time required to complete all jobs and report it.
    unsigned int uiJobsProcessedCount = (gbsUnregisteredJobs.size()-gbsUnregisteredJobs.count()) + guiUnregisteredJobLowerBound; //this one hasn't been reset in gbsUnregisteredJobs yet.
    grPrintDirection.Printf(gszReplicationFormatString, BasePrint::P_STDOUT, guiJobsReported, guiJobCount, result);
    if (uiJobsProcessedCount==10) {
      ::ReportTimeEstimate(gConstructionTime, guiJobCount, rParam, &grPrintDirection);
      TreeScan::Timestamp tsReleaseTime; tsReleaseTime.Now(); tsReleaseTime.AddSeconds(3);//queue lines until 3 seconds from now
      grPrintDirection.SetThresholdPolicy(TimedReleaseThresholdPolicy(tsReleaseTime));
    }
  }
  catch (prg_exception & e)
  {
    e.addTrace("RegisterResult_NoAutoAbort()", "MCSimJobSource");
    throw;
  }
}


//When we're through checking for auto-abort, we want to release any resources
//used.  (This is mostly for cancel and exception conditions that occur while
//auto-abort checking is active.)
void MCSimJobSource::ReleaseAutoAbortCheckResources()
{
  gmapOverflowResults.clear();
}
