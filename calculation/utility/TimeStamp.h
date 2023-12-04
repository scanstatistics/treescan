//******************************************************************************
#ifndef TimeStampH
#define TimeStampH
//******************************************************************************
#include <cmath>
#include <string>
#include <cstring>
#include <cctype>

using namespace TreeScan;

namespace TreeScan {

class Timestamp {
public:
   static const short  STAMP_FLD_LEN         =   17;
   static const short  DATE_FLD_LEN          =    8;
   static const short  TIME_FLD_LEN          =    9;

private:
   static const char mgsDefault[STAMP_FLD_LEN+1];
   static const unsigned short  mguwDays[12];
   static const unsigned short  mguwJulianDaysUpToMonth[12];

   char   gsStamp[STAMP_FLD_LEN+1];

   inline double GetJulianTime() const      { return static_cast<double> ( GetTimeInMilliseconds() ) / 86400000.0; }
   inline void   SetJulianTime ( double d ) { SetTimeInMilliseconds ( static_cast<unsigned long> ( std::fmod ( d, 1.0 ) * 86400000.0 ) ); }

public:
   inline Timestamp();
   virtual ~Timestamp() {}
   inline Timestamp ( const Timestamp &rhs );

   inline virtual Timestamp * Clone() const                  { return new Timestamp(*this); }

   inline Timestamp &operator= (const Timestamp &rhs)      { strcpy ( gsStamp, rhs.gsStamp ); return *this; }

   inline bool     operator==  (const Timestamp &rhs) const  { return ( !memcmp ( gsStamp, rhs.gsStamp, STAMP_FLD_LEN ) ); }

   inline bool     operator!=  (const Timestamp &rhs) const  { return ( memcmp ( gsStamp, rhs.gsStamp, STAMP_FLD_LEN ) != 0 ); }
   inline bool     operator>   (const Timestamp &rhs) const  { return ( memcmp ( gsStamp, rhs.gsStamp, STAMP_FLD_LEN ) > 0 ); }
   inline bool     operator<   (const Timestamp &rhs) const  { return ( memcmp ( gsStamp, rhs.gsStamp, STAMP_FLD_LEN ) < 0 ); };
   inline bool     operator>=  (const Timestamp &rhs) const  { return ( memcmp ( gsStamp, rhs.gsStamp, STAMP_FLD_LEN ) >= 0 ); }
   inline bool     operator<=  (const Timestamp &rhs) const  { return ( memcmp ( gsStamp, rhs.gsStamp, STAMP_FLD_LEN ) <= 0 ); }

   inline int      DateCompare ( const Timestamp &rhs ) const { return memcmp ( gsStamp, rhs.gsStamp, DATE_FLD_LEN ); }
   inline int      TimeCompare ( const Timestamp &rhs ) const { return memcmp ( gsStamp + DATE_FLD_LEN, rhs.gsStamp + DATE_FLD_LEN, TIME_FLD_LEN ); }

   unsigned long   GetJulianDayFromCalendarStart() const;
   void            SetJulianDayFromCalendarStart ( unsigned long ulJulian );

   inline void     AddDays ( long uwAmt )                   { SetJulianDayFromCalendarStart ( GetJulianDayFromCalendarStart() + uwAmt ); }
   void            AddHours ( long uwAmt );
   void            AddMilliseconds ( long uwAmt );
   void            AddMinutes ( long uwAmt );
   void            AddMonths ( long uwAmt );
   void            AddSeconds ( long uwAmt );
   void            AddYears ( long uwAmt );

   inline void Clear()                                         { strcpy ( gsStamp, mgsDefault ); }

   inline void ClearDate()                                     { memcpy ( gsStamp, mgsDefault, DATE_FLD_LEN ); }
   inline void ClearTime()                                     { memcpy ( gsStamp + DATE_FLD_LEN, mgsDefault + DATE_FLD_LEN, TIME_FLD_LEN ); }

