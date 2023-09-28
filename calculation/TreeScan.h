//*****************************************************************************
#ifndef __TREESCAN_H
#define __TREESCAN_H
//*****************************************************************************
/** Current version information. */
#define VERSION_ID      "10"  /** incremental version identifier - this value must
                                  be incremented for each released version in order
                                  for update feature to function correctly */
#define VERSION_MAJOR   "2"
#define VERSION_MINOR   "2"
#define VERSION_RELEASE "0"
#define VERSION_PHASE   "Beta 1" /** testing phase name - leave blank for release */
#define VERSION_DATE    "Not Released"

#if defined(_MSC_VER)
#include <Windows.h>
#endif

#ifndef _WINDOWS_
  #define stricmp strcasecmp
  #define strnicmp strncasecmp
#endif

#include <cstdlib>
#include <stdio.h>

#ifdef _WINDOWS_
  #include <io.h>
#else
  #include <unistd.h>
#endif
#ifdef _MSC_VER
  /** default string buffer size for vsnprintf call */
  #define MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE 1000
#endif
#define DATE_TIME_INLINE
#include "boost/date_time/posix_time/ptime.hpp"
#undef DATE_TIME_INLINE

#include <cmath>
#include <string>
#include <vector>
#include <ctime>
#include "boost/tuple/tuple.hpp"
#include <boost/any.hpp>

  /** average days in year */
#define AVERAGE_DAYS_IN_YEAR 365.25
/** comparision tolerance for double precision numbers */
#define DBL_CMP_TOLERANCE 1.0E-9
/** determines equality between two numbers given some tolerance */
#define macro_equal(x,y,tolerance) (std::fabs(x - y) < tolerance)
/** determines whether number x is less than number y given some tolerance */
#define macro_less_than(x,y,tolerance) (!macro_equal(x,y,tolerance) && x < y)
/** determines whether number x is less than number y given some tolerance */
#define macro_less_than_or_equal(x,y,tolerance) (macro_less_than(x,y,tolerance) || macro_equal(x,y,tolerance))
/* DateSource types */
enum SourceType {CSV=0, EXCEL}; // TODO -- add EXCEL
/* data source fields map container typedef */
typedef std::vector<boost::any> FieldMapContainer_t;
typedef std::pair<double, double> RecurrenceInterval_t;
/* require more than 9 replications to report p-values */
#define MIN_REPLICA_RPT_PVALUE 9
/* minimmum cut log likelihood of interest */
#define MIN_CUT_LLR 0.001

/** va_copy not defined on all compilers */
#if defined (_MSC_VER) || ( defined(__GNUC__) && (__GNUC__ < 3) )
  #define macro_va_copy(dst,src) dst = src
#else
  #define macro_va_copy(dst,src) va_copy(dst,src);
#endif

namespace TreeScan {
}

#include "BasePrint.h"
//*****************************************************************************
#endif
