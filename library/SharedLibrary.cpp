//******************************************************************************
//#include "SharedLibrary.h"
#pragma hdrstop
//******************************************************************************
//#ifdef WINDOWS
//#include <windows.h>
//#endif
#include <time.h>
#include "UtilityFunctions.h"
#include "PrintScreen.h"
#include "ScanRunner.h"
#include "PrjException.h"
#include <jni.h>
#include "JNIPrintWindow.h"
#include "ParametersUtility.h"
#include "FileName.h"
#include "JNIException.h"
#include "PrintCallback.h"
#include "org_treescan_app_CalculationThread.h"
#include "org_treescan_gui_ParameterSettingsFrame.h"
#include "org_treescan_app_AppConstants.h"
#include "Toolkit.h"
#include "ParameterFileAccess.h"
#include "ParametersValidate.h"
//#pragma argsused

void __TreeScanInit() {
printf("__TreeScanInit called\n"); 
  reserve_memory_cache();
  std::set_new_handler(prg_new_handler);
  std::string dir;
  FileName::getCurDirectory(dir);
  dir += "satscan.exe";
  AppToolkit::ToolKitCreate(dir.c_str());
}

void __TreeScanExit() {
printf("__TreeScanExit called\n"); 
}

#ifdef __GNUC__
void __attribute__((constructor)) my_init()
{
      __TreeScanInit();
      //printf("initialized\n");
}
void __attribute__((destructor)) my_fini()
{
      //printf("existing\n");
      __TreeScanExit();
}
#elif defined(_MSC_VER)
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID/*lpReserved*/)
{

    if (dwReason == DLL_PROCESS_ATTACH) {
      //printf("initializing\n");
      __TreeScanInit();
    }
    if (dwReason == DLL_PROCESS_DETACH) {
      //printf("existing\n");
      __TreeScanExit();
    }
   return 1;
}
#else
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{

    if (reason == DLL_PROCESS_ATTACH) {
      printf("initializing\n");
      __TreeScanInit();
    }
    if (reason == DLL_PROCESS_DETACH) {
      printf("existing\n");
      __TreeScanExit();
    }
   return 1;
}
#endif

/** Run execution function given passed parameter file and base print. */
void _runAnalysis(const Parameters& parameters, BasePrint& Console) {
  std::string           sMessage;

  Console.Printf(AppToolkit::getToolkit().GetAcknowledgment(sMessage), BasePrint::P_STDOUT);

  /* validate parameters - print errors to console */
  if (!ParametersValidate(parameters).Validate(Console))
    throw resolvable_error("\nThe parameter file contains incorrect settings that prevent TreeScan from continuing. "
                           "Please review above message(s) and modify parameter settings accordingly.");

  //create analysis runner object and execute analysis
  ScanRunner runner(parameters, Console);
  runner.run();
}

///////////////////////////////// JNI Shared Library Methods ///////////////////////////////////////////

JNIEXPORT jstring JNICALL Java_org_treescan_app_AppConstants_getVersion(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(AppToolkit::getToolkit().GetVersion());
}

JNIEXPORT jstring JNICALL Java_org_treescan_app_AppConstants_getWebSite(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(AppToolkit::getToolkit().GetWebSite());
}

JNIEXPORT jstring JNICALL Java_org_treescan_app_AppConstants_getSubstantiveSupportEmail(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(AppToolkit::getToolkit().GetSubstantiveSupportEmail());
}

JNIEXPORT jstring JNICALL Java_org_treescan_app_AppConstants_getTechnicalSupportEmail(JNIEnv *pEnv, jclass) {
  return pEnv->NewStringUTF(AppToolkit::getToolkit().GetTechnicalSupportEmail());
}

JNIEXPORT jstring JNICALL Java_org_treescan_app_AppConstants_getReleaseDate(JNIEnv *pEnv, jclass) {
  return pEnv->NewStringUTF(VERSION_DATE);
}

JNIEXPORT jstring JNICALL Java_org_treescan_app_AppConstants_getVersionId(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(VERSION_ID);
}

