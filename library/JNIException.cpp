//---------------------------------------------------------------------------
#include "TreeScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "JNIException.h"

/** Returns the method id for an instance method of a class or interface. 
    Throws a jni_error exception if method can not be determined by its name and signature. */
jmethodID _getMethodId_Checked(JNIEnv& Env, jclass clazz, std::string funcName, std::string funcSigniture) {
	jmethodID mid = Env.GetMethodID(clazz, funcName.c_str(), funcSigniture.c_str());
	if (mid == NULL) {
		jni_error theError("Unknown class method. Function name: %s  Signature: %s", funcName.c_str(), funcSigniture.c_str());
		jni_error::_throwByName(Env, jni_error::_javaRuntimeExceptionClassName, theError.what());
		throw theError;
	}
	return mid;
}

/** Returns the field id for an instance field of a class. 
    Throws a jni_error exception if field can not be determined by its name and signature. */
jfieldID _getFieldId_Checked(JNIEnv& Env, jclass clazz, std::string fieldName, std::string fieldType) {
	jfieldID fid = Env.GetFieldID(clazz, fieldName.c_str(), fieldType.c_str());
	if (fid == NULL) {
		jni_error theError("Unknown class field. Field name: %s  Type: %s", fieldName.c_str(), fieldType.c_str());
		jni_error::_throwByName(Env, jni_error::_javaRuntimeExceptionClassName, theError.what());
		throw theError;
	}
	return fid;
}

///////////////// class jni_error ///////////////////////////////////////////////////////

const char * jni_error::_javaRuntimeExceptionClassName = "java/lang/RuntimeException";

jni_error::jni_error() : prg_exception() {}

jni_error::jni_error(const char * format, ...) : prg_exception() {
  try {
#ifdef _MSC_VER
    std::vector<char> temp(MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE);
    va_list varArgs;
    va_start (varArgs, format);
    vsnprintf(&temp[0], temp.size() - 1, format, varArgs);
    va_end(varArgs);
#else
    std::vector<char> temp(1);
    va_list varArgs;
    va_start(varArgs, format);
    size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, varArgs);
    va_end(varArgs);
    temp.resize(iStringLength + 1);
    va_start(varArgs, format);
    vsnprintf(&temp[0], iStringLength + 1, format, varArgs);
    va_end(varArgs);
#endif
    _what = &temp[0];
  }
  catch (...) {}
}

jni_error::~jni_error() throw() {}

/** Explicit call to throw new Java exception (if not already exists) and
    then throw C++ jni_error exception object. */
void jni_error::_throwException(JNIEnv& Env) {
   if (Env.ExceptionCheck())
     Env.ExceptionDescribe();
   else
     _throwByName(Env, _javaRuntimeExceptionClassName, "Runtime error in JNI code.");
   throw jni_error("JNI error condition detected.");
}

/** Throws jni_error exception if a Java exception exists.*/
void jni_error::_detectError(JNIEnv& Env) {
	if (Env.ExceptionCheck()) {
       Env.ExceptionDescribe();
       throw jni_error("JNI exception condition detected.");
   }
}

/** Throws bnamed Java object. After calling this function, limited JNI functions 
    may be called. */
void jni_error::_throwByName(JNIEnv& Env, const char *name, const char *msg) {
   jclass cls = Env.FindClass(name);
   /* if cls is NULL, an exception has already been thrown */
   if (cls != NULL) {
	   Env.ThrowNew(cls, msg);
   }
   /* free the local ref */
   Env.DeleteLocalRef(cls);
}
