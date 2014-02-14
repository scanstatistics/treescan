//******************************************************************************
#ifndef __MCSimJobSource_H
#define __MCSimJobSource_H
//******************************************************************************

#include <utility>
#include <deque>
#include "boost/dynamic_bitset.hpp"
#include "ScanRunner.h"
#include "DataFileWriter.h"
#include "PrintQueue.h"
#include "PrjException.h"

class MCSimJobSource
{
public://types/enums
  typedef std::pair<double,int> successful_result_type;

  struct job_result {
    enum                        exception_type {unknown=0, std, prg, memory};

    bool                        bUnExceptional;
    exception_type              eException_type;
    prg_exception               Exception;
    successful_result_type      dSuccessfulResult;
  };

  typedef unsigned int                        param_type;
  typedef job_result                          result_type;
  typedef unsigned long                       job_id_type;
  typedef std::pair<job_id_type, param_type>  job_info_type;

private://types/enums
  typedef void (MCSimJobSource::*result_registration_fn_type)(job_id_type const &, param_type const &, result_type const &);
  typedef std::map<job_id_type,std::pair<param_type,result_type> >   overflow_jobs_container_type;
  typedef std::pair<job_id_type,std::pair<param_type,result_type> >  exception_type;
  typedef std::deque<exception_type>                                 exception_sequence_type;

private://data members
  const unsigned int                        guiJobCount;
  unsigned int                              guiNextJobParam;
  unsigned int                              guiUnregisteredJobLowerBound;//gbsUnregisteredJobs[0] refers to the job with this index.  gbsUnregisteredJobs[n] refers to the job at index 'guiUnregisteredJobLowerBound'+n .
  boost::dynamic_bitset<>                   gbsUnregisteredJobs;
  exception_sequence_type                   gvExceptions;
  job_id_type                               guiNextProcessingJobId;
  job_id_type                               guiJobsReported;
  result_registration_fn_type               gfnRegisterResult;
  overflow_jobs_container_type              gmapOverflowResults;//contains unprocesed results for jobs, sorted by jobid.
  const boost::posix_time::ptime            gConstructionTime;
  PrintQueue                              & grPrintDirection;
  const char                              * gszReplicationFormatString;
  std::auto_ptr<LoglikelihoodRatioWriter>   _ratio_writer;
  ScanRunner                              & grRunner;
  ScanRunner::Loglikelihood_t               grLoglikelihood;

private://functions
  void                      RegisterResult_NoAutoAbort(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void                      RegisterResult_AutoAbort(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void                      RegisterResult_AutoAbortConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void                      RegisterResult_ExceptionConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);
  void                      RegisterResult_CancelConditionExists(job_id_type const & rJobId, param_type const & rParam, result_type const & rResult);

  bool                      CancelRequested() const;
  void                      ReleaseAutoAbortCheckResources();
  static void               DynamicBitsetPopFrontN(boost::dynamic_bitset<> & operand, unsigned long N);

public:
  MCSimJobSource(boost::posix_time::ptime CurrentTime, PrintQueue & rPrintDirection, const char * szReplicationFormatString, ScanRunner & rRunner);

  bool                      is_exhausted() const;
  void                      acquire(job_id_type & dst_job_id, param_type & dst_param);
  void                      register_result(job_id_type const & job_id, param_type const & param, result_type const & result);

  unsigned int              GetSuccessfullyCompletedJobCount() const;
  unsigned int              GetUnregisteredJobCount() const;
  std::deque<unsigned int>  GetUnregisteredJobs() const;
  bool                      AutoAbortConditionExists() const { return gfnRegisterResult == &MCSimJobSource::RegisterResult_AutoAbortConditionExists; }
  bool                      CancelConditionExists() const { return gfnRegisterResult == &MCSimJobSource::RegisterResult_CancelConditionExists; }
  bool                      ExceptionConditionExists() const { return gfnRegisterResult == &MCSimJobSource::RegisterResult_ExceptionConditionExists; }
  unsigned int              GetExceptionCount() const;
  exception_sequence_type   GetExceptions() const;
  void                      Assert_NoExceptionsCaught() const;
};
//******************************************************************************
#endif

