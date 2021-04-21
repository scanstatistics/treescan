//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersUtility.h"
#include "ParameterFileAccess.h"
#include "JNIException.h"
#include "ScanRunner.h"
#include <iostream>

/** Reads parameters from file 'filename' in C++ code and sets class members of Java JParameters class. */
JNIEXPORT jboolean JNICALL Java_org_treescan_app_Parameters_Read(JNIEnv * pEnv, jobject jParameters, jstring filename) {
  Parameters parameters;
  jboolean iscopy;

  try {
     const char *sParameterFilename = pEnv->GetStringUTFChars(filename, &iscopy);
     if (sParameterFilename) {
       PrintNull NoPrint;
       ParameterAccessCoordinator(parameters).read(sParameterFilename, NoPrint);
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
    ParameterAccessCoordinator(parameters).write(parameters.getSourceFileName().c_str(), NoPrint);
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

JNIEXPORT jstring JNICALL Java_org_treescan_app_Parameters_getAlphaSpentToDateString(JNIEnv * pEnv, jclass JParameters , jstring filename) {
    jboolean iscopy;

    try {
        const char *sParameterFilename = pEnv->GetStringUTFChars(filename, &iscopy);
        std::string buffer(sParameterFilename);
        if (iscopy == JNI_TRUE)
            pEnv->ReleaseStringUTFChars(filename, sParameterFilename);
        std::stringstream stringbuffer;
        stringbuffer << SequentialStatistic::getAlphaSpentToDate(buffer);
        return pEnv->NewStringUTF(stringbuffer.str().c_str());
    }
    catch (jni_error&) {
        return pEnv->NewStringUTF("-1.0"); // let the Java exception to be handled in the caller of JNI function
    }
    catch (std::exception& x) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
        return pEnv->NewStringUTF("-1.0");
    }
    catch (...) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
        return pEnv->NewStringUTF("-1.0");
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
jobject& ParametersUtility::copyCParametersToJParameters(JNIEnv& Env, Parameters& parameters, jobject& jParameters) {
  //set jParameters object from data members of CParameters class
  jclass clazz = Env.GetObjectClass(jParameters);

  jmethodID mid = _getMethodId_Checked(Env, clazz, "setNumProcesses", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getNumRequestedParallelProcesses());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setNumReplications", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getNumReplicationsRequested());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCountFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(parameters.getCountFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setControlFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(parameters.getControlFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCutsFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(parameters.getCutsFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setTreeFileName", "(Ljava/lang/String;I)V");
  for (Parameters::FileNameContainer_t::const_iterator itr=parameters.getTreeFileNames().begin(); itr != parameters.getTreeFileNames().end(); ++itr) {
    Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(itr->c_str()), (jint)(std::distance(parameters.getTreeFileNames().begin(), itr) + 1));
    jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "setOutputFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(parameters.getOutputFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setResultsFormat", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getResultsFormat());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSourceFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(parameters.getSourceFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setIsProspectiveAnalysis", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getIsProspectiveAnalysis());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRandomlyGeneratingSeed", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.isRandomlyGeneratingSeed());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRandomizationSeed", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getRandomizationSeed());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setGeneratingHtmlResults", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.isGeneratingHtmlResults());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setGeneratingTableResults", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.isGeneratingTableResults());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPrintColunHeaders", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.isPrintColumnHeaders());
  jni_error::_detectError(Env);

  jfieldID vfid = _getFieldId_Checked(Env, clazz, "_creationversion", "Lorg/treescan/app/Parameters$CreationVersion;");
  jobject versionobject = Env.GetObjectField(jParameters, vfid);
  jclass vclazz = Env.GetObjectClass(versionobject);
  vfid = _getFieldId_Checked(Env, vclazz, "_major", "I");
  Env.SetIntField(versionobject, vfid, (jint)parameters.getCreationVersion().iMajor);
  jni_error::_detectError(Env);
  vfid = _getFieldId_Checked(Env, vclazz, "_minor", "I");
  Env.SetIntField(versionobject, vfid, (jint)parameters.getCreationVersion().iMinor);
  jni_error::_detectError(Env);
  vfid = _getFieldId_Checked(Env, vclazz, "_release", "I");
  Env.SetIntField(versionobject, vfid, (jint)parameters.getCreationVersion().iRelease);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setModelType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getModelType());
  jni_error::_detectError(Env);

  Parameters::ratio_t ratio = parameters.getProbabilityRatio();
  mid = _getMethodId_Checked(Env, clazz, "setProbabilityRatioNumerator", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)ratio.first);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setProbabilityRatioDenominator", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)ratio.second);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setScanType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getScanType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setConditionalType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getConditionalType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPrecisionOfTimesType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getDatePrecisionType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRestrictTemporalWindows", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getRestrictTemporalWindows());
  jni_error::_detectError(Env);

  if (Parameters::isTemporalScanType(parameters.getScanType()) && parameters.getDataTimeRangeSet().getDataTimeRangeSets().size()) {
    std::pair<std::string, std::string> data_time_range;
    std::stringstream buffer;
    if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC) {
        buffer << parameters.getDataTimeRangeSet().getDataTimeRangeSets().begin()->getStart();
        data_time_range.first = buffer.str();
        buffer.str("");
        buffer << parameters.getDataTimeRangeSet().getDataTimeRangeSets().begin()->getEnd();
        data_time_range.second = buffer.str();
    } else {
        const DataTimeRange& range = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front();
        data_time_range = range.rangeToGregorianStrings(range.getStart(), range.getEnd(), parameters.getDatePrecisionType());
    }
    mid = _getMethodId_Checked(Env, clazz, "setDataTimeRangeBegin", "(Ljava/lang/String;)V");
    Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(data_time_range.first.c_str()));
    jni_error::_detectError(Env);
    mid = _getMethodId_Checked(Env, clazz, "setDataTimeRangeClose", "(Ljava/lang/String;)V");
    Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(data_time_range.second.c_str()));
    jni_error::_detectError(Env);

    if (parameters.getRestrictTemporalWindows()) {
        if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC) {
            buffer.str("");
            buffer << parameters.getTemporalStartRange().getStart();
            data_time_range.first = buffer.str();
            buffer.str("");
            buffer << parameters.getTemporalStartRange().getEnd();
            data_time_range.second = buffer.str();
        } else {
            const DataTimeRange& range = parameters.getTemporalStartRange();
            data_time_range = range.rangeToGregorianStrings(range.getStart(), range.getEnd(), parameters.getDatePrecisionType());
        }
        mid = _getMethodId_Checked(Env, clazz, "setStartRangeStartDate", "(Ljava/lang/String;)V");
        Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(data_time_range.first.c_str()));
        jni_error::_detectError(Env);
        mid = _getMethodId_Checked(Env, clazz, "setStartRangeEndDate", "(Ljava/lang/String;)V");
        Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(data_time_range.second.c_str()));
        jni_error::_detectError(Env);

        if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC) {
            buffer.str("");
            buffer << parameters.getTemporalEndRange().getStart();
            data_time_range.first = buffer.str();
            buffer.str("");
            buffer << parameters.getTemporalEndRange().getEnd();
            data_time_range.second = buffer.str();
        } else {
            const DataTimeRange& range = parameters.getTemporalEndRange();
            data_time_range = range.rangeToGregorianStrings(range.getStart(), range.getEnd(), parameters.getDatePrecisionType());
        }
        mid = _getMethodId_Checked(Env, clazz, "setEndRangeStartDate", "(Ljava/lang/String;)V");
        Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(data_time_range.first.c_str()));
        jni_error::_detectError(Env);
        mid = _getMethodId_Checked(Env, clazz, "setEndRangeEndDate", "(Ljava/lang/String;)V");
        Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(data_time_range.second.c_str()));
        jni_error::_detectError(Env);
    }
  }

  mid = _getMethodId_Checked(Env, clazz, "setGeneratingLLRResults", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.isGeneratingLLRResults());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setMaximumWindowPercentage", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)parameters.getMaximumWindowPercentage());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setMaximumWindowLength", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getMaximumWindowLength());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setMaximumWindowType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getMaximumWindowType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setMinimumWindowLength", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getMinimumWindowLength());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPerformPowerEvaluations", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getPerformPowerEvaluations());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEvaluationType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getPowerEvaluationType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEvaluationTotalCases", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getPowerEvaluationTotalCases());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEvaluationReplications", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getPowerEvaluationReplications());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEvaluationAltHypothesisFilename", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(parameters.getPowerEvaluationAltHypothesisFilename().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setReportCriticalValues", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getReportCriticalValues());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPerformDayOfWeekAdjustment", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getPerformDayOfWeekAdjustment());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setReportAttributableRisk", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getReportAttributableRisk());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setAttributableRiskExposed", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getAttributableRiskExposed());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSelfControlDesign", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getSelfControlDesign());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "clearInputSourceSettings", "()V");
  Env.CallVoidMethod(jParameters, mid);
  jni_error::_detectError(Env);

  jmethodID mid_add_source = _getMethodId_Checked(Env, clazz, "addInputSourceSettings", "(Lorg/treescan/importer/InputSourceSettings;)V");
  jclass issclazz = Env.FindClass("org/treescan/importer/InputSourceSettings");
  jmethodID mid_constructor = _getMethodId_Checked(Env, issclazz, "<init>", "()V");
  Parameters::InputSourceContainer_t::const_iterator itr=parameters.getInputSources().begin();
  for (; itr != parameters.getInputSources().end(); ++itr) {
      const Parameters::InputSourceKey_t& key = itr->first;
      const Parameters::InputSource& iss = itr->second;
      jobject issobject = Env.NewObject(issclazz, mid_constructor);
      Env.CallVoidMethod(jParameters, mid_add_source, issobject);

      // translate ParameterType to Java InputSourceSettings.InputFileType
      mid = _getMethodId_Checked(Env, issclazz, "setInputFileType", "(I)V");
      switch (key.first) {
        case Parameters::TREE_FILE : Env.CallVoidMethod(issobject, mid, (jint)0); break;
        case Parameters::COUNT_FILE : Env.CallVoidMethod(issobject, mid, (jint)1); break;
        case Parameters::CUT_FILE : Env.CallVoidMethod(issobject, mid, (jint)2); break;
        case Parameters::POWER_EVALUATIONS_FILE : Env.CallVoidMethod(issobject, mid, (jint)3); break;
        case Parameters::CONTROL_FILE: Env.CallVoidMethod(issobject, mid, (jint)4); break;
        default : throw prg_error("Unknown parameter type for translation: %d", "copyCParametersToJParameters()", key.first);
      }

      mid = _getMethodId_Checked(Env, issclazz, "setIndex", "(I)V");
      Env.CallVoidMethod(issobject, mid, (jint)key.second);
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "setSourceDataFileType", "(I)V");
      Env.CallVoidMethod(issobject, mid, (jint)iss.getSourceType());

      mid = _getMethodId_Checked(Env, issclazz, "setDelimiter", "(Ljava/lang/String;)V");
      Env.CallVoidMethod(issobject, mid, Env.NewStringUTF(iss.getDelimiter().c_str()));
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "setGroup", "(Ljava/lang/String;)V");
      Env.CallVoidMethod(issobject, mid, Env.NewStringUTF(iss.getGroup().c_str()));
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "setSkiplines", "(I)V");
      Env.CallVoidMethod(issobject, mid, (jint)iss.getSkip());
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "setFirstRowHeader", "(Z)V");
      Env.CallVoidMethod(issobject, mid, (jboolean)iss.getFirstRowHeader());
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "addFieldMapping", "(Ljava/lang/String;)V");
      FieldMapContainer_t::const_iterator itrMap=iss.getFieldsMap().begin();
      for (;itrMap != iss.getFieldsMap().end(); ++itrMap) {
          std::stringstream s;
          if (itrMap->type() == typeid(long)) {
              long c = boost::any_cast<long>(*itrMap);
              if (c == 0) s << c;
              else s << (boost::any_cast<long>(*itrMap));
          } else {
            throw prg_error("Unknown type '%s'.", "WriteInputSource()", itrMap->type().name());
          }
          Env.CallVoidMethod(issobject, mid, Env.NewStringUTF(s.str().c_str()));
          jni_error::_detectError(Env);
      }
  }

  ratio = parameters.getPowerBaselineProbabilityRatio();
  mid = _getMethodId_Checked(Env, clazz, "setPowerBaselineProbabilityRatioNumerator", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)ratio.first);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerBaselineProbabilityRatioDenominator", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)ratio.second);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRestrictTreeLevels", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getRestrictTreeLevels());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRestrictedTreeLevels", "(Ljava/lang/String;)V");
  std::string s;
  if (!typelist_csv_string<unsigned int>(parameters.getRestrictedTreeLevels(), s)) 
    throw prg_error("Unable to convert tree levels", "copyCParametersToJParameters()");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(s.c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSequentialScan", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.getSequentialScan());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSequentialMinimumSignal", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getSequentialMinimumSignal());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSequentialMaximumSignal", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getSequentialMaximumSignal());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSequentialAlphaOverall", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)parameters.getSequentialAlphaOverall());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSequentialAlphaSpending", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)parameters.getSequentialAlphaSpending());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSequentialFilename", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(parameters.getSequentialFilename().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setApplyingRiskWindowRestriction", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.isApplyingRiskWindowRestriction());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRiskWindowPercentage", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)parameters.getRiskWindowPercentage());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setApplyingExclusionTimeRanges", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)parameters.isApplyingExclusionTimeRanges());
  jni_error::_detectError(Env);

  if (parameters.isApplyingExclusionTimeRanges()) {
      mid = _getMethodId_Checked(Env, clazz, "setExclusionTimeRangeSet", "(Ljava/lang/String;)V");
      parameters.getExclusionTimeRangeSet().toString(s, parameters.getDatePrecisionType());
      Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(s.c_str()));
      jni_error::_detectError(Env);
  }

  return jParameters;
}

