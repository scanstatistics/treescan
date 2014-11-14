//******************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//******************************************************************************
#include "TreeScan.h"
#include "PrjException.h"
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

using namespace TreeScan;

namespace TreeScan {
    template <class T>
    T& cumulative(T& t) {
        typename T::reverse_iterator from(t.end()), until(t.begin());
        for (; from != until; ++from) {
            typename T::reverse_iterator prior(from + 1);
            if (prior != until) {
                *prior += *from;
            }
        }
        return t;
    }
}

double                          getNumCombinations(size_t total, size_t choose);
unsigned int                    GetNumSystemProcessors();
void                            ReportTimeEstimate(boost::posix_time::ptime StartTime, int nRepetitions, int nRepsCompleted, BasePrint *pPrintDirection);
boost::posix_time::ptime        GetCurrentTime_HighResolution();
bool                            ValidateFileAccess(const std::string& filename, bool bWriteEnable=false);
std::string                   & trimString(std::string &source, const char * t=" ");
std::string                   & lowerString(std::string &source); 
std::string                   & printString(std::string& s, const char * format, ...);
unsigned int                    getFormatPrecision(double value, unsigned int iSignificant=2);
std::string                   & getValueAsString(double value, std::string& s, unsigned int iSignificant=2);
std::string                   & getRoundAsString(double value, std::string& s, unsigned int precision);
std::string                   & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath);
std::string                   & GetUserTemporaryDirectory(std::string& s);
bool                            getlinePortable(std::ifstream& readstream, std::string& line);
template <typename T>           bool string_to_type(const char * s, T& t) {
                                    try {
                                        t = boost::lexical_cast<T>(s);
                                        return boost::math::isfinite<T>(t);
                                    } catch (boost::bad_lexical_cast&) {
                                        return false;
                                    } 
                                    return true;
                                }
template <typename T>           bool type_to_string(T& t, std::string& s) {
                                    try {
                                        s = boost::lexical_cast<std::string>(t);
                                    } catch (boost::bad_lexical_cast&) {
                                        return false;
                                    } 
                                    return true;
                                }
std::string                   & getDerivedFilename(const std::string& source, const std::string& suffix, const std::string& extension, std::string& destination);

template<class _OutTy> inline
void writePadRight(const std::string& text, _OutTy& destintation, size_t width, const char& padValue) {
    if (text.size() > width)
        throw prg_error("String %s is longer than specified width (%u).","writePadRight()", text.c_str(), width);
    destintation << text.c_str();
    std::fill_n(std::ostream_iterator<char>(destintation), width - text.size(), ' ');
}
//******************************************************************************
#endif
