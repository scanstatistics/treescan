//******************************************************************************
#ifndef stsParametersUtilityH
#define stsParametersUtilityH
//******************************************************************************
#include <jni.h>
#include "org_treescan_app_Parameters.h"

#include "Parameters.h"

class ParametersUtility {
  public:
    static jobject & copyCParametersToJParameters(JNIEnv& Env, Parameters& Parameters, jobject& jParameters);
    static Parameters & copyJParametersToCParameters(JNIEnv& Env, jobject& jParameters, Parameters& Parameters);
    static int getEnumTypeOrdinalIndex(JNIEnv& Env, jobject& jParameters, const char * sFunctionName, const char * sEnumClassSignature);
};

//******************************************************************************
#endif
