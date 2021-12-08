//******************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//******************************************************************************
#include "TreeScan.h"
#include "PrjException.h"
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include<boost/tokenizer.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

using namespace TreeScan;

namespace TreeScan {
    template <class T>
    T& cumulative_backward(T& t) {
        typename T::reverse_iterator from(t.end()), until(t.begin());
        for (; from != until; ++from) {
            typename T::reverse_iterator prior(from + 1);
            if (prior != until) {
                *prior += *from;
            }
        }
        return t;
    }

    template <class T>
    T& cumulative_forward(T& t) {
        typename T::iterator from(t.begin()), until(t.end());
        for (; from != until;) {
            typename T::iterator next(from + 1);
            if (next != until) {
                *next += *from;
            }
            ++from;
        }
        return t;
    }
}


double                          getNumCombinations(size_t total, size_t choose);
unsigned int                    GetNumSystemProcessors();
void                            ReportTimeEstimate(boost::posix_time::ptime StartTime, int nRepetitions, int nRepsCompleted, BasePrint *pPrintDirection);
boost::posix_time::ptime        GetCurrentTime_HighResolution();
bool                            validateFileAccess(const std::string& filename, bool bWriteEnable=false, bool useTempFile=false);
std::string                   & trimString(std::string &source, const char * t=" ");
std::string                   & lowerString(std::string &source); 
std::string                   & printString(std::string& s, const char * format, ...);
unsigned int                    getFormatPrecision(double value, unsigned int iSignificant=2);
std::string                   & getValueAsString(double value, std::string& s, unsigned int iSignificant=2);
std::string                   & getRoundAsString(double value, std::string& s, unsigned int precision, bool localize=false);
std::string                   & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath);
std::string                   & GetUserTemporaryDirectory(std::string& s);
std::string                   & GetTemporaryFilename(std::string& s);
bool                            getlinePortable(std::ifstream& readstream, std::string& line);
std::string                   & htmlencode(std::string& data);
template <typename T>           bool string_to_numeric_type(const char * s, T& t, bool test_finite=true) {
                                    try {
                                        t = boost::lexical_cast<T>(s);
                                        return test_finite ? boost::math::isfinite<T>(t) : true;
                                    } catch (boost::bad_lexical_cast&) {
                                        return false;
                                    } 
                                    return true;
                                }
template <typename T>           bool string_to_type(const char * s, T& t) {
                                    try {
                                        t = boost::lexical_cast<T>(s);
                                    } catch (boost::bad_lexical_cast&) {
                                        return false;
                                    } 
                                    return true;
                                }
template <typename T>           bool csv_string_to_typelist(const char * s, std::vector<T>& list) {
                                    try {
                                        list.clear();
                                        std::string value(s);
                                        boost::escaped_list_separator<char> separator('\\', ',', '\"');
                                        boost::tokenizer<boost::escaped_list_separator<char> > identifiers(value, separator);
                                        for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
                                            std::string token(*itr);
                                            T t = boost::lexical_cast<T>(trimString(token).c_str());
                                            list.push_back(t);
                                        }
                                    } catch (boost::bad_lexical_cast&) {
                                        return false;
                                    } 
                                    return true;
                                }
template <typename T>           bool typelist_csv_string(const std::vector<T>& list, std::string& s) {
                                    try {
                                        std::stringstream buffer;
                                        if (list.empty()) {
                                            s = "";
                                        } else {
                                            buffer << *(list.begin());
                                            //buffer << boost::lexical_cast<std::string>(*(list.begin()));
                                            for (typename std::vector<T>::const_iterator itr = list.begin() + 1; itr != list.end(); ++itr) {
                                                buffer << "," << *itr;
                                                //buffer << "," << boost::lexical_cast<std::string>(*itr);
                                            }
                                            s = buffer.str();
                                        }
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
const char                    * getOrdinalSuffix(unsigned int ordinal);

template<class _OutTy> inline
void writePadRight(const std::string& text, _OutTy& destintation, size_t width, const char& padValue) {
    if (text.size() > width)
        throw prg_error("String %s is longer than specified width (%u).","writePadRight()", text.c_str(), width);
    destintation << text.c_str();
    std::fill_n(std::ostream_iterator<char>(destintation), width - text.size(), ' ');
}

using boost::uuids::detail::md5;
std::string toString(const md5::digest_type &digest);
//******************************************************************************
#endif
