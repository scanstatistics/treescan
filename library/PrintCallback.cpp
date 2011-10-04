#include "TreeScan.h"
#pragma hdrstop
#include "PrintCallback.h"

/////////////// C callback class  ////////////////////////////////////

/** constructor */
C_PrintCallback::C_PrintCallback(C_Callback * callback, bool bSuppressWarnings)
                :CallbackPrint(bSuppressWarnings) {
  _callback = *callback;
}

void C_PrintCallback::print(const char * sMessage) {
  _cancelled = (_callback(sMessage) != 0);
}

/////////////// Python callback class  ////////////////////////////////

#ifdef _PYTHON_CALLBACK_
#pragma comment(lib, "python25.lib") // -- actually, this needs to go into: Linker -> Input -> Additional Dependencies
/** constructor */
PY_PrintCallback::PY_PrintCallback(PY_Callback * callback, bool bSuppressWarnings)
                :CallbackPrint(bSuppressWarnings) {
  //_callback = *callback;
  _callback = callback;//(C_Callback(callback));
}

void PY_PrintCallback::print(const char * sMessage) {
  _cancelled = (PY_Callback(_callback)(Py_BuildValue("s", sMessage)) != 0);
}
#endif

/////////////// Visual Basic callback class  //////////////////////////

#ifdef _WINDOWS_
VB_PrintCallback::VB_PrintCallback(long callBackAddress, bool bSuppressWarnings)
                 :CallbackPrint(bSuppressWarnings) {
  // Point the function pointer at the passed-in address.
  _vb_callback = (VB_Callback)callBackAddress;
}

void VB_PrintCallback::print(const char * sMessage) {
   _cancelled = (_vb_callback(sMessage) != 0);
}
#endif
