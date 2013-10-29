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

  mid = _getMethodId_Checked(Env, clazz, "setPopulationFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getPopulationFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCutsFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getCutsFileName().c_str()));
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

  //mid = _getMethodId_Checked(Env, clazz, "setDuplicates", "(Z)V");
  //Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.isDuplicates());
  //jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setGeneratingHtmlResults", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.isGeneratingHtmlResults());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setGeneratingTableResults", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.isGeneratingTableResults());
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

  mid = _getMethodId_Checked(Env, clazz, "setModelType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getModelType());
  jni_error::_detectError(Env);

  Parameters::ratio_t ratio = Parameters.getProbabilityRatio();
  mid = _getMethodId_Checked(Env, clazz, "setProbabilityRatioNumerator", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)ratio.first);
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "setProbabilityRatioDenominator", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)ratio.second);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setScanType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getScanType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setConditionalType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getConditionalType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setDataTimeRangeBegin", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getDataTimeRangeSet().getDataTimeRangeSets().begin()->getStart());
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "setDataTimeRangeClose", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getDataTimeRangeSet().getDataTimeRangeSets().begin()->getEnd());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setTemporalStartRangeBegin", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getTemporalStartRange().getStart());
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "setTemporalStartRangeClose", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getTemporalStartRange().getEnd());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setTemporalEndRangeBegin", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getTemporalEndRange().getStart());
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "setTemporalEndRangeClose", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getTemporalEndRange().getEnd());
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

  mid = _getMethodId_Checked(Env, clazz, "getPopulationFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setPopulationFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getCutsFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setCutsFileName(sFilename);
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
  Parameters.setRandomlyGeneratingSeed(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRandomizationSeed", "()I");
  Parameters.setRandomizationSeed(static_cast<long>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  //mid = _getMethodId_Checked(Env, clazz, "isDuplicates", "()Z");
  //Parameters.setDuplicates(Env.CallBooleanMethod(jParameters, mid));
  //jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isGeneratingHtmlResults", "()Z");
  Parameters.setGeneratingHtmlResults(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isGeneratingTableResults", "()Z");
  Parameters.setGeneratingTableResults(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isPrintColumnHeaders", "()Z");
  Parameters.setPrintColunHeaders(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  Parameters.setModelType((Parameters::ModelType)getEnumTypeOrdinalIndex(Env, jParameters, "getModelType", "Lorg/treescan/app/Parameters$ModelType;"));

  Parameters::ratio_t ratio;
  mid = _getMethodId_Checked(Env, clazz, "getProbabilityRatioNumerator", "()I");
  ratio.first = static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "getProbabilityRatioDenominator", "()I");
  ratio.second = static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);
  Parameters.setProbabilityRatio(ratio);

  Parameters.setScanType((Parameters::ScanType)getEnumTypeOrdinalIndex(Env, jParameters, "getScanType", "Lorg/treescan/app/Parameters$ScanType;"));
  Parameters.setConditionalType((Parameters::ConditionalType)getEnumTypeOrdinalIndex(Env, jParameters, "getConditionalType", "Lorg/treescan/app/Parameters$ConditionalType;"));

  mid = _getMethodId_Checked(Env, clazz, "getDataTimeRangeBegin", "()I");
  int begin = Env.CallIntMethod(jParameters, mid);
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "getDataTimeRangeClose", "()I");
  int close = Env.CallIntMethod(jParameters, mid);
  jni_error::_detectError(Env);
  DataTimeRangeSet range_set;
  range_set.add(DataTimeRange(begin, close));
  Parameters.setDataTimeRangeSet(range_set);

  mid = _getMethodId_Checked(Env, clazz, "getTemporalStartRangeBegin", "()I");
  begin = Env.CallIntMethod(jParameters, mid);
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "getTemporalStartRangeClose", "()I");
  close = Env.CallIntMethod(jParameters, mid);
  jni_error::_detectError(Env);
  Parameters.setTemporalStartRange(DataTimeRange(begin, close));

  mid = _getMethodId_Checked(Env, clazz, "getTemporalEndRangeBegin", "()I");
  begin = Env.CallIntMethod(jParameters, mid);
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "getTemporalEndRangeClose", "()I");
  close = Env.CallIntMethod(jParameters, mid);
  jni_error::_detectError(Env);
  Parameters.setTemporalEndRange(DataTimeRange(begin, close));

  return Parameters;
}
