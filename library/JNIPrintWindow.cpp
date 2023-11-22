//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "JNIPrintWindow.h"
#include "JNIException.h"

#if defined(_MSC_VER)
#pragma warning( disable : 4800 )
#endif

/** Constructor */
JNIPrintWindow::JNIPrintWindow(JNIEnv& JNI_Env, jobject& ProgressWindowObj, bool bSuppressWarnings)
               :BasePrint(bSuppressWarnings), gJNI_Env(JNI_Env) /*, gProgressWindowObj(ProgressWindowObj)*/ {
  gProgressWindowObj = gJNI_Env.NewGlobalRef(ProgressWindowObj);


  if (JNI_Env.GetJavaVM(&pJVM) < 0)
     jni_error::_throwByName(gJNI_Env, jni_error::_javaRuntimeExceptionClassName, "Unable to get JavaVM interface pointer.");
  //NOTE: We only need the JVM pointer when executing calculation engine with boost::threads.
  //      I'm not sure how expensive it is to attach and detach a thread, so it might
  //      be better to define a switching feature; where when executing in boost::thread -
  //      we must retrieve JNIEnv pointer - else use passed JNIEnv object.

  jclass clazz = gJNI_Env.GetObjectClass(gProgressWindowObj);
  gCancelStatusMethodId = _getMethodId_Checked(gJNI_Env, clazz, "IsCancelled", "()Z");
  gPrintStandardMethodId = _getMethodId_Checked(gJNI_Env, clazz, "PrintStandard", "(Ljava/lang/String;)V");
  gPrintErrorMethodId = _getMethodId_Checked(gJNI_Env, clazz, "PrintError", "(Ljava/lang/String;)V");
  gPrintWarningMethodId = _getMethodId_Checked(gJNI_Env, clazz, "PrintWarning", "(Ljava/lang/String;)V");
  gPrintNoticeMethodId = _getMethodId_Checked(gJNI_Env, clazz, "PrintNotice", "(Ljava/lang/String;)V");
  gSetCallpathMethodId = _getMethodId_Checked(gJNI_Env, clazz, "setCallpath", "(Ljava/lang/String;)V");
}

/** Destructor */
JNIPrintWindow::~JNIPrintWindow(){
  gJNI_Env.DeleteGlobalRef(gProgressWindowObj);
  gProgressWindowObj = NULL;
}

/** Returns whether analysis has been cancelled through run analysis window via calculation thread . */
bool JNIPrintWindow::GetIsCanceled() const {
  bool bReturn=false;

  JNIEnv *env;
  if (pJVM->AttachCurrentThread((void**)&env, NULL) < 0)
     jni_error::_throwByName(gJNI_Env, jni_error::_javaRuntimeExceptionClassName, "Unable to attach current thread to JVM.");
  bReturn = env->CallBooleanMethod(gProgressWindowObj, gCancelStatusMethodId);
  //jni_error::_detectError(&env);
  pJVM->DetachCurrentThread(); //if (pJVM->DetachCurrentThread() < 0) printf("Unable to detach current thread from JVM.\n");
  return bReturn;
}

void JNIPrintWindow::PrintError(const char * sMessage) {
  JNIEnv *env;
  if (pJVM->AttachCurrentThread((void**)&env, NULL) < 0)
     jni_error::_throwByName(gJNI_Env, jni_error::_javaRuntimeExceptionClassName, "Unable to attach current thread to JVM.");
  jstring jMessage= env->NewStringUTF(sMessage);
  env->CallVoidMethod(gProgressWindowObj, gPrintErrorMethodId, jMessage);
  env->DeleteLocalRef(jMessage);
  //jni_error::_detectError(&env);
  pJVM->DetachCurrentThread(); //if (pJVM->DetachCurrentThread() < 0) printf("Unable to detach current thread from JVM.\n");
}

