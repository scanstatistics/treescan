//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "UtilityFunctions.h"
#include "FileName.h"
#include <iostream>
#include <fstream>
#include <boost/math/special_functions/factorials.hpp>
using namespace boost::math;
#include <boost/filesystem.hpp>
#include <locale>
#include <boost/regex.hpp>

/** Returns number of combinations, given 'total' to choose from, choosing 'choose'; where order does not matter and repetition not allowed. */
double getNumCombinations(unsigned int total, unsigned int choose) {
    if (total < choose) return 0.0;
    return (total == choose) ? 1.0 : factorial<double>(total)/(factorial<double>(choose) * factorial<double>(total - choose));
}

/** What is the current time? (UTC | Coordinated Universal Time) */
#ifdef _WINDOWS_
boost::posix_time::ptime GetCurrentTime_HighResolution()
{
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  SYSTEMTIME stm;
  GetSystemTime(&stm);
  time_duration::fractional_seconds_type frct_secs = static_cast<time_duration::fractional_seconds_type>(stm.wMilliseconds * pow(static_cast<double>(10), time_duration::num_fractional_digits()-3));
  return ptime(date(stm.wYear,stm.wMonth,stm.wDay), time_duration(stm.wHour,stm.wMinute,stm.wSecond,frct_secs));
}
#else
#include <sys/time.h>
boost::posix_time::ptime GetCurrentTime_HighResolution()
{
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  struct timeval   tmStruct;
  gettimeofday(&tmStruct, 0);
  time_duration::fractional_seconds_type frct_secs = static_cast<time_duration::fractional_seconds_type>(tmStruct.tv_usec * std::pow(static_cast<double>(10), time_duration::num_fractional_digits()-6));
  return ptime(date(1970,1,1), time_duration(0,0,tmStruct.tv_sec,frct_secs));
}
#endif

/** Returns number of processors in the system. */
unsigned int GetNumSystemProcessors() {
  unsigned int iNumProcessors = 1;

#ifdef _WINDOWS_
   SYSTEM_INFO siSysInfo;
   GetSystemInfo(&siSysInfo);
   iNumProcessors = siSysInfo.dwNumberOfProcessors;
#else
  iNumProcessors = sysconf(_SC_NPROCESSORS_ONLN);
#endif

  //return at least one, system calls might have failed
  return (iNumProcessors > 0 ? iNumProcessors : 1);
}

/** Calculates an estimate for the time remaining to complete X repetition given Y completed. */
void ReportTimeEstimate(boost::posix_time::ptime StartTime, int nRepetitions, int nRepsCompleted, BasePrint *pPrintDirection) {
  boost::posix_time::ptime StopTime(GetCurrentTime_HighResolution());
  double dSecondsElapsed;

  // nothing to report if number of repetitions less than 2 or none have been completed
  if (nRepetitions <= 1 || nRepsCompleted <= 0) return;
  // nothing to report if start time greater than stop time -- error?
  if (StartTime > StopTime) return;

  boost::posix_time::time_duration ElapsedTime = StopTime - StartTime;
  dSecondsElapsed = ElapsedTime.fractional_seconds() / std::pow(static_cast<double>(10), ElapsedTime.num_fractional_digits());
  dSecondsElapsed += ElapsedTime.seconds();
  dSecondsElapsed += ElapsedTime.minutes() * 60;
  dSecondsElapsed += ElapsedTime.hours() * 60 * 60;
  double dEstimatedSecondsRemaining = dSecondsElapsed/nRepsCompleted * (nRepetitions - nRepsCompleted);

  // print an estimation only if estimated time will be 30 seconds or more
  if (dEstimatedSecondsRemaining >= 30) {
    if (dEstimatedSecondsRemaining < 60.0)
        pPrintDirection->Printf(".... this will take approximately %.0lf seconds.\n", BasePrint::P_STDOUT, dEstimatedSecondsRemaining);
    else if (dEstimatedSecondsRemaining < 3600.0) {
      double dMinutes = std::ceil(dEstimatedSecondsRemaining/60.0);
      pPrintDirection->Printf(".... this will take approximately %.0lf minute%s.\n",
                              BasePrint::P_STDOUT, dMinutes, (dMinutes == 1.0 ? "" : "s"));
    }
    else {
      double dHours = std::floor(dEstimatedSecondsRemaining/3600.0);
      double dMinutes = std::ceil((dEstimatedSecondsRemaining - dHours * 3600.0)/60.0);
      pPrintDirection->Printf(".... this will take approximately %.0lf hour%s %.0lf minute%s.\n",
                              BasePrint::P_STDOUT, dHours, (dHours == 1.0 ? "" : "s"), dMinutes, (dMinutes == 1.0 ? "" : "s"));
    }
  }
}