/** Copies JParameter object data members to CParameters object. */

Parameters& ParametersUtility::copyJParametersToCParameters(JNIEnv& Env, jobject& jParameters, Parameters& parameters) {
  jboolean              iscopy;
  const char          * sFilename;

  //set CParameter class from jParameters object
  jclass clazz = Env.GetObjectClass(jParameters);

  parameters.setDatePrecisionType((DataTimeRange::DatePrecisionType)getEnumTypeOrdinalIndex(Env, jParameters, "getPrecisionOfTimesType", "Lorg/treescan/app/Parameters$DatePrecisionType;"));

  jmethodID mid = _getMethodId_Checked(Env, clazz, "getNumRequestedParallelProcesses", "()I");
  parameters.setNumProcesses(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getNumReplicationsRequested", "()I");
  parameters.setNumReplications(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getCountFileName", "()Ljava/lang/String;");
  jstring jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  parameters.setCountFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getControlFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  parameters.setControlFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getCutsFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  parameters.setCutsFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getTreeFileNames", "()Ljava/util/ArrayList;");
  jobject vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  jclass vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  jni_error::_detectError(Env);
  jint vsize = Env.CallIntMethod(vectorobject, mid);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      parameters.setTreeFileName(sFilename, false, i + 1);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
  }

  mid = _getMethodId_Checked(Env, clazz, "getOutputFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  parameters.setOutputFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  parameters.setResultsFormat((Parameters::ResultsFormat)getEnumTypeOrdinalIndex(Env, jParameters, "getResultsFormat", "Lorg/treescan/app/Parameters$ResultsFormat;"));

  mid = _getMethodId_Checked(Env, clazz, "getSourceFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  parameters.setSourceFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "isRandomlyGeneratingSeed", "()Z");
  parameters.setRandomlyGeneratingSeed(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRandomizationSeed", "()I");
  parameters.setRandomizationSeed(static_cast<long>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isGeneratingHtmlResults", "()Z");
  parameters.setGeneratingHtmlResults(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isGeneratingTableResults", "()Z");
  parameters.setGeneratingTableResults(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isPrintColumnHeaders", "()Z");
  parameters.setPrintColumnHeaders(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  parameters.setModelType((Parameters::ModelType)getEnumTypeOrdinalIndex(Env, jParameters, "getModelType", "Lorg/treescan/app/Parameters$ModelType;"));

  mid = _getMethodId_Checked(Env, clazz, "getIsProspectiveAnalysis", "()Z");
  parameters.setIsProspectiveAnalysis(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  Parameters::ratio_t ratio;
  mid = _getMethodId_Checked(Env, clazz, "getProbabilityRatioNumerator", "()I");
  ratio.first = static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "getProbabilityRatioDenominator", "()I");
  ratio.second = static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);
  parameters.setProbabilityRatio(ratio);

  parameters.setScanType((Parameters::ScanType)getEnumTypeOrdinalIndex(Env, jParameters, "getScanType", "Lorg/treescan/app/Parameters$ScanType;"));
  parameters.setConditionalType((Parameters::ConditionalType)getEnumTypeOrdinalIndex(Env, jParameters, "getConditionalType", "Lorg/treescan/app/Parameters$ConditionalType;"));

  mid = _getMethodId_Checked(Env, clazz, "getRestrictTemporalWindows", "()Z");
  parameters.setRestrictTemporalWindows(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  if (Parameters::isTemporalScanType(parameters.getScanType())) {
    std::stringstream buffer;
    mid = _getMethodId_Checked(Env, clazz, "getDataTimeRangeBegin", "()Ljava/lang/String;");
    jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
    jni_error::_detectError(Env);
    sFilename = Env.GetStringUTFChars(jstr, &iscopy);
    buffer << "[" << sFilename;
    if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

    mid = _getMethodId_Checked(Env, clazz, "getDataTimeRangeClose", "()Ljava/lang/String;");
    jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
    jni_error::_detectError(Env);
    sFilename = Env.GetStringUTFChars(jstr, &iscopy);
    buffer << "," << sFilename << "]";
    if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);
    parameters.setDataTimeRangeSet(DataTimeRangeSet(buffer.str(), parameters.getDatePrecisionType(), boost::optional<boost::gregorian::date>()));

    if (parameters.getRestrictTemporalWindows()) {
        buffer.str("");
        mid = _getMethodId_Checked(Env, clazz, "getStartRangeStartDate", "()Ljava/lang/String;");
        jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
        jni_error::_detectError(Env);
        sFilename = Env.GetStringUTFChars(jstr, &iscopy);
        buffer << "[" << sFilename;
        if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);
        mid = _getMethodId_Checked(Env, clazz, "getStartRangeEndDate", "()Ljava/lang/String;");
        jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
        jni_error::_detectError(Env);
        sFilename = Env.GetStringUTFChars(jstr, &iscopy);
        buffer << "," << sFilename << "]";
        if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);
        parameters.setTemporalStartRange(
            DataTimeRange(buffer.str(), parameters.getDatePrecisionType(), parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart())
        );

        buffer.str("");
        mid = _getMethodId_Checked(Env, clazz, "getEndRangeStartDate", "()Ljava/lang/String;");
        jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
        jni_error::_detectError(Env);
        sFilename = Env.GetStringUTFChars(jstr, &iscopy);
        buffer << "[" << sFilename;
        if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);
        mid = _getMethodId_Checked(Env, clazz, "getEndRangeEndDate", "()Ljava/lang/String;");
        jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
        jni_error::_detectError(Env);
        sFilename = Env.GetStringUTFChars(jstr, &iscopy);
        buffer << "," << sFilename << "]";
        if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);
        parameters.setTemporalEndRange(
            DataTimeRange(buffer.str(), parameters.getDatePrecisionType(), parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart())
        );
    } else {
        parameters.setTemporalStartRange(parameters.getDataTimeRangeSet().getDataTimeRangeSets().front());
        parameters.setTemporalEndRange(parameters.getDataTimeRangeSet().getDataTimeRangeSets().front());
    }
  }

  mid = _getMethodId_Checked(Env, clazz, "isGeneratingLLRResults", "()Z");
  parameters.setGeneratingLLRResults(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getMaximumWindowPercentage", "()D");
  parameters.setMaximumWindowPercentage(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getMaximumWindowLength", "()I");
  parameters.setMaximumWindowLength(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  parameters.setMaximumWindowType((Parameters::MaximumWindowType)getEnumTypeOrdinalIndex(Env, jParameters, "getMaximumWindowType", "Lorg/treescan/app/Parameters$MaximumWindowType;"));

  mid = _getMethodId_Checked(Env, clazz, "getMinimumWindowLength", "()I");
  parameters.setMinimumWindowLength(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getPerformPowerEvaluations", "()Z");
  parameters.setPerformPowerEvaluations(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  parameters.setPowerEvaluationType((Parameters::PowerEvaluationType)getEnumTypeOrdinalIndex(Env, jParameters, "getPowerEvaluationType", "Lorg/treescan/app/Parameters$PowerEvaluationType;"));

  mid = _getMethodId_Checked(Env, clazz, "getPowerEvaluationTotalCases", "()I");
  parameters.setPowerEvaluationTotalCases(static_cast<int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getPowerEvaluationReplications", "()I");
  parameters.setPowerEvaluationReplications(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getPowerEvaluationAltHypothesisFilename", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  parameters.setPowerEvaluationAltHypothesisFilename(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getReportCriticalValues", "()Z");
  parameters.setReportCriticalValues(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getPerformDayOfWeekAdjustment", "()Z");
  parameters.setPerformDayOfWeekAdjustment(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getReportAttributableRisk", "()Z");
  parameters.setReportAttributableRisk(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getAttributableRiskExposed", "()I");
  parameters.setAttributableRiskExposed(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getSelfControlDesign", "()Z");
  parameters.setSelfControlDesign(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getInputSourceSettings", "()Ljava/util/ArrayList;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jobject iss_object = (jobject)Env.CallObjectMethod(vectorobject, mid, i);
      jclass issclazz = Env.GetObjectClass(iss_object);

      Parameters::InputSource inputsource;
      inputsource.setSourceType((SourceType)getEnumTypeOrdinalIndex(Env, iss_object, "getSourceDataFileType", "Lorg/treescan/importer/InputSourceSettings$SourceDataFileType;"));

      mid = _getMethodId_Checked(Env, issclazz, "getFieldMaps", "()Ljava/util/ArrayList;");
      jobject vectorobject_mappings = Env.CallObjectMethod(iss_object, mid);
      jni_error::_detectError(Env);
      jclass vclazz_mappings = Env.GetObjectClass(vectorobject_mappings);
      mid = _getMethodId_Checked(Env, vclazz_mappings, "size", "()I");
      std::vector<boost::any> map;
      jint vsize_mappings = Env.CallIntMethod(vectorobject_mappings, mid);
      for (jint j=0; j < vsize_mappings; ++j) {
        mid = _getMethodId_Checked(Env, vclazz_mappings, "get", "(I)Ljava/lang/Object;");
        jstring str_object = (jstring)Env.CallObjectMethod(vectorobject_mappings, mid, j);
        jni_error::_detectError(Env);
        sFilename = Env.GetStringUTFChars(str_object, &iscopy);
        std::string buffer(sFilename);
        if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
        int column;
        if (!string_to_type<int>(buffer.c_str(), column))
            throw prg_error("Unable to read parameter value '%s' as mapping item.", buffer.c_str());
            // The field mappings will be a collection of integers. The position of element is relative to the input fields order.
        map.push_back((long)column);
      }
      inputsource.setFieldsMap(map);

      mid = _getMethodId_Checked(Env, issclazz, "getDelimiter", "()Ljava/lang/String;");
      jstr = (jstring)Env.CallObjectMethod(iss_object, mid);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(jstr, &iscopy);
      inputsource.setDelimiter(std::string(sFilename));
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

      mid = _getMethodId_Checked(Env, issclazz, "getGroup", "()Ljava/lang/String;");
      jstr = (jstring)Env.CallObjectMethod(iss_object, mid);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(jstr, &iscopy);
      inputsource.setGroup(std::string(sFilename));
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

      mid = _getMethodId_Checked(Env, issclazz, "getSkiplines", "()I");
      inputsource.setSkip(Env.CallIntMethod(iss_object, mid));
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "getFirstRowHeader", "()Z");
      inputsource.setFirstRowHeader(Env.CallBooleanMethod(iss_object, mid));
      jni_error::_detectError(Env);

      unsigned int idx;
      mid = _getMethodId_Checked(Env, issclazz, "getIndex", "()I");
      idx = static_cast<unsigned int>(Env.CallIntMethod(iss_object, mid));
      jni_error::_detectError(Env);

      /* Translate Java class InputSourceSettings.InputFileType into ParameterType.
        {Case=0, Control, Population, Coordinates, SpecialGrid, MaxCirclePopulation, AdjustmentsByRR}
      */
      Parameters::ParameterType type=Parameters::TREE_FILE;
      int filetype = getEnumTypeOrdinalIndex(Env, iss_object, "getInputFileType", "Lorg/treescan/importer/InputSourceSettings$InputFileType;");
      switch (filetype) {
        case 0/*Tree*/         : type = Parameters::TREE_FILE; break;
        case 1/*Count*/        : type = Parameters::COUNT_FILE; break;
        case 2/*Cuts*/         : type = Parameters::CUT_FILE; break;
        case 3/*Powers Evals*/ : type = Parameters::POWER_EVALUATIONS_FILE; break;
        case 4/*Control*/: type = Parameters::CONTROL_FILE; break;
        default : throw prg_error("Unknown filetype for translation: %d", "copyJParametersToCParameters()", filetype);
      }
      parameters.defineInputSource(type, inputsource, idx);
  }

  mid = _getMethodId_Checked(Env, clazz, "getPowerBaselineProbabilityRatioNumerator", "()I");
  ratio.first = static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "getPowerBaselineProbabilityRatioDenominator", "()I");
  ratio.second = static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);
  parameters.setPowerBaselineProbabilityRatio(ratio);

  mid = _getMethodId_Checked(Env, clazz, "getRestrictTreeLevels", "()Z");
  parameters.setRestrictTreeLevels(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRestrictedTreeLevels", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters::RestrictTreeLevels_t list;
  if (!csv_string_to_typelist<unsigned int>(sFilename, list)) {
    if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);
    throw prg_error("Unable to convert tree levels", "copyJParametersToCParameters()");
  }
  parameters.setRestrictedTreeLevels(list);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getSequentialScan", "()Z");
  parameters.setSequentialScan(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getSequentialAlphaOverall", "()D");
  parameters.setSequentialAlphaOverall(static_cast<double>(Env.CallDoubleMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getSequentialAlphaSpending", "()D");
  parameters.setSequentialAlphaSpending(static_cast<double>(Env.CallDoubleMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getSequentialMinimumSignal", "()I");
  parameters.setSequentialMinimumSignal(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setSequentialMaximumSignal", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)parameters.getSequentialMaximumSignal());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getSequentialFilename", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  parameters.setSequentialFilename(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "isApplyingRiskWindowRestriction", "()Z");
  parameters.setApplyingRiskWindowRestriction(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRiskWindowPercentage", "()D");
  parameters.setRiskWindowPercentage(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "isApplyingExclusionTimeRanges", "()Z");
  parameters.setApplyingExclusionTimeRanges(static_cast<bool>(Env.CallBooleanMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  if (parameters.isApplyingExclusionTimeRanges()) {
      mid = _getMethodId_Checked(Env, clazz, "getExclusionTimeRangeSet", "()Ljava/lang/String;");
      jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(jstr, &iscopy);
      parameters.setExclusionTimeRangeSet(DataTimeRangeSet(
          std::string(sFilename), parameters.getDatePrecisionType(), parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart()
      ));
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);
  }

  return parameters;
}