   double                 GetFractionalDay() const;
   inline unsigned short  GetDay() const                             { return (unsigned short)((long)gsStamp[6]  * 10 + (long)gsStamp[7]  - 11 * (long)'0'); }
   unsigned short         GetDayOfWeek() const;
   inline unsigned short  GetHour() const                            { return (unsigned short)((long)gsStamp[8]  * 10 + (long)gsStamp[9]  - 11 * (long)'0'); }
   double                 GetJulianDateFromCalendarStart() const;
   double                 GetJulianDate() const;
   inline unsigned short  GetMinute() const                          { return (unsigned short)((long)gsStamp[10] * 10 + (long)gsStamp[11] - 11 * (long)'0'); }
   inline unsigned short  GetMillisecond() const;
   double                 GetModifiedJulianDate() const;
   inline unsigned short  GetMonth() const                           { return (unsigned short)((long)gsStamp[4]  * 10 + (long)gsStamp[5]  - 11 * (long)'0'); }
   inline const char *    GetRawTime() const                         { return gsStamp + DATE_FLD_LEN; }
   inline unsigned short  GetSecond() const                          { return (unsigned short)((long)gsStamp[12] * 10 + (long)gsStamp[13] - 11 * (long)'0'); }
   unsigned long          GetTimeInMilliseconds() const;
   inline unsigned short  GetYear() const;

   void            MakeLocalTime();
   void            MakeUniversalTime();

   void            SetDay(unsigned short wDay);
   void            SetHour(unsigned short uwHour);
   void            SetJulianDate ( double dTime );
   void            SetJulianDateFromCalendarStart ( double dTime );
   void            SetMinute(unsigned short uwMin);
   void            SetMillisecond( unsigned short uwMilli );
   void            SetModifiedJulianDate ( double dTime );
   void            SetMonth(unsigned short wMonth);
   inline void     SetRawDate ( const void *pDate )                           { memcpy ( gsStamp, pDate, DATE_FLD_LEN ); }
   inline void     SetRawTime ( const void *pTime )                           { memcpy ( gsStamp + DATE_FLD_LEN, pTime, TIME_FLD_LEN ); }
   void            SetSecond(unsigned short uwSec);
   void            SetTimeInMilliseconds ( unsigned long ulMilli );
   void            SetYear(unsigned short wYear );
   inline void     StoreDateOnly ( void *sDate ) const                   { memcpy ( sDate, gsStamp, DATE_FLD_LEN ); }
   inline void     StoreTimeOnly ( void *sTime ) const                   { memcpy ( sTime, gsStamp + DATE_FLD_LEN, TIME_FLD_LEN ); }

   inline void     StoreTimestamp ( char *sTimestamp ) const             { memcpy ( sTimestamp, gsStamp, STAMP_FLD_LEN + 1 ); }
   inline void     RetrieveStamp ( std::string & sValue ) const             { sValue = gsStamp;/*gsStamp is stored with trailing NULL character*/ }
   inline void     SetTimestamp ( const char *sTimestamp )               { memcpy ( gsStamp, sTimestamp, STAMP_FLD_LEN ); }
   void            Now( bool bNoMilliseconds = true );
   static Timestamp  Current ( bool bNoMilli = true )                  { Timestamp retVal; retVal.Now ( bNoMilli ); return retVal; }

   static bool     IsValidDate ( const char *sRawDate, bool bAllowZeroMonth = false, bool bAllowZeroDay = false );
   static bool     ValuesIndicateValidDate ( unsigned short uwYear, unsigned short uwMonth, unsigned short uwDay );
   static bool     IsLeapYear(unsigned short uwYear);