void JNIPrintWindow::PrintNotice(const char * sMessage) {
  JNIEnv *env;
  if (pJVM->AttachCurrentThread((void**)&env, NULL) < 0)
     jni_error::_throwByName(gJNI_Env, jni_error::_javaRuntimeExceptionClassName, "Unable to attach current thread to JVM.");
  jstring jMessage= env->NewStringUTF(sMessage);
  env->CallVoidMethod(gProgressWindowObj, gPrintNoticeMethodId, jMessage);
  env->DeleteLocalRef(jMessage);
  //jni_error::_detectError(&env);
  pJVM->DetachCurrentThread(); //if (pJVM->DetachCurrentThread() < 0) printf("Unable to detach current thread from JVM.\n");
}

void JNIPrintWindow::PrintStandard(const char * sMessage) {
  JNIEnv *env;
  if (pJVM->AttachCurrentThread((void**)&env, NULL) < 0)
     jni_error::_throwByName(gJNI_Env, jni_error::_javaRuntimeExceptionClassName, "Unable to attach current thread to JVM.");
  jstring jMessage= env->NewStringUTF(sMessage);
  env->CallVoidMethod(gProgressWindowObj, gPrintStandardMethodId, jMessage);
  env->DeleteLocalRef(jMessage);
  //jni_error::_detectError(&env);
  pJVM->DetachCurrentThread(); //if (pJVM->DetachCurrentThread() < 0) printf("Unable to detach current thread from JVM.\n");
}

void JNIPrintWindow::PrintWarning(const char * sMessage) {
  JNIEnv *env;
  if (pJVM->AttachCurrentThread((void**)&env, NULL) < 0)
     jni_error::_throwByName(gJNI_Env, jni_error::_javaRuntimeExceptionClassName, "Unable to attach current thread to JVM.");
  jstring jMessage= env->NewStringUTF(sMessage);
  env->CallVoidMethod(gProgressWindowObj, gPrintWarningMethodId, jMessage);
  env->DeleteLocalRef(jMessage);
  //jni_error::_detectError(&env);
  pJVM->DetachCurrentThread(); //if (pJVM->DetachCurrentThread() < 0) printf("Unable to detach current thread from JVM.\n");
}

/** Creates formatted output from variable number of parameter arguments and calls class Print() method. */
void JNIPrintWindow::Printf(const char * sMessage, PrintType ePrintType, ...) {
  if (!sMessage || sMessage == &gsMessage[0]) return;
   
  try {
#ifdef _MSC_VER
    va_list varArgs;
    va_start (varArgs, ePrintType);
    vsnprintf(&gsMessage[0], gsMessage.size() - 1, sMessage, varArgs);
    va_end(varArgs);
#else
    va_list varArgs;
    va_start(varArgs, ePrintType);
    size_t iStringLength = vsnprintf(&gsMessage[0], gsMessage.size(), sMessage, varArgs);
    va_end(varArgs);
    gsMessage.resize(iStringLength + 1);
    va_start(varArgs, ePrintType);
    vsnprintf(&gsMessage[0], iStringLength + 1, sMessage, varArgs);
    va_end(varArgs);
#endif
  }
  catch (...) {}
  Print(&gsMessage[0], ePrintType);
}

void JNIPrintWindow::RecordCallpath(const char * sCallpath) {
  JNIEnv *env;
  if (pJVM->AttachCurrentThread((void**)&env, NULL) < 0)
     jni_error::_throwByName(gJNI_Env, jni_error::_javaRuntimeExceptionClassName, "Unable to attach current thread to JVM.");
  jstring jMessage= env->NewStringUTF(sCallpath);
  env->CallVoidMethod(gProgressWindowObj, gSetCallpathMethodId, jMessage);
  env->DeleteLocalRef(jMessage);
  //jni_error::_detectError(&env);
  pJVM->DetachCurrentThread(); //if (pJVM->DetachCurrentThread() < 0) printf("Unable to detach current thread from JVM.\n");
}

