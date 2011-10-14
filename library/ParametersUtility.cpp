//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersUtility.h"
#include "ParameterFileAccess.h"
#include "JNIException.h"
#include <iostream>

/** Reads parameters from file 'filename' in C++ code and sets class members of Java JParameters class. */
JNIEXPORT jboolean JNICALL Java_org_treescan_app_Parameters_Read(JNIEnv * pEnv, jobject jParameters, jstring filename) {
  Parameters parameters;
  jboolean iscopy;

  try {
     const char *sParameterFilename = pEnv->GetStringUTFChars(filename, &iscopy);
     if (sParameterFilename) {
       PrintNull NoPrint;
       ParameterAccessCoordinator(parameters).read(sParameterFilename);
     }
     else {
       //New session - creation version is this version.
       Parameters::CreationVersion vVersion = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};
       parameters.setVersion(vVersion);
     }
     if (iscopy == JNI_TRUE)
     	pEnv->ReleaseStringUTFChars(filename, sParameterFilename);
     ParametersUtility::copyCParametersToJParameters(*pEnv, parameters, jParameters);
  }
  catch (jni_error &) {    
    return 1; // let the Java exception to be handled in the caller of JNI function
  }
  catch (std::exception& x) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return 1;
  }
  catch (...) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return 1;
  }
  return true;
}

/** Set parameters of C++ object from Java object and writes parameters to file 'filename'. */
JNIEXPORT void JNICALL Java_org_treescan_app_Parameters_Write(JNIEnv * pEnv, jobject jParameters, jstring) {
  Parameters   parameters;

  try {
    ParametersUtility::copyJParametersToCParameters(*pEnv, jParameters, parameters);
    PrintNull NoPrint;
    ParameterAccessCoordinator(parameters).write(parameters.getSourceFileName().c_str());
  }
  catch (jni_error&) {    
    return; // let the Java exception to be handled in the caller of JNI function
  }
  catch (std::exception& x) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return;
  }
  catch (...) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return;
  }
}

/** Returns ordinal of enumeration gotten from 'sFunctionName' called. */
int ParametersUtility::getEnumTypeOrdinalIndex(JNIEnv& Env, jobject& jParameters, const char * sFunctionName, const char * sEnumClassSignature) {
  jclass clazz = Env.GetObjectClass(jParameters);
  jmethodID mid = _getMethodId_Checked(Env, clazz, sFunctionName, std::string(std::string("()") + sEnumClassSignature).c_str());
  jobject t_object = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  jclass t_class = Env.FindClass(sEnumClassSignature);
  jmethodID t_mid = _getMethodId_Checked(Env, t_class, "ordinal", "()I");
  int value = Env.CallIntMethod(t_object, t_mid);
  jni_error::_detectError(Env);
  return value;

}

/** Copies CParameter object data members to JParameters object. */
jobject& ParametersUtility::copyCParametersToJParameters(JNIEnv& Env, Parameters& Parameters, jobject& jParameters) {
  //set jParameters object from data members of CParameters class
  jclass clazz = Env.GetObjectClass(jParameters);

  jmethodID mid = _getMethodId_Checked(Env, clazz, "setNumProcesses", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getNumRequestedParallelProcesses());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setNumReplications", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getNumReplicationsRequested());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCountFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getCountFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setTreeFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getTreeFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setOutputFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getOutputFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setResultsFormat", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getResultsFormat());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSourceFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getSourceFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRandomlyGeneratingSeed", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.isRandomlyGeneratingSeed());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRandomizationSeed", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getRandomizationSeed());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setConditional", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.isConditional());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setDuplicates", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.isDuplicates());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPrintColunHeaders", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.isPrintColumnHeaders());
  jni_error::_detectError(Env);

  jfieldID vfid = _getFieldId_Checked(Env, clazz, "_creationversion", "Lorg/treescan/app/Parameters$CreationVersion;");
  jobject versionobject = Env.GetObjectField(jParameters, vfid);
  jclass vclazz = Env.GetObjectClass(versionobject);
  vfid = _getFieldId_Checked(Env, vclazz, "_major", "I");
  Env.SetIntField(versionobject, vfid, (jint)Parameters.getCreationVersion().iMajor);
  jni_error::_detectError(Env);
  vfid = _getFieldId_Checked(Env, vclazz, "_minor", "I");
  Env.SetIntField(versionobject, vfid, (jint)Parameters.getCreationVersion().iMinor);
  jni_error::_detectError(Env);
  vfid = _getFieldId_Checked(Env, vclazz, "_release", "I");
  Env.SetIntField(versionobject, vfid, (jint)Parameters.getCreationVersion().iRelease);
  jni_error::_detectError(Env);

  return jParameters;
}

/** Copies JParameter object data members to CParameters object. */

Parameters& ParametersUtility::copyJParametersToCParameters(JNIEnv& Env, jobject& jParameters, Parameters& Parameters) {
  jboolean              iscopy;
  const char          * sFilename;

  //set CParameter class from jParameters object
  jclass clazz = Env.GetObjectClass(jParameters);

  jmethodID mid = _getMethodId_Checked(Env, clazz, "getNumRequestedParallelProcesses", "()I");
  Parameters.setNumProcesses(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getNumReplicationsRequested", "()I");
  Parameters.setNumReplications(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getCountFileName", "()Ljava/lang/String;");
  jstring jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setCountFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getTreeFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setTreeFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getOutputFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setOutputFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  Parameters.setResultsFormat((Parameters::ResultsFormat)getEnumTypeOrdinalIndex(Env, jParameters, "getResultsFormat", "Lorg/treescan/app/Parameters$ResultsFormat;"));

  mid = _getMethodId_Checked(Env, clazz, "getSourceFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setSourceFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "isRandomlyGeneratingSeed", "()Z");
  Parameters.setRandomlyGeneratingSeed(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRandomizationSeed", "()I");
  Parameters.setRandomizationSeed(static_cast<long>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isConditional", "()Z");
  Parameters.setConditional(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isDuplicates", "()Z");
  Parameters.setDuplicates(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isPrintColumnHeaders", "()Z");
  Parameters.setPrintColunHeaders(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  return Parameters;
}