   static Timestamp min();
   static Timestamp max();
};

inline bool Timestamp::IsValidDate ( const char *sRawDate, bool bAllowZeroMonth, bool bAllowZeroDay )
{
   bool bRetVal = true;
   int  i;

   for ( i = 0; i < DATE_FLD_LEN && bRetVal; i++ )
      bRetVal = isdigit ( sRawDate[i] ) != 0;

   if ( bRetVal )
      {
      Timestamp  temp;

      temp.SetRawDate ( sRawDate );
      bRetVal  = ( temp.GetMonth() >= (bAllowZeroMonth ? 0 : 1) ) && ( temp.GetDay() >= (bAllowZeroDay ? 0 : 1) ) && ( temp.GetMonth() <= 12 ) && ( temp.GetDay() <= mguwDays[temp.GetMonth()-1] );
      bRetVal &= ( temp.GetMonth() != 2 ) || ( temp.GetDay() < 29 ) || ( Timestamp::IsLeapYear( temp.GetYear() ) );
      }

   return bRetVal;
}

inline bool Timestamp::ValuesIndicateValidDate ( unsigned short uwYear, unsigned short uwMonth, unsigned short uwDay )
{
   bool bRetVal = true;

   bRetVal &= (uwYear <= 9999);
   bRetVal &= (uwMonth <= 12);
   bRetVal &= (uwDay <= 31);

   if ( bRetVal )
      {
      char sRawDate[DATE_FLD_LEN + 1];

      sRawDate[0] = ( uwYear / 1000 ) + '0';
      sRawDate[1] = ( ( uwYear % 1000 ) / 100 ) + '0';
      sRawDate[2] = ( ( uwYear % 100 ) / 10 ) + '0';
      sRawDate[3] = ( uwYear % 10 ) + '0';
      sRawDate[4] = ( uwMonth / 10 ) + '0';
      sRawDate[5] = ( uwMonth % 10 ) + '0';
      sRawDate[6] = ( uwDay / 10 ) + '0';
      sRawDate[7] = ( uwDay % 10 ) + '0';
      sRawDate[DATE_FLD_LEN] = 0; // null terminate

      bRetVal = IsValidDate(sRawDate);
      }

   return bRetVal;
}

/** Every year divisible by 4 is a leap year.
    However, every year divisible by 100 is not a leap year.
    However, every year divisible by 400 is a leap year after all. */
inline bool Timestamp::IsLeapYear(unsigned short uwYear)
{
   return !(uwYear % 4) && ( (uwYear % 100) || !(uwYear %400) );
}

inline Timestamp::Timestamp()
{
   strcpy ( gsStamp, mgsDefault );
}

inline Timestamp::Timestamp ( const Timestamp &rhs )
{
   strcpy ( gsStamp, rhs.gsStamp );
}

/** Returns the milliseconds stored in the time. */
inline unsigned short Timestamp::GetMillisecond() const
{
   return (unsigned short)((long)gsStamp[14] * 100 + (long)gsStamp[15] * 10 + (long)gsStamp[16] - 111 * (long)'0' );
}

/** Returns the year stored in the date. */
inline unsigned short Timestamp::GetYear() const
{
   return (unsigned short)((long)gsStamp[0] * 1000 + (long)gsStamp[1] * 100
                         + (long)gsStamp[2] * 10 + (long)gsStamp[3] - 1111 * (long)'0' );
}

class Date {
  private:
    Timestamp   gStamp; // The date
    mutable char  gsRawDate[Timestamp::DATE_FLD_LEN+1]; // Used by get raw date

  public:

   inline Date()                                        { Clear(); }
   inline explicit Date ( const Timestamp &stamp ) : gStamp ( stamp ) {}
   inline Date ( const Date &rhs ) : gStamp ( rhs.gStamp ) {}
   virtual ~Date() {}

   inline virtual Date * Clone() const                  { return new Date(*this); }

   inline operator Timestamp &()                        { return gStamp; }
   inline operator const Timestamp &() const            { return gStamp; }

   inline Date &operator= (const Date& rhs)           { gStamp = rhs.gStamp; return *this; }
   inline Date &operator= ( const char *sDate )         { SetRawDate ( sDate ); return *this; }

   inline bool     operator==  (const Date& rhs) const  { return !gStamp.DateCompare ( rhs.gStamp ); }

   inline bool     operator!=  (const Date& rhs) const  { return gStamp.DateCompare ( rhs.gStamp ) != 0; }
   inline bool     operator>   (const Date& rhs) const  { return gStamp.DateCompare ( rhs.gStamp ) > 0; }
   inline bool     operator<   (const Date& rhs) const  { return gStamp.DateCompare ( rhs.gStamp ) < 0; };
   inline bool     operator>=  (const Date& rhs) const  { return gStamp.DateCompare ( rhs.gStamp ) >= 0; }
   inline bool     operator<=  (const Date& rhs) const  { return gStamp.DateCompare ( rhs.gStamp ) <= 0; }

   inline void     AddDays ( unsigned short uwAmt )                  { gStamp.AddDays ( uwAmt ); }
   inline void     AddMonths ( unsigned short uwAmt )                { gStamp.AddMonths ( uwAmt ); }
   inline void     AddYears ( unsigned short uwAmt )                 { gStamp.AddYears ( uwAmt ); }