/** Returns indication of whether file exists and is readable/writable. */
bool validateFileAccess(const std::string& filename, bool bWriteEnable, bool useTempFile) {
    FILE * fp=0;
    bool bReturn=true;

#ifdef __APPLE__
    // Hack for Mac application - where examples are in application bundle. We don't want to write result files to the bundle.
    if (bWriteEnable && filename.find("/Contents/app/sample_data/") != std::string::npos)
        return false;
#endif

    if (useTempFile) {
        std::string buffer;
        FileName test(filename.c_str());
        test.setExtension("write-test");
        bReturn = ((fp = fopen(test.getFullPath(buffer).c_str(), bWriteEnable ? "w" : "r")) != NULL);
        if (fp) fclose(fp);
        remove(test.getFullPath(buffer).c_str());
    } else {
        bReturn = ((fp = fopen(filename.c_str(), bWriteEnable ? "w" : "r")) != NULL);
        if (fp) fclose(fp);
    }
    return bReturn;
}

/** Trims leading and trailing 't' strings from source, inplace. */
std::string & trimString(std::string &source, const char * t) {
  source.erase(0, source.find_first_not_of(t));
  source.erase(source.find_last_not_of(t)+1);
  return source;
}

/** Converts string to lower case. */
std::string& lowerString(std::string &source) {
  std::transform(source.begin(), source.end(), source.begin(), (int(*)(int)) tolower);
  return source;
}

/** Assigns formatted strng to destination */
std::string& printString(std::string& destination, const char * format, ...) {
  try {
#ifdef _MSC_VER
    std::vector<char> temp(MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE);
    va_list varArgs;
    va_start (varArgs, format);
    vsnprintf(&temp[0], temp.size() - 1, format, varArgs);
    va_end(varArgs);
#else
    std::vector<char> temp(1);
    va_list varArgs_static;
    va_start (varArgs_static, format);

    std::va_list arglist_test; 
    macro_va_copy(arglist_test, varArgs_static);
    size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, arglist_test);
    temp.resize(iStringLength + 1);

    std::va_list arglist;
    macro_va_copy(arglist, varArgs_static);
    vsnprintf(&temp[0], iStringLength + 1, format, arglist);
    va_end(varArgs_static);
#endif
    destination = &temp[0];
  }
  catch (...) {}
  return destination;
}

/** Get printf precision format specifer given passed value. 
    Returns a minimum of 'iSignificant' significant decimal digits. */
unsigned int getFormatPrecision(double value, unsigned int iSignificant) {
    unsigned int iPrecision = iSignificant;

    if (value == 0.0) return 0;

    if (fabs(value) < 1.0) {
        // If value less than 1.0, we can use log10 to determine what is the 10 power.
        // ex. value = 0.0023:
        //   log10(0.0023) = log10(10^-3) + log10(2.3)
        //   log10(0.0023) = -3 + 0.36172783601759287886777711225119
        //   log10(0.0023) = -2.6382721639824071211322228877488
        //   take ceiling since we really are not interested in log10(2.3) portion
        iPrecision += static_cast<unsigned int>(ceil(fabs(log10(fabs(value))))) - 1;
    }
    return iPrecision;
}

