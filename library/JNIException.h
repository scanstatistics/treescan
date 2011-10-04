//*****************************************************************************
#ifndef  __JNIEXCEPTIONCLASS_H
#define  __JNIEXCEPTIONCLASS_H
//*****************************************************************************
#include "PrjException.h"
#include <jni.h>

jmethodID   _getMethodId_Checked(JNIEnv& Env, jclass clazz, std::string funcName, std::string funcSigniture);
jfieldID    _getFieldId_Checked(JNIEnv& Env, jclass clazz, std::string fieldName, std::string fieldType);

/** Native code may handle a pending exception in two ways:
     • The native method implementation can choose to return immediately, causing
       the exception to be handled in the caller.
     • The native code can clear the exception by calling ExceptionClear and then
       execute its own exception handling code. 
     
	 The intent of this class is to provide a means to jump a catch statement in the 
	 initiating JNI function call; essentially allowing for the bullet listed above.
 
	   */
class jni_error : public prg_exception {
  protected:
    jni_error();

  public:
    jni_error(const char * format, ...);
    virtual ~jni_error() throw();

    static const char *   _javaRuntimeExceptionClassName;
    static void           _detectError(JNIEnv& Env);
    static void           _throwException(JNIEnv& Env);
    static void           _throwByName(JNIEnv& Env, const char *name, const char *msg);
};

//*****************************************************************************
#endif
