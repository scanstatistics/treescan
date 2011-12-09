//******************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//******************************************************************************
#include "TreeScan.h"
#include "boost/lexical_cast.hpp"

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
std::string                   & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath);
bool                            getlinePortable(std::ifstream& readstream, std::string& line);
template <typename T>           bool string_to_type(const char * s, T& t) {
                                    try {
                                        t = boost::lexical_cast<T>(s);
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
//******************************************************************************
#endif
