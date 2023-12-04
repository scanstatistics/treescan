//******************************************************************************
#ifndef FieldDefH
#define FieldDefH
//******************************************************************************
#include "TimeStamp.h"

class FieldValue {
  public:
    static const char ALPHA_FLD       = 'A';
    static const char LONG_FLD        = 'I';
    static const char ULONG_FLD       = 'W';
    static const char BOOLEAN_FLD     = 'L';
    static const char NUMBER_FLD      = 'N';
    static const char SHORT_FLD       = 'S';
    static const char USHORT_FLD      = 'U';
    static const char DATE_FLD        = 'D';
    static const char STAMP_FLD       = '@';
    static const char TIME_FLD        = 'T';
    //static const char BINARY_FLD      = 'Y';

  protected://typedefs
   typedef union
      {
      bool               b;
      double             d;
      long               l;
      unsigned long      ul;
      short              w;
      unsigned short     uw;
      std::string        *  pString;
      TreeScan::Date      * pDate;
      TreeScan::Time      * pTime;
      TreeScan::Timestamp *  pTimestamp;
      } val_t;

   typedef bool & (FieldValue::* BoolGetter_t) ();
   typedef const char * (FieldValue::* CStringGetter_t) ();
   typedef std::string & (FieldValue::* StringGetter_t) ();
   typedef double & (FieldValue::* DoubleGetter_t) ();
   typedef TreeScan::Date & (FieldValue::* DateGetter_t) ();
   typedef TreeScan::Time & (FieldValue::* TimeGetter_t) ();
   typedef TreeScan::Timestamp & (FieldValue::* TimestampGetter_t) ();
   typedef long & (FieldValue::* LongGetter_t) ();
   typedef unsigned long & (FieldValue::* UnsignedLongGetter_t) ();
   typedef short & (FieldValue::* ShortGetter_t) ();
   typedef unsigned short & (FieldValue::* UnsignedShortGetter_t) ();

  private:
   void Init();
   void Setup(char cType, bool bIsTypeEnforced);

   void Copy(const FieldValue & rhs);

  protected:
   FieldValue::val_t gValue;
   char  gcType;

   BoolGetter_t gBoolGetter;
   CStringGetter_t gCStringGetter;
   StringGetter_t gStringGetter;
   DoubleGetter_t gDoubleGetter;
   DateGetter_t gDateGetter;
   TimeGetter_t gTimeGetter;
   TimestampGetter_t gTimestampGetter;
   LongGetter_t gLongGetter;
   UnsignedLongGetter_t gUnsignedLongGetter;
   ShortGetter_t gShortGetter;
   UnsignedShortGetter_t gUnsignedShortGetter;

   virtual bool & AsBool_TypeEnforced();
   virtual bool & AsBool_TypeUnenforced();
   virtual const char * AsCString_TypeEnforced();
   virtual const char * AsCString_TypeUnenforced();
   virtual std::string & AsString_TypeEnforced();
   virtual std::string & AsString_TypeUnenforced();
   virtual double & AsDouble_TypeEnforced();
   virtual double & AsDouble_TypeUnenforced();
   virtual TreeScan::Date & AsDate_TypeEnforced();
   virtual TreeScan::Date & AsDate_TypeUnenforced();
   virtual TreeScan::Time & AsTime_TypeEnforced();
   virtual TreeScan::Time & AsTime_TypeUnenforced();
   virtual TreeScan::Timestamp & AsTimestamp_TypeEnforced();
   virtual TreeScan::Timestamp & AsTimestamp_TypeUnenforced();
   virtual long & AsLong_TypeEnforced();
   virtual long & AsLong_TypeUnenforced();
   virtual unsigned long & AsUnsignedLong_TypeEnforced();
   virtual unsigned long & AsUnsignedLong_TypeUnenforced();
   virtual short & AsShort_TypeEnforced();
   virtual short & AsShort_TypeUnenforced();
   virtual unsigned short & AsUnsignedShort_TypeEnforced();
   virtual unsigned short & AsUnsignedShort_TypeUnenforced();

   virtual void ReclaimDataValue();
   virtual void AllocateDataValueAsType(char cType);
   virtual void AllocateDataValueAsClone(const FieldValue & rhs);

  public:
   FieldValue(char cType = ALPHA_FLD, bool bIsTypeEnforced = true);
   FieldValue(const FieldValue & rhs);
   virtual ~FieldValue();

   inline virtual FieldValue * Clone() const                                 { return new FieldValue(*this); }

   // assignment operator
   inline FieldValue & operator= (const FieldValue & rhs)                  { Copy(rhs); return *this; }
   // comparison
   virtual bool IsLessThan(const FieldValue & rhs) const;
   // equality
   virtual bool IsEqualTo(const FieldValue & rhs) const { return (!IsLessThan(rhs)) && (!rhs.IsLessThan(*this)); }
   virtual int  ComparedTo(const FieldValue & rhs) const;

