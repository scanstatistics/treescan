//******************************************************************************
#ifndef stsJNIPrintWindowH
#define stsJNIPrintWindowH
//******************************************************************************
#include "BasePrint.h"
#include "boost/thread/mutex.hpp"
#include <jni.h>

/** Print direction class that directs messages to calculation thread so
    that communication with analysis run window is synchronized with main
    VCL thread. */
class JNIPrintWindow : public BasePrint {
  protected:
    JavaVM            * pJVM; 
    JNIEnv            & gJNI_Env;
    jobject             gProgressWindowObj;
    jmethodID           gCancelStatusMethodId;
    jmethodID           gPrintErrorMethodId;
    jmethodID           gPrintNoticeMethodId;
    jmethodID           gPrintStandardMethodId;
    jmethodID           gPrintWarningMethodId;
    jmethodID           gSetCallpathMethodId;

    virtual void        PrintError(const char * sMessage);
    virtual void        PrintNotice(const char * sMessage);
    virtual void        PrintStandard(const char * sMessage);
    virtual void        PrintWarning(const char * sMessage);

   public:
     JNIPrintWindow(JNIEnv& JNI_Env, jobject& ProgressWindowObj, bool bSuppressWarnings);
     virtual ~JNIPrintWindow();

     bool               GetIsCanceled() const;
     virtual void       Printf(const char * sMessage, PrintType ePrintType, ...);
     void               RecordCallpath(const char * sCallpath);
};
//******************************************************************************
#endif