   inline void            Clear()                                               { gStamp.Clear(); }
   inline unsigned short  GetDay() const                                        { return gStamp.GetDay(); }
   inline unsigned short  GetDayOfWeek() const                                  { return gStamp.GetDayOfWeek(); }
   inline unsigned short  GetMonth() const                                      { return gStamp.GetMonth(); }
   inline unsigned short  GetYear() const                                       { return gStamp.GetYear(); }
   inline unsigned long   GetJulianDayFromCalendarStart() const                 { return gStamp.GetJulianDayFromCalendarStart(); }
   inline const char *    GetRawDate() const                                    { gStamp.StoreDateOnly ( gsRawDate ); gsRawDate[Timestamp::DATE_FLD_LEN] = 0; return gsRawDate; }
   inline void            RetrieveRawDate(std::string & sValue) const              { char sRawDate[Timestamp::DATE_FLD_LEN + 1]; gStamp.StoreDateOnly ( sRawDate ); sRawDate[Timestamp::DATE_FLD_LEN] = 0; sValue = sRawDate; }
   void            SetRawDate ( const void *pDate )                             { gStamp.SetRawDate ( pDate ); }
   inline void     SetDay(unsigned short wDay)                                  { gStamp.SetDay ( wDay ); }
   inline void     SetJulianDayFromCalendarStart ( unsigned long ulJulian )     { gStamp.SetJulianDayFromCalendarStart ( ulJulian ); }
   inline void     SetMonth(unsigned short wMonth)                              { gStamp.SetMonth ( wMonth ); }
   inline void     SetYear(unsigned short wYear )                               { gStamp.SetYear ( wYear ); }
   void            StoreDate ( void *sDate ) const                              { gStamp.StoreDateOnly ( sDate ); }
   inline void     Today()                                                      { gStamp.Now(); gStamp.ClearTime(); }
};


class Time {
  private:
    Timestamp gStamp; // The date

  public:
   inline Time()                                        { Clear(); }
   inline explicit Time ( const Timestamp &stamp ) : gStamp ( stamp ) {}
   inline Time ( const Time &rhs ) : gStamp ( rhs.gStamp ) {}
   virtual ~Time() {}

   inline virtual Time * Clone() const                  { return new Time(*this); }

   inline operator Timestamp &()                        { return gStamp; }
   inline operator const Timestamp &() const            { return gStamp; }

   inline Time &operator= (const Time& rhs)           { gStamp = rhs.gStamp; return *this; }

   inline bool     operator==  (const Time& rhs) const  { return !gStamp.TimeCompare ( rhs.gStamp ); }

   inline bool     operator!=  (const Time& rhs) const  { return gStamp.TimeCompare ( rhs.gStamp ) != 0; }
   inline bool     operator>   (const Time& rhs) const  { return gStamp.TimeCompare ( rhs.gStamp ) > 0; }
   inline bool     operator<   (const Time& rhs) const  { return gStamp.TimeCompare ( rhs.gStamp ) < 0; };
   inline bool     operator>=  (const Time& rhs) const  { return gStamp.TimeCompare ( rhs.gStamp ) >= 0; }
   inline bool     operator<=  (const Time& rhs) const  { return gStamp.TimeCompare ( rhs.gStamp ) <= 0; }

   inline void            Clear()                                    { gStamp.Clear(); }
   inline const char *    GetRawTime() const                         { return gStamp.GetRawTime(); }
   inline void            RetrieveRawTime(std::string & sValue) const   { sValue = gStamp.GetRawTime(); }
   inline unsigned short  GetHour() const                            { return gStamp.GetHour(); }
   inline unsigned short  GetMinute() const                          { return gStamp.GetMinute(); }
   inline unsigned short  GetMillisecond() const                     { return gStamp.GetMillisecond(); }
   inline unsigned short  GetSecond() const                          { return gStamp.GetSecond(); }
   inline double          GetFractionalDay() const                   { return gStamp.GetFractionalDay(); }
   inline unsigned long   GetTimeInMilliseconds() const              { return gStamp.GetTimeInMilliseconds(); }
   void            SetRawTime ( const void *pTime )                  { gStamp.SetRawTime ( pTime ); }
   inline void     SetTimeInMilliseconds ( unsigned long ulMilli )   { gStamp.SetTimeInMilliseconds ( ulMilli ); }
   inline void     SetHour(unsigned short uwHour)                    { gStamp.SetHour ( uwHour ); }
   inline void     SetMinute(unsigned short uwMin)                   { gStamp.SetMinute ( uwMin ); }
   inline void     SetMillisecond( unsigned short uwMilli )          { gStamp.SetMillisecond ( uwMilli ); }
   inline void     SetSecond(unsigned short uwSec)                   { gStamp.SetSecond ( uwSec ); }
   void            StoreTime ( void *sTime ) const                   { gStamp.StoreTimeOnly ( sTime ); }
   inline void     Now( bool bNoMilliseconds = true )                { gStamp.Now ( bNoMilliseconds ); gStamp.ClearDate(); }
};

}
//******************************************************************************
#endif

