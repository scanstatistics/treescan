//*****************************************************************************
#ifndef __TREESCAN_H
#define __TREESCAN_H
//*****************************************************************************
/** Current version information. */
#define VERSION_ID      "1"  /** incremental version identifier - this value must
                                  be incremented for each released version in order
                                  for update feature to function correctly */
#define VERSION_MAJOR   "0"
#define VERSION_MINOR   "1"
#define VERSION_RELEASE "0"
#define VERSION_PHASE   "Development" /** testing phase name - leave blank for release */
#define VERSION_DATE    "Not Released"

#if defined(_MSC_VER)
#include <Windows.h>
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

#include "BasePrint.h"
//*****************************************************************************
#endif
