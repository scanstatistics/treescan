//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "FieldDef.h"
#include "PrjException.h"

//constructor
FieldValue::FieldValue(char cType, bool bIsTypeEnforced) {
  Init();
  Setup(cType, bIsTypeEnforced);
}

//copy constructor
FieldValue::FieldValue(const FieldValue & rhs) {
  Init();
  Copy(rhs);
}

//destructor
FieldValue::~FieldValue() {
   ReclaimDataValue();
}

//Initialize members of basic types.
void FieldValue::Init() {
   gcType = LONG_FLD;
   SetIsTypeEnforced(true);
}

//Initialize *this.
void FieldValue::Setup(char cType, bool bIsTypeEnforced) {
  SetIsTypeEnforced(bIsTypeEnforced);
  SetType(cType);
}

//Going on the assumption that gValue is not pointing to dynamically allocated space,
//setup gValue so that it holds the default value for cType and set gcType to reflect
//the new type.
void FieldValue::AllocateDataValueAsType(char cType) {
  //setup new type data -- maybe use an array of types and Clone(), eventually
  switch (cType) {
    case ALPHA_FLD     : gValue.pString = new std::string(); break;
    case DATE_FLD      : gValue.pDate = new TreeScan::Date(); break;
    case LONG_FLD      : gValue.l = 0; break;
    case ULONG_FLD     : gValue.ul = 0; break;
    case BOOLEAN_FLD   : gValue.b = false; break;
    case NUMBER_FLD    : gValue.d = 0.0; break;
    case SHORT_FLD     : gValue.w = 0; break;
    case STAMP_FLD     : gValue.pTimestamp = new TreeScan::Timestamp(); break;
    case TIME_FLD      : gValue.pTime = new TreeScan::Time(); break;
    case USHORT_FLD   : gValue.uw = 0; break;
  }
  gcType = cType;
}

//Going on the assumption that gValue is not pointing to dynamically allocated space,
//setup gValue so that it holds a copy of what rhs holds and set gcType to reflect
//the new type.
void FieldValue::AllocateDataValueAsClone(const FieldValue & rhs) {
   //setup new type data
   switch (rhs.GetType()) {
      case ALPHA_FLD   : gValue.pString = new std::string(rhs.AsString()); break;
      case DATE_FLD    : gValue.pDate = rhs.AsDate().Clone(); break;
      case LONG_FLD    : gValue.l = rhs.AsLong(); break;
      case ULONG_FLD   : gValue.ul = rhs.AsUnsignedLong(); break;
      case BOOLEAN_FLD : gValue.b = rhs.AsBool(); break;
      case NUMBER_FLD  : gValue.d = rhs.AsDouble(); break;
      case SHORT_FLD   : gValue.w = rhs.AsShort(); break;
      case STAMP_FLD   : gValue.pTimestamp = rhs.AsTimestamp().Clone(); break;
      case TIME_FLD    : gValue.pTime = rhs.AsTime().Clone(); break;
      case USHORT_FLD  : gValue.uw = rhs.AsUnsignedShort(); break;
   }
   gcType = rhs.GetType();
}

//Treat the value as a BOOLEAN.
bool & FieldValue::AsBool_TypeEnforced() {
  if (GetType() != BOOLEAN_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsBool_TypeEnforced",
                    GetTypeCString(GetType()), GetTypeCString(BOOLEAN_FLD));
  return gValue.b;
}

//Treat the value as a BOOLEAN.
bool & FieldValue::AsBool_TypeUnenforced() {
   return gValue.b;
}

//Treat the value as an ALPHA.
const char * FieldValue::AsCString_TypeEnforced() {
  if (GetType() != ALPHA_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "FieldValue",
                    GetTypeCString(GetType()), GetTypeCString(ALPHA_FLD));
  return gValue.pString->c_str();
}

//Treat the value as an ALPHA.
const char * FieldValue::AsCString_TypeUnenforced() {
   return gValue.pString->c_str();
}

//Treat the value as a NUMBER.
double & FieldValue::AsDouble_TypeEnforced() {
  if (GetType() != NUMBER_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsDouble_TypeEnforced",
                    GetTypeCString(GetType()), GetTypeCString(NUMBER_FLD));
  return gValue.d;
}

//Treat the value as a NUMBER.
double & FieldValue::AsDouble_TypeUnenforced() {
   return gValue.d;
}