   virtual char GetType() const;
   virtual void SetType(char cType);
   virtual bool GetIsValidTypeIndicator(char cTypeChar);

   inline virtual       bool & AsBool()                                        { return (this->*gBoolGetter)(); }
   inline virtual const bool & AsBool() const                                  { return (const_cast<FieldValue *>(this)->*gBoolGetter)(); }
   inline virtual const char * AsCString()                                     { return (const_cast<FieldValue *>(this)->*gCStringGetter)(); }
   inline virtual       std::string & AsString()                               { return (this->*gStringGetter)(); }
   inline virtual const std::string & AsString() const                         { return (const_cast<FieldValue *>(this)->*gStringGetter)(); }
   inline virtual       double & AsDouble()                                    { return (this->*gDoubleGetter)(); }
   inline virtual const double & AsDouble() const                              { return (const_cast<FieldValue *>(this)->*gDoubleGetter)(); }
   inline virtual       TreeScan::Date & AsDate()                               { return (this->*gDateGetter)(); }
   inline virtual const TreeScan::Date & AsDate() const                         { return (const_cast<FieldValue *>(this)->*gDateGetter)(); }
   inline virtual       TreeScan::Time & AsTime()                               { return (this->*gTimeGetter)(); }
   inline virtual const TreeScan::Time & AsTime() const                         { return (const_cast<FieldValue *>(this)->*gTimeGetter)(); }
   inline virtual       TreeScan::Timestamp & AsTimestamp()                     { return (this->*gTimestampGetter)(); }
   inline virtual const TreeScan::Timestamp & AsTimestamp() const               { return (const_cast<FieldValue *>(this)->*gTimestampGetter)(); }
   inline virtual       long & AsLong()                                        { return (this->*gLongGetter)(); }
   inline virtual const long & AsLong() const                                  { return (const_cast<FieldValue *>(this)->*gLongGetter)(); }
   inline virtual       unsigned long & AsUnsignedLong()                       { return (this->*gUnsignedLongGetter)(); }
   inline virtual const unsigned long & AsUnsignedLong() const                 { return (const_cast<FieldValue *>(this)->*gUnsignedLongGetter)(); }
   inline virtual       short & AsShort()                                      { return (this->*gShortGetter)(); }
   inline virtual const short & AsShort() const                                { return (const_cast<FieldValue *>(this)->*gShortGetter)(); }
   inline virtual       unsigned short & AsUnsignedShort()                     { return (this->*gUnsignedShortGetter)(); }
   inline virtual const unsigned short & AsUnsignedShort() const               { return (const_cast<FieldValue *>(this)->*gUnsignedShortGetter)(); }

   virtual bool GetIsTypeEnforced() const;
   virtual void SetIsTypeEnforced(bool b);

   static const char  * GetTypeCString(char cType);
};


// FieldDef class
class FieldDef {
  protected:
    char              gcType;                              // Field Type
    std::string       gsName;                              // Field Name
    unsigned short    gwLength;                            // Field Length
    short             gwPrecision;                         // Field Precision
    unsigned short    gwOffset;                            // Start position in record buffer for this field.
    unsigned short    gwAsciiDecimals;                     // Number of significant decimals.

  public:
   FieldDef(const char * sName, char cType, short wLength, short wPrecision, unsigned short wOffset, unsigned short wAsciiDecimals=2);
   virtual ~FieldDef() {}

   static inline char TypeOf ( bool )                       { return FieldValue::BOOLEAN_FLD; }
   static inline char TypeOf ( const char * )               { return FieldValue::ALPHA_FLD; }
   static inline char TypeOf ( double )                     { return FieldValue::NUMBER_FLD; }
   static inline char TypeOf ( long )                       { return FieldValue::LONG_FLD; }
   static inline char TypeOf ( unsigned long )              { return FieldValue::ULONG_FLD; }
   static inline char TypeOf ( short )                      { return FieldValue::SHORT_FLD; }
   static inline char TypeOf ( unsigned short )             { return FieldValue::USHORT_FLD; }
   static inline char TypeOf ( const TreeScan::Date & )      { return FieldValue::DATE_FLD; }
   static inline char TypeOf ( const TreeScan::Timestamp & ) { return FieldValue::STAMP_FLD; }
   static inline char TypeOf ( const TreeScan::Time & )      { return FieldValue::TIME_FLD; }
   
   unsigned short       GetAsciiDecimals() const {return gwAsciiDecimals;}
   short                GetDataLength() const;
   short                GetLength() const {return gwLength;}
   const char  *        GetName() const {return gsName.c_str();}
   unsigned short       GetOffset() const {return gwOffset;}
   short                GetPrecision() const {return gwPrecision;}
   const char           GetType() const {return gcType;}
   virtual FieldDef *   Clone() const {return new FieldDef(*this);}
};
//******************************************************************************
#endif
