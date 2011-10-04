//*****************************************************************************
#ifndef __PrintCallback_H
#define __PrintCallback_H
//*****************************************************************************
#include "BasePrint.h"

#ifndef CALLBACK
#define CALLBACK
#endif

/** Abstract base class that extends BasePrint class. */
class CallbackPrint : public BasePrint {
  private:
    virtual void   print(const char * sMessage) = 0;

  protected:
    bool           _cancelled; 

    void PrintError(const char * sMessage) {print(sMessage);}
    inline void PrintNotice(const char * sMessage) {print(sMessage);}
    inline void PrintStandard(const char * sMessage) {print(sMessage);}
    inline void PrintWarning(const char * sMessage) {print(sMessage);}
  
  public:
    CallbackPrint(bool bSuppressWarnings):BasePrint(bSuppressWarnings) {_cancelled=false;}
    virtual ~CallbackPrint() {}

    inline bool GetIsCanceled() const {
        return _cancelled;
    }
};

/** Extends CallbackPrint class to invoke C function callback. */
typedef int (CALLBACK *C_Callback)(const char * sMessage);
class C_PrintCallback : public CallbackPrint {
  private:
    C_Callback     _callback;
    virtual void   print(const char * sMessage);

  public:
    C_PrintCallback(C_Callback * callback, bool bSuppressWarnings);
    virtual ~C_PrintCallback() {}

};


#ifdef _PYTHON_CALLBACK_
#include "Python.h" // -- would need to add path to: C/C++ -> General -> Additional Include Directories

/** Extends CallbackPrint class to invoke Python function callback. */
typedef int (CALLBACK *PY_Callback)(PyObject * sMessage); 
class PY_PrintCallback : public CallbackPrint {
  private:
    PY_Callback  * _callback;
    virtual void   print(const char * sMessage);

  public:
    PY_PrintCallback(PY_Callback * callback, bool bSuppressWarnings);
    virtual ~PY_PrintCallback() {}

};
#endif


#ifdef _WINDOWS_
/** Extends CallbackPrint class to invoke Visual Basic function callback. */
typedef int (__stdcall *VB_Callback)(const char * pbstr);
class VB_PrintCallback : public CallbackPrint {
  private:
    VB_Callback    _vb_callback;
    virtual void   print(const char * sMessage);
 
  public:
    VB_PrintCallback(long callBackAddress, bool bSuppressWarnings);
    virtual ~VB_PrintCallback() {}
};
#endif


#ifdef _WINDOWS_
#define DLL_EXP __declspec(dllexport)
#else
#define DLL_EXP
#endif

// Declaration for the RunAnalysis functions.
extern "C" {
  int DLL_EXP C_RunAnalysis(const char * filename, C_Callback* call_back);
#ifdef _PYTHON_CALLBACK_
  int DLL_EXP PY_RunAnalysis(const char * filename, PY_Callback* call_back);
#endif
#ifdef _WINDOWS_
  int DLL_EXP VB_RunAnalysis(const char * filename, long callBackAddress);
#endif
}
//*****************************************************************************
#endif
