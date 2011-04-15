//*****************************************************************************
#ifndef __PrintScreen_H
#define __PrintScreen_H
//*****************************************************************************
#include "BasePrint.h"

class PrintScreen : public BasePrint {
  protected:
    inline void PrintError(const char * sMessage) {fprintf(stderr, sMessage);}
    inline void PrintNotice(const char * sMessage) {fprintf(stderr, sMessage);}
    inline void PrintStandard(const char * sMessage) {fprintf(stdout, sMessage);}
    inline void PrintWarning(const char * sMessage) {fprintf(stderr, sMessage);}
  
  public:
    PrintScreen(bool bSuppressWarnings);
    virtual ~PrintScreen();

    inline bool GetIsCanceled() const {return false;}
};
//*****************************************************************************
#endif
