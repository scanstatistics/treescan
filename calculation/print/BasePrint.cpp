//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "BasePrint.h"
#include "PrjException.h"

/** constructor */
BasePrint::BasePrint(bool bSuppressWarnings) : gbSuppressWarnings(bSuppressWarnings), giMaximumReadErrors(75) {
   SetImpliedInputFileType(CASEFILE);
#ifdef _MSC_VER
   gsMessage.resize(MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE);
#else
   gsMessage.resize(1);
#endif
}

/** destructor */
BasePrint::~BasePrint() {}

/** Returns indication of whether maximum number of read errors have been printed
    through this object. */
bool BasePrint::GetMaximumReadErrorsPrinted() const {
  std::map<eInputFileType, int>::const_iterator iter = gInputFileWarningsMap.find(geInputFileType);

  return (iter == gInputFileWarningsMap.end() ? false : iter->second == giMaximumReadErrors);
}

/** Directs message to appropriate output based  upon PrintType. */
void BasePrint::Print(const char * sMessage, PrintType ePrintType) {
   switch (ePrintType) {
     case P_STDOUT    : PrintStandard(sMessage); break;
     case P_NOTICE    : if (!gbSuppressWarnings)
                          PrintNotice(sMessage);
                        break;
     case P_WARNING   : if (!gbSuppressWarnings)
                          PrintWarning(sMessage);
                        break;
     case P_READERROR : PrintReadError(sMessage); break;
     case P_PARAMERROR:                   
     case P_ERROR     :
     default          : PrintError(sMessage);
   };
}

/** Creates formatted output from variable number of parameter arguments and calls class Print() method. */
void BasePrint::Printf(const char * sMessage, PrintType ePrintType, ...) {
  if (!sMessage || sMessage == &gsMessage[0]) return;

  try {
#ifdef _MSC_VER
    va_list varArgs;
    va_start (varArgs, ePrintType);
    vsnprintf(&gsMessage[0], gsMessage.size() - 1, sMessage, varArgs);
    va_end(varArgs);
#else
    va_list varArgs_static;
    va_start(varArgs_static, ePrintType);

	std::va_list arglist_test; 
	macro_va_copy(arglist_test, varArgs_static);
    size_t iStringLength = vsnprintf(&gsMessage[0], gsMessage.size(), sMessage, arglist_test);
    gsMessage.resize(iStringLength + 1);

	std::va_list arglist;
	macro_va_copy(arglist, varArgs_static);
	vsnprintf(&gsMessage[0], iStringLength + 1, sMessage, arglist);

    va_end(varArgs_static);
#endif
  }
  catch (...) {}

  Print(&gsMessage[0], ePrintType);
}

// function for printing out input file warning messages, this function will print out MAX_READ_ERRORS
// number of input file messages from each input file type, then will print a warning telling the user to check the
// input file format
// pre : none
// post : increments the counter in the global map for the message type (or starts a new counter if not found) and
//       if the number of messages for that file type is less than the maximum then it just prints as normal
void BasePrint::PrintReadError(const char * sMessage) {
   bool bPrintAsNormal(true);
   std::map<eInputFileType, int>::iterator iter = gInputFileWarningsMap.find(geInputFileType);

   if (iter == gInputFileWarningsMap.end())
      gInputFileWarningsMap.insert(std::make_pair(geInputFileType, 1));
   else {
     iter->second++;
     // print the excessive warning message on the MAX_READ_ERRORS time - else print nothing past -- AJV
     if (iter->second == giMaximumReadErrors) {
       bPrintAsNormal = false;
       std::string message;
       message = "Error: Excessive number of errors reading ";
       message += GetImpliedFileTypeString().c_str();
       message += " data.\n";
       PrintError(message.c_str());
     }
     else if(iter->second > giMaximumReadErrors)
       bPrintAsNormal = false;
   }

   if (bPrintAsNormal)
     PrintError(sMessage);
}

void BasePrint::SetImpliedInputFileType(eInputFileType eType) {
  geInputFileType = eType;
  switch (eType) {
    case CASEFILE                : gsInputFileString = "case file"; break;
    case CONTROLFILE             : gsInputFileString = "control file"; break;
    case POPFILE                 : gsInputFileString = "population file"; break;
    case COORDFILE               : gsInputFileString = "coordinates file"; break;
    case GRIDFILE                : gsInputFileString = "grid file"; break;
    case MAXCIRCLEPOPFILE        : gsInputFileString = "max circle size file"; break;
    case ADJ_BY_RR_FILE          : gsInputFileString = "adjustments file"; break;
    case LOCATION_NEIGHBORS_FILE : gsInputFileString = "location neighbors file"; break;
    case META_LOCATIONS_FILE     : gsInputFileString = "meta locations file"; break;
    default : throw prg_error("Invalid input file type warning message!", "SetImpliedInputFileType()");
  }
}