/** Returns value as string with number of 'iSignificant' significant decimals.
    The 'g' format specifier might have sufficed but Martin wanted this format. */
std::string& getValueAsString(double value, std::string& s, unsigned int iSignificant) {
    if (std::isinf(value))
        s = "infinity";
    else if (boost::math::isnan(value))
        s = "undefined";
    else {
        unsigned int iPrecision = getFormatPrecision(value, iSignificant);
        std::string format;
        printString(format, "%%.%dlf", iPrecision);
        printString(s, format.c_str(), value);
    }
    return s;
}

/** Returns double as string with specified decimal precision. */
std::string & getRoundAsString(double value, std::string& s, unsigned int precision, bool localize) {
    std::stringstream buffer;
    if (localize) {
        std::locale the_locale("");
        buffer.imbue(the_locale);
    }
    buffer << std::setprecision(precision) << std::setiosflags(std::ios_base::fixed) << value;
    s = buffer.str();
    return s;
}

#ifdef _WINDOWS_
#include "shlobj.h"

std::string & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath) {
  TCHAR szPath[MAX_PATH];

  if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, szPath))) {
    //if (!DirectoryExists(szPath)) {
    //  if (!CreateDir(szPath))
    //    throw prg_error("Unable to create My Documents.", "GetMyDocumentsDirectory()");
    //}
    s = szPath;
  } else {
    s = defaultPath; 
  }  
  return s;
}
#else
#include <sys/types.h>
#include <pwd.h>
   
std::string & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath) {
  uid_t           uid;
  struct passwd * pwd;
  uid = getuid();
  if (!(pwd = getpwuid(uid))) {
    s = defaultPath; 
  } else {
    s = pwd->pw_dir;
  }
  endpwent();
  return s;
}    
#endif

std::string & GetUserTemporaryDirectory(std::string& s) {
    s = boost::filesystem::temp_directory_path().string();
    return s;
}

/** Obtains a temporary filename either from user temp directory or specified path. */
std::string & GetTemporaryFilename(std::string& s, const char * atLocation) {
    if (atLocation) {
        FileName filename(atLocation);
        filename.getLocation(s);
    } else {
        GetUserTemporaryDirectory(s);
        s += boost::filesystem::path::preferred_separator;
    }
    s += boost::filesystem::unique_path().string();
    s += ".tmp";
    return s;
}

/** Attempt to readline for stream giving consideration to DOS, UNIX (or Mac Os X) and Mac 9 (or earlier) line ends. 
    Returns whether data was read or end of file encountered. */
bool getlinePortable(std::ifstream& readstream, std::string& line) {
  std::ifstream::char_type nextChar;
  std::stringstream readStream;

  while (readstream.good()) {
	  if (!readstream.get(nextChar) || !readstream.good())
          break; // Reading next char brings us to end of file or error state
      
      if (nextChar == readstream.widen('\r')) {
          // could be either DOS or Mac 9 end of line -- peek at next char
          nextChar = readstream.peek();
          if (nextChar == readstream.widen('\n')) {
		      // DOS end of line characters -- read it.
              readstream.get(nextChar);
          }
          break;
      }
      if (nextChar == readstream.widen('\n')) {
	      // UNIX or Mac OS X end of line character.
          break;
      }
      readStream << nextChar;
  }
  line = readStream.str();
  return line.size() > 0 ? true : readstream.good();
}

std::string & getDerivedFilename(const std::string& source, const std::string& suffix, const std::string& extension, std::string& destination) {
  FileName     _fileName(source.c_str());
  std::string buffer(_fileName.getFileName());

  buffer += suffix;
  _fileName.setFileName(buffer.c_str());
  if (!extension.empty()) _fileName.setExtension(extension.c_str());

  // should source name equal destination, update filename again
  if (source == _fileName.getFullPath(destination)) {
      _fileName.setFileName(printString(buffer, "%s%s_2", _fileName.getFileName().c_str(), suffix.c_str()).c_str());
      _fileName.getFullPath(destination);
  }

  return destination;
}