//Treat the value as a LONG.
long & FieldValue::AsLong_TypeEnforced() {
  if (GetType() != LONG_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsLong_TypeEnforced",
                    GetTypeCString(GetType()), GetTypeCString(LONG_FLD));
  return gValue.l;
}

//Treat the value as a LONG.
long & FieldValue::AsLong_TypeUnenforced() {
   return gValue.l;
}

//Treat the value as a SHORT.
short & FieldValue::AsShort_TypeEnforced() {
  if (GetType() != SHORT_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsShort_TypeEnforced",
                    GetTypeCString(GetType()), GetTypeCString(SHORT_FLD));
  return gValue.w;
}

//Treat the value as a SHORT.
short & FieldValue::AsShort_TypeUnenforced() {
  return gValue.w;
}

//Treat the value as a ULONG.
unsigned long & FieldValue::AsUnsignedLong_TypeEnforced() {
  if (GetType() != ULONG_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsUnsignedLong_TypeEnforced",
                    GetTypeCString(GetType()), GetTypeCString(ULONG_FLD));
  return gValue.ul;
}

//Treat the value as a ULONG.
unsigned long & FieldValue::AsUnsignedLong_TypeUnenforced() {
   return gValue.ul;
}

//Treat the value as a USHORT.
unsigned short & FieldValue::AsUnsignedShort_TypeEnforced() {
   if (GetType() != USHORT_FLD)
     throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsUnsignedShort_TypeEnforced",
                     GetTypeCString(GetType()), GetTypeCString(USHORT_FLD));
   return gValue.uw;
}

//Treat the value as a USHORT.
unsigned short & FieldValue::AsUnsignedShort_TypeUnenforced() {
   return gValue.uw;
}


//Treat the value as a DATE.
TreeScan::Date & FieldValue::AsDate_TypeEnforced() {
   if (GetType() != DATE_FLD)
      prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsZdDate_TypeEnforced", GetTypeCString(GetType()), GetTypeCString(DATE_FLD));
   return *(gValue.pDate);
}

//Treat the value as a DATE.
TreeScan::Date & FieldValue::AsDate_TypeUnenforced() {
   return *(gValue.pDate);
}
//Treat the value as an ALPHA.
std::string & FieldValue::AsString_TypeEnforced() {
  if (GetType() != ALPHA_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsString_TypeEnforced",
                    GetTypeCString(GetType()), GetTypeCString(ALPHA_FLD));
  return *(gValue.pString);
}

//Treat the value as an ALPHA.
std::string & FieldValue::AsString_TypeUnenforced()  {
   return *(gValue.pString);
}