JNIEXPORT jint JNICALL Java_org_treescan_app_CalculationThread_RunAnalysis(JNIEnv *pEnv, jobject JCalculationThread, jobject JParameters) {
  try {
    Parameters           Parameters;
    JNIPrintWindow        Console(*pEnv, JCalculationThread, false);
     
    try {
      ParametersUtility::copyJParametersToCParameters(*pEnv, JParameters, Parameters);
      _runAnalysis(Parameters, Console);
    } catch (resolvable_error & x) {
      Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
      return 1;
    } catch (jni_error&) {
      // Let the Java exception to be handled in the caller of JNI function.
      // It is preferable to report the error through the JNIPrintWindow
      // object but once a java error exists, our options are limited.
      return 1; 
    } catch (prg_exception & x) {
      Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
      Console.RecordCallpath(x.trace());
      return 1;
    } catch (std::bad_alloc&) {
      Console.Printf("\nTreeScan is unable to perform analysis due to insuffient memory.\n"
                     "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                     "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
      return 1;
    } catch (std::exception& x) {
      Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
      return 1;
    }  
  } catch (jni_error&) {
    // Let the Java exception to be handled in the caller of JNI function.
    // It is preferable to report the error through the JNIPrintWindow
    // object but once a java error exists, our options are limited.
    return 1; 
  } catch (...) {
    jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return 1;
  } 

  return 0;
}

///////////////////////////////// C Shared Library Methods ///////////////////////////////////////////

/* Alternative ways of doing this.
1) The good old functionpointers.
2) Functionobjects (these are classes that define the operator () ()
3) The template way:
      template<typename Callable>
      void registerCallback(Callable const &call_back);
4) The Functors of the Lokilibrary.
*/

int DLL_EXP C_RunAnalysis(const char * filename, C_Callback* call_back) {
  Parameters            parameters;
  C_PrintCallback       Console(call_back, false);
     
  try {
    //TODO: ParameterAccessCoordinator reader(Parameters);
    //reader.Read(filename, Console);
    parameters.read(filename, Parameters::XML);
    _runAnalysis(parameters, Console);
  } catch (resolvable_error & x) {
    Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (prg_exception & x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (std::bad_alloc&) {
    Console.Printf("\nTreeScan is unable to perform analysis due to insuffient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                   "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  } catch (std::exception& x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (...) {
    Console.Printf("\nUnknown Program Error Encountered\n\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  }
  
  return 0;
}

///////////////////////////////// Python Shared Library Methods ///////////////////////////////////////////

#ifdef _PYTHON_CALLBACK_
int DLL_EXP PY_RunAnalysis(const char * filename, PY_Callback* call_back) {

  //(C_Callback(call_back))("hello");
  //return 978;
  Parameters           parameters;
  PY_PrintCallback       Console(call_back, false);
     
  try {
    //TODO: ParameterAccessCoordinator reader(Parameters);
    //reader.Read(filename, Console);
    parameters.read(filename, Parameters::XML);
    _runAnalysis(parameters, Console);
  } catch (resolvable_error & x) {
    Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (prg_exception & x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (std::bad_alloc&) {
    Console.Printf("\nTreeScan is unable to perform analysis due to insuffient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                   "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  } catch (std::exception& x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (...) {
    Console.Printf("\nUnknown Program Error Encountered\n\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  }
  
  return 0;
}
#endif

///////////////////////////////// VB Shared Library Methods ///////////////////////////////////////////

#ifdef _WINDOWS_
int DLL_EXP VB_RunAnalysis(const char * filename, long cbAddress) {
  Parameters        parameters;
  VB_PrintCallback  Console(cbAddress, false);
     
  try {
    // TODO ParameterAccessCoordinator reader(Parameters);
    //reader.Read(filename, Console);
    parameters.read(filename, Parameters::XML);
    _runAnalysis(parameters, Console);
  } catch (resolvable_error & x) {
    Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (prg_exception & x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (std::bad_alloc&) {
    Console.Printf("\nTreeScan is unable to perform analysis due to insuffient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                   "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  } catch (std::exception& x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (...) {
    Console.Printf("\nUnknown Program Error Encountered\n\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  }  
  return 0;
}
#endif
//******************************************************************************