const char * getOrdinalSuffix(unsigned int ordinal) {
    switch (ordinal % 100) {
        case 11 : case 12 : case 13 : return "th"; // special case
        default : {
            switch (ordinal % 10) {
                case 1 : return "st";
                case 2 : return "nd";
                case 3 : return "rd";
                default : return "th";
            }
        }
    }
    return "th";
}

/** HTML encodes text - replacing symbols/punctuation with html codes. */
std::string& htmlencode(std::string& text) {
    std::stringstream ss;
    for (size_t pos = 0; pos != text.size(); ++pos) {
        if (std::ispunct(static_cast<unsigned char>(text[pos])) || text[pos] == ' ')
            ss << "&#" << int(text[pos]) << ';';
        else
            ss << text[pos];
    }
    text = ss.str();
    return text;
}

/** Returns string representation of MD5 digest. */
std::string toString(const md5::digest_type &digest) {
    const char * charDigest = reinterpret_cast<const char *>(&digest);
    std::string result;
    boost::algorithm::hex(charDigest, charDigest + sizeof(md5::digest_type), std::back_inserter(result));
    return result;
}

/** Replaces 'replaceStub' text in passed stringstream 'templateText' with text of 'replaceWith'. */
std::stringstream & templateReplace(std::stringstream& templateText, const std::string& replaceStub, const std::string& replaceWith) {
    boost::regex to_be_replaced(replaceStub);
    std::string changed(boost::regex_replace(templateText.str(), to_be_replaced, replaceWith));
    templateText.str(std::string());
    templateText << changed;
    return templateText;
}

/** Returns text that is margin aligned and wraps text at those margins. */
std::string& getWrappedText(const std::string& text, unsigned int marginLeft, unsigned int marginRight, const std::string& newline, std::string& wrapped) {
    std::stringstream wrappedText;
    auto leftPadding = [&wrappedText, marginLeft]() {
        unsigned int iPad = 0;
        while (iPad++ < marginLeft) wrappedText << ' ';
    };
    leftPadding();
    unsigned int line_length = 0, max_width = marginRight - marginLeft;
    std::stringstream line;
    boost::escaped_list_separator<char> separator('\\', ' ', '\"');
    boost::tokenizer<boost::escaped_list_separator<char> > tokens(text, separator);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr = tokens.begin(); itr != tokens.end(); ++itr) {
        if (line_length + itr->size() > max_width) {
            wrappedText << line.str() << newline;
            if (itr != tokens.end()) leftPadding();
            line.str(""); line << (*itr) << " "; line_length = itr->size() + 1;
        } else {
            line << (*itr) << " ";
            line_length += itr->size() + 1;
        }
    }
    if (line_length) wrappedText << line.str() << newline;
    wrapped = wrappedText.str();
    return wrapped;
}

/** Returns humanized string for decmical value. Derived from https://gist.github.com/cslarsen/1870641 */
std::string& humanize(double n, std::string& humanized, int decimals) {
    std::vector<std::string> fmt = { "%1.*lf %s", "%1.*lf" };
    std::vector<std::string> units = {
        "", "thousand", "million", "billion", "trillion", "quadrillion", "quintillion", "sextillion", "septillion"
    };

    /*
     * Number of digits in n is given by
     * 10^x = n ==> x = log(n)/log(10) = log_10(n).
     *
     * So 1000 would be 1 + floor(log_10(10^3)) = 4 digits.
     */
    int digits = n == 0 ? 0 : 1 + std::floor(log10l(fabs(n)));

    // determine base 10 exponential
    int exp = digits <= 4 ? 0 : 3 * ((digits - 1) / 3);

    // normalized number
    double m = n / powl(10, exp);

    // no decimals? then don't print any
    if (m - static_cast<long>(m) == 0)
        decimals = 0;

    // don't print unit for exp<3
    return printString(humanized, fmt[exp < 3].c_str(), decimals, m, units[exp / 3].c_str());
}

