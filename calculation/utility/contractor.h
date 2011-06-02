//---------------------------------------------------------------------------

#ifndef contractorH
#define contractorH
//---------------------------------------------------------------------------
#include <vector>
#include <algorithm>
#include "boost/thread/thread.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/dynamic_bitset.hpp"
#include "PrjException.h"
//---------------------------------------------------------------------------

namespace treescan {
  struct unhandled_exception {
    enum                        exception_type {unknown=0, std, prg, memory};

    bool                        bUnExceptional;
    exception_type              eException_type;
    prg_exception               Exception;
  };
}

template <typename JobSource>
class contractor
{
public:
  typedef typename JobSource::param_type job_param_type;
  typedef typename JobSource::result_type job_result_type;

private:
  typedef boost::recursive_mutex access_mutex_t;
  typedef std::pair<typename JobSource::job_id_type, typename JobSource::param_type> job_info_type;
  typedef std::vector<std::pair<void const *, typename JobSource::job_id_type> > current_subcontracts_type;

  current_subcontracts_type m_current_subcontracts;//first: subcontractors who have acquired a job and haven't yet registered results for it; second: job id
  JobSource & m_jobs;
  mutable access_mutex_t m_access_mutex;

public:
    treescan::unhandled_exception m_unhandled_exception;

public:
  contractor(JobSource & jobs)
   : m_jobs(jobs)
  {// setup();
   m_unhandled_exception.bUnExceptional = true;
  }

  void set_unhandled_exception(const prg_exception &e, treescan::unhandled_exception::exception_type except_type) {
      access_mutex_t::scoped_lock lcl_lock(m_access_mutex);
      m_unhandled_exception.eException_type = except_type;
      m_unhandled_exception.Exception = e;
      m_unhandled_exception.bUnExceptional = false;
  }

  void throw_unhandled_exception() const {
      access_mutex_t::scoped_lock lcl_lock(m_access_mutex);
      if (!m_unhandled_exception.bUnExceptional) {
          switch (m_unhandled_exception.eException_type) {
              case treescan::unhandled_exception::memory  : throw memory_exception(m_unhandled_exception.Exception.what());
              case treescan::unhandled_exception::std     : 
              case treescan::unhandled_exception::prg     : 
              case treescan::unhandled_exception::unknown : 
              default                                    : throw m_unhandled_exception.Exception;
          }
      }
  }

  bool is_finished() const
  { access_mutex_t::scoped_lock lcl_lock(m_access_mutex);
    return !m_unhandled_exception.bUnExceptional || m_jobs.is_exhausted();
  }


  template <typename SubcontractorType>
  bool job_acquired(SubcontractorType const & subcontractor, job_param_type & job_param)
  { access_mutex_t::scoped_lock lcl_lock(m_access_mutex);

    {//check to make sure subcontractor doesn't have an uncompleted job:
      typename current_subcontracts_type::const_iterator itr = m_current_subcontracts.begin();
      typename current_subcontracts_type::const_iterator itrend = m_current_subcontracts.end();
      for (; (itr != itrend) && (itr->first != &subcontractor); ++itr) {}
      if (itr != m_current_subcontracts.end())
        throw std::runtime_error("Subcontractor has not registered results of previously acquired job.");
    }

    if (is_finished())
      return false;

    typename JobSource::job_id_type new_job_id;
    job_param_type new_job_param;
    m_jobs.acquire(new_job_id, new_job_param);
    m_current_subcontracts.push_back(std::make_pair(&subcontractor, new_job_id));
    job_param = new_job_param;
    return true;
  }

  template <typename SubcontractorType>
  void register_result(SubcontractorType const & subcontractor, job_param_type const & job_param, job_result_type const & job_result)
  { access_mutex_t::scoped_lock lcl_lock(m_access_mutex);

    typename current_subcontracts_type::iterator itr = m_current_subcontracts.begin();
    {//check to make sure subcontractor has an uncompleted job:
      typename current_subcontracts_type::const_iterator itrend = m_current_subcontracts.end();
      for (; (itr != itrend) && (itr->first != &subcontractor); ++itr) {}
      if (itr == m_current_subcontracts.end())
        throw std::runtime_error("Subcontractor has not acquired a job.");
    }

    typename JobSource::job_id_type const & job_id(itr->second);
    m_jobs.register_result(job_id, job_param, job_result);
    m_current_subcontracts.erase(itr);
  }
};



template <typename ContractorType, typename Function>
class subcontractor
{
  ContractorType & m_contractor;
  Function m_function;

public:
  subcontractor(ContractorType & contractor, Function function)
   : m_contractor(contractor)
   , m_function(function)
  {}

  void operator() ()
  {
    try {
        while (!m_contractor.is_finished())
        {
            typename ContractorType::job_param_type param;
            if (m_contractor.job_acquired(*this, param))
            {
            typename ContractorType::job_result_type result(m_function(param));
            m_contractor.register_result(*this, param, result);
            }
        }
    } catch (memory_exception & e) {
      prg_exception x(e.what(), "subcontractor::operator()");
      m_contractor.set_unhandled_exception(x, treescan::unhandled_exception::memory);
    } catch (prg_exception & e) {
      m_contractor.set_unhandled_exception(e, treescan::unhandled_exception::prg);
    } catch (std::exception & e) {
      prg_exception x(e.what(), "subcontractor::operator()");
      m_contractor.set_unhandled_exception(x, treescan::unhandled_exception::std);
    } catch (...) {
      prg_exception x("(...) -- unknown error", "subcontractor::operator()");
      m_contractor.set_unhandled_exception(x, treescan::unhandled_exception::unknown);
    }
  }

};


template <typename ParamType, typename ResultType>
class null_jobsource_continuation_policy
{
  typedef ParamType param_type;
  typedef ResultType result_type;

public:
  bool ShouldContinue() const { return true; }
  void Report_ResultsRegistered(unsigned job_idx, param_type const & param, result_type const & result) {}
};

#endif