//Treat the value as a TIME.
TreeScan::Time & FieldValue::AsTime_TypeEnforced() {
  if (GetType() != TIME_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsTime_TypeEnforced", GetTypeCString(GetType()), GetTypeCString(TIME_FLD));
   return *(gValue.pTime);
}

//Treat the value as a TIME.
TreeScan::Time & FieldValue::AsTime_TypeUnenforced() {
   return *(gValue.pTime);
}

//Treat the value as a STAMP.
TreeScan::Timestamp & FieldValue::AsTimestamp_TypeEnforced() {
  if (GetType() != STAMP_FLD)
    throw prg_error("Cannot treat a field value of type \"%s\" as type \"%s\".", "AsTimestamp_TypeEnforced", GetTypeCString(GetType()), GetTypeCString(STAMP_FLD));
   return *(gValue.pTimestamp);
}

//Treat the value as a STAMP.
TreeScan::Timestamp & FieldValue::AsTimestamp_TypeUnenforced()  {
   return *(gValue.pTimestamp);
}

//How does '*this' compare to 'rhs' ?
int FieldValue::ComparedTo(const FieldValue & rhs) const {
  return IsLessThan(rhs) ? -1 : ( rhs.IsLessThan(*this) ? 1 : 0 );
}

//Ensure that *this is equal to rhs.
//This does not copy the TypeEnforced property, only the value.
void FieldValue::Copy(const FieldValue & rhs) {
  if (GetType() != rhs.GetType()) {
    ReclaimDataValue();
    AllocateDataValueAsClone(rhs);
  }
  else {//types are equal, so a simple assignment suffices
    switch (GetType()) {
      case BOOLEAN_FLD  : AsBool() = rhs.AsBool(); break;
      case ALPHA_FLD    : AsString() = rhs.AsString(); break;
      case NUMBER_FLD   : AsDouble() = rhs.AsDouble(); break;
      case DATE_FLD     : AsDate() = rhs.AsDate(); break;
      case TIME_FLD     : AsTime() = rhs.AsTime(); break;
      case STAMP_FLD    : AsTimestamp() = rhs.AsTimestamp(); break;
      case LONG_FLD     : AsLong() = rhs.AsLong(); break;
      case ULONG_FLD    : AsUnsignedLong() = rhs.AsUnsignedLong(); break;
      case SHORT_FLD    : AsShort() = rhs.AsShort(); break;
      case USHORT_FLD   : AsUnsignedShort() = rhs.AsUnsignedShort(); break;
      default : throw prg_error("The character, \'%c\', is not a valid field type.", "Copy", GetType());
    }
  }
}

//Will calls to As... functions throw an exception if the type isn't appropriate ?
bool FieldValue::GetIsTypeEnforced() const {
   return gBoolGetter == &FieldValue::AsBool_TypeEnforced;
}

//Does 'cTypeChar' indicate a valid type ?
bool FieldValue::GetIsValidTypeIndicator(char cTypeChar) {
  switch (cTypeChar) {
    case ALPHA_FLD   :
    case DATE_FLD    :
    case LONG_FLD    :
    case ULONG_FLD   :
    case BOOLEAN_FLD :
    case NUMBER_FLD  :
    case SHORT_FLD   :
    case STAMP_FLD:
    case TIME_FLD:
    case USHORT_FLD  : return true;
    default: return false;
  }
}

//What is the type of the value ?
char FieldValue::GetType() const {
  return gcType;
}

//Is '*this' less than 'rhs' ?
bool FieldValue::IsLessThan(const FieldValue & rhs) const {
  if (!(GetType() == rhs.GetType()))
     throw prg_error("Cannot compare FieldValue of type %s to FieldValue of type %s.", "FieldValue",
                     GetTypeCString(GetType()), GetTypeCString(rhs.GetType()));
  //field_type_is_valid precondition checked in switch statement
  switch (GetType()) {
    case ALPHA_FLD   : return AsString() < rhs.AsString();
    case DATE_FLD    : return AsDate() < rhs.AsDate();
    case LONG_FLD    : return AsLong() < rhs.AsLong();
    case ULONG_FLD   : return AsUnsignedLong() < rhs.AsUnsignedLong();
    case BOOLEAN_FLD : return AsBool() < rhs.AsBool();
    case NUMBER_FLD  : return AsDouble() < rhs.AsDouble();
    case SHORT_FLD   : return AsShort() < rhs.AsShort();
    case STAMP_FLD   : return AsTimestamp() < rhs.AsTimestamp();
    case TIME_FLD    : return AsTime() < rhs.AsTime();
    case USHORT_FLD  : return AsUnsignedShort() < rhs.AsUnsignedShort();
    default : return false;
  }
}

//Reclaim *gValue if it is of a dynamically allocated type; then set all the bytes
//in gValue to 0.
void FieldValue::ReclaimDataValue() {
  switch (GetType()) {
    case ALPHA_FLD   : delete gValue.pString; break;
    case DATE_FLD    : delete gValue.pDate; break;
    case LONG_FLD    : break;
    case ULONG_FLD   : break;
    case BOOLEAN_FLD : break;
    case NUMBER_FLD  : break;
    case SHORT_FLD   : break;
    case STAMP_FLD   : delete gValue.pTimestamp; break;
    case TIME_FLD    : delete gValue.pTime; break;
    case USHORT_FLD  : break;
    default : throw prg_error("The character, \'%c\', is not a valid field type.", "ReclaimDataValue", GetType());
   }
   ::memset(reinterpret_cast<void*>(&gValue), 0, sizeof(gValue));
}

//Specify whether calls to As... functions will throw an exception if the type isn't
//appropriate.
void FieldValue::SetIsTypeEnforced(bool b) {
  if (b) {
    gBoolGetter             = &FieldValue::AsBool_TypeEnforced;
    gCStringGetter          = &FieldValue::AsCString_TypeEnforced;
    gStringGetter           = &FieldValue::AsString_TypeEnforced;
    gDoubleGetter           = &FieldValue::AsDouble_TypeEnforced;
    gDateGetter             = &FieldValue::AsDate_TypeEnforced;
    gTimeGetter             = &FieldValue::AsTime_TypeEnforced;
    gTimestampGetter        = &FieldValue::AsTimestamp_TypeEnforced;
    gLongGetter             = &FieldValue::AsLong_TypeEnforced;
    gUnsignedLongGetter     = &FieldValue::AsUnsignedLong_TypeEnforced;
    gShortGetter            = &FieldValue::AsShort_TypeEnforced;
    gUnsignedShortGetter    = &FieldValue::AsUnsignedShort_TypeEnforced;
  }
  else {
    gBoolGetter             = &FieldValue::AsBool_TypeUnenforced;
    gCStringGetter          = &FieldValue::AsCString_TypeUnenforced;
    gStringGetter           = &FieldValue::AsString_TypeUnenforced;
    gDoubleGetter           = &FieldValue::AsDouble_TypeUnenforced;
    gDateGetter             = &FieldValue::AsDate_TypeUnenforced;
    gTimeGetter             = &FieldValue::AsTime_TypeUnenforced;
    gTimestampGetter        = &FieldValue::AsTimestamp_TypeUnenforced;
    gLongGetter             = &FieldValue::AsLong_TypeUnenforced;
    gUnsignedLongGetter     = &FieldValue::AsUnsignedLong_TypeUnenforced;
    gShortGetter            = &FieldValue::AsShort_TypeUnenforced;
    gUnsignedShortGetter    = &FieldValue::AsUnsignedShort_TypeUnenforced;
  }
}

//Set the value so that it is the default value of the type indicated by 'cNewType',
//or it is unchanged if 'cNewType' == GetType().
void FieldValue::SetType(char cNewType) {
  if ( !GetIsValidTypeIndicator(cNewType) )
    throw prg_error( "The character, \'%c\', is not a valid field type.", "SetType", cNewType );
  ReclaimDataValue();
  AllocateDataValueAsType(cNewType);
}

//Get the field-type identifier that corresponds to 'cType' as a c-style string. 
const char * FieldValue::GetTypeCString(char cType) {
  switch (cType) {
    case FieldValue::ALPHA_FLD   : return "FieldValue::ALPHA_FLD";
    case FieldValue::LONG_FLD    : return "FieldValue::LONG_FLD";
    case FieldValue::ULONG_FLD   : return "FieldValue::ULONG_FLD";
    case FieldValue::BOOLEAN_FLD : return "FieldValue::BOOLEAN_FLD";
    case FieldValue::NUMBER_FLD  : return "FieldValue::NUMBER_FLD";
    case FieldValue::SHORT_FLD   : return "FieldValue::SHORT_FLD";
    case FieldValue::USHORT_FLD  : return "FieldValue::USHORT_FLD";
    case FieldValue::DATE_FLD    : return "FieldValue::DATE_FLD";
    case FieldValue::STAMP_FLD   : return "FieldValue::STAMP_FLD";
    case FieldValue::TIME_FLD    : return "FieldValue::TIME_FLD";
    default: throw prg_error("The character, \'%c\', is not a valid field type.", "GetTypeCString", cType);
  }
}


// This constructor will setup the field for the parameters passed in.
FieldDef::FieldDef(const char * sName, char cType, short wLength, short wPrecision, unsigned short wOffset, unsigned short wAsciiDecimals)
         :gcType(0), gwLength(0), gwPrecision(0), gwOffset(0) {
  gsName = sName;
  gcType = cType;
  if (wLength > 0)
    gwLength = wLength;
  if (wPrecision >= 0)
    gwPrecision = wPrecision;
  gwOffset = wOffset;
  gwAsciiDecimals = wAsciiDecimals;
}

// This function returns the size of the data for the field.
short FieldDef::GetDataLength() const {
  switch (gcType) {
    case FieldValue::NUMBER_FLD  : return sizeof(double);
    case FieldValue::LONG_FLD    : return sizeof(long);
    case FieldValue::ULONG_FLD   : return sizeof(unsigned long);
    case FieldValue::BOOLEAN_FLD : return sizeof(bool);
    case FieldValue::SHORT_FLD   : return sizeof(short);
    case FieldValue::USHORT_FLD  : return sizeof(unsigned short);
    case FieldValue::DATE_FLD    : return TreeScan::Timestamp::DATE_FLD_LEN;
    case FieldValue::TIME_FLD    : return TreeScan::Timestamp::TIME_FLD_LEN;
    case FieldValue::STAMP_FLD   : return TreeScan::Timestamp::STAMP_FLD_LEN;
    default : return GetLength();
  }
}

