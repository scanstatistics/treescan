//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "TimeStamp.h"
#include "PrjException.h"

// Default timestamp
const char TreeScan::Timestamp::mgsDefault[STAMP_FLD_LEN+1] = "00010101000000000";

// Zero-based month index
const unsigned short  TreeScan::Timestamp::mguwDays[12] = {31,29,31,30,31,30,31,31,30,31,30,31};

// Number of days before a given month
const unsigned short  TreeScan::Timestamp::mguwJulianDaysUpToMonth[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

/** Adds uwAmt hours to the current time. Note that this may change the date. */
void TreeScan::Timestamp::AddHours ( long lAmt )
{
   long   lHours; // Current hour
   long   lDays;  // Number of days to add

   try
      {
      lHours = GetHour();
      lHours += lAmt;

      if ( lHours >= 0 )
      {
         lDays  = lHours / 24;
         SetHour ( lHours % 24 );
      }
      else
      {
         lDays = ( ( lHours + 1 ) / 24 ) - 1;
         SetHour ( ( 24 + ( lHours % 24 ) ) % 24 );
      }

      if ( lDays )
         AddDays ( lDays );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "AddHours()", "Timestamp" );
      throw;
      }
}

/** Adds uwAmt milliseconds to the time. */
void TreeScan::Timestamp::AddMilliseconds ( long lAmt )
{
   long   lMillis; // Current milliseconds
   long   lSecs;

   try
      {
      lMillis = GetMillisecond();
      lMillis += lAmt;

      if ( lMillis >= 0 )
      {
         lSecs  = lMillis / 1000;
         SetMillisecond ( lMillis % 1000 );
      }
      else
      {
         lSecs = ( ( lMillis + 1 ) / 1000 ) - 1;
         SetMillisecond ( ( 1000 + ( lMillis % 1000 ) ) % 1000 );
      }

      if ( lSecs )
         AddSeconds ( lSecs );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "AddMilliseconds()", "Timestamp" );
      throw;
      }
}

/** Adds uwAmt minutes to the time. */
void TreeScan::Timestamp::AddMinutes ( long lAmt )
{
   long   lMins; // Current milliseconds
   long   lHours;

   try
      {
      lMins = GetMinute();
      lMins += lAmt;

      if ( lMins >= 0 )
      {
         lHours  = lMins / 60;
         SetMinute ( lMins % 60 );
      }
      else
      {
         lHours = ( ( lMins + 1 ) / 60 ) - 1;
         SetMinute ( ( 60 + ( lMins % 60 ) ) % 60 );
      }

      if ( lHours )
         AddHours ( lHours );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "AddMinutes()", "Timestamp" );
      throw;
      }
}

/** Adds uwAmt months to the date. Note that this may cause both the year AND the
    _day_ to change. */
void TreeScan::Timestamp::AddMonths ( long lAmt )
{
   long   lMonth; // Current month
   long   lYears;

   try
      {
      lMonth = GetMonth() - 1;
      lMonth += lAmt;

      if ( lMonth >= 0 )
      {
         lYears  = lMonth / 12;
         SetMonth ( ( lMonth % 12 ) + 1 );
      }
      else
      {
         lYears = ( ( lMonth + 1 ) / 12 ) - 1;
         SetMonth ( ( ( 12 + ( lMonth % 12 ) ) % 12 ) + 1 );
      }

      // Check for a month overflow
      if ( GetDay() > mguwDays[GetMonth()-1] )
         {
         SetDay ( GetDay() - mguwDays[GetMonth()-1] );
         SetMonth ( GetMonth() + 1 );
         }

      if ( lYears )
         AddYears ( lYears );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "AddMonths()", "Timestamp" );
      throw;
      }
}

/** Adds uwAmt milliseconds to the time. */
void TreeScan::Timestamp::AddSeconds ( long lAmt )
{
   long   lSecs; // Current milliseconds
   long   lMins;

   try
      {
      lSecs = GetSecond();
      lSecs += lAmt;

      if ( lSecs >= 0 )
      {
         lMins  = lSecs / 60;
         SetSecond ( lSecs % 60 );
      }
      else
      {
         lMins = ( ( lSecs + 1 ) / 60 ) - 1;
         SetSecond ( ( 60 + ( lSecs % 60 ) ) % 60 );
      }

      if ( lMins )
         AddMinutes ( lMins );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "AddSeconds()", "Timestamp" );
      throw;
      }
}

/** Adds uwAmt years to the date. Month and day are unchanged. */
void TreeScan::Timestamp::AddYears ( long lAmt )
{
   long lYear;  // The new year

   try
      {
      lYear = GetYear() + lAmt;

      if ( lYear <= 0 || lYear >= 10000 )
         throw prg_error ( "Dates only support years from 0 to 9999. %d is too large.", "Timestamp", lYear );

      SetYear ( static_cast<unsigned short>(lYear) );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "AddYears()", "Timestamp" );
      throw;
      }
}

/** Returns the day of the week for the date. */
unsigned short TreeScan::Timestamp::GetDayOfWeek() const
{
   unsigned short uwAdjYear;  // The adjusted year
   unsigned short uwAdjMonth;  // The adjusted month
   unsigned short uwRetVal;    // Return value
   try
      {
      uwAdjMonth = GetMonth();
      uwAdjYear  = GetYear();

      if ( uwAdjMonth < 3 )
         {
         uwAdjYear--;
         uwAdjMonth += 10;
         }
      else
         uwAdjMonth -= 2;

      uwRetVal  = GetDay() + uwAdjYear + ( 31 * uwAdjMonth ) / 12;
      uwRetVal += uwAdjYear / 4 - uwAdjYear / 100 + uwAdjYear / 400;

      uwRetVal = uwRetVal % 7;
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "GetDayOfWeek()", "Timestamp" );
      throw;
      }
   return uwRetVal;
}

/** Returns the current time as a fraction of a day */
double TreeScan::Timestamp::GetFractionalDay() const
{
   double dRetVal;

   try
      {
      dRetVal  = (double)GetHour()   * 3600000.0
               + (double)GetMinute() * 60000.0
               + (double)GetSecond() * 1000.0
               + (double)GetMillisecond();

      dRetVal /= 86400000.0;
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "GetFractionalDay()", "Timestamp" );
      throw;
      }
   return dRetVal;
}

/** Returns the julian date. Standard julian dates are reckoned from January 1,
    4713 BC at noon. Thus, the start of our calendar is at 1721424.5 in this
    system */
double TreeScan::Timestamp::GetJulianDate() const
{
   double   dRetVal;

   try
      {
      dRetVal = static_cast<double> ( GetJulianDayFromCalendarStart() ) + 1721424.5;

      dRetVal += GetJulianTime();
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "GetJulianDate()", "Timestamp" );
      throw;
      }
   return dRetVal;
}

/** Returns the julian date from January 1st,1 at midnight. This is to
    coincide with Borland's concept of a Julian date. Other functions are
    provided to get "real" Julian dates. */
double TreeScan::Timestamp::GetJulianDateFromCalendarStart() const
{
   double   dRetVal;

   try
      {
      dRetVal = static_cast<double> ( GetJulianDayFromCalendarStart() );

      dRetVal += GetJulianTime();
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "GetJulianDateFromCalendarStart()", "Timestamp" );
      throw;
      }
   return dRetVal;
}

/** Returns the number of days from January 1st, 1. (The initial day is
    included, so January 1st,b1 is 1.) */
unsigned long TreeScan::Timestamp::GetJulianDayFromCalendarStart() const
{
   unsigned short uwLeapYearYear; // The "leap year year", starts at 03/01
   unsigned long  ulRetVal;       // Return value
   try
      {
      // Note that the one-base of the julian day is introduced by the fact
      // that the days in a month are numbered starting at "1".
      ulRetVal =  GetDay();
      ulRetVal += mguwJulianDaysUpToMonth[GetMonth()-1];
      ulRetVal += ( (unsigned long)GetYear() - 1 ) * 365;

      // LeapYearYear starts on March 1st
      uwLeapYearYear = GetYear();

      if ( GetMonth() < 3 )
         uwLeapYearYear--;

      // Add in the leap year adjustments
      ulRetVal += uwLeapYearYear / 4;
      ulRetVal -= uwLeapYearYear / 100;
      ulRetVal += uwLeapYearYear / 400;
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "GetJulianDayFromCalendarStart()", "Timestamp" );
      throw;
      }
   return ulRetVal;
}

/** Returns the modified Julian date. Modified Julian dates are reckoned from
    November 17th, 1858 at midnight. Thus, the start of our calendar is
    -678576 as a modified Julian dates. */
double TreeScan::Timestamp::GetModifiedJulianDate() const
{
   double   dRetVal;

   try
      {
      dRetVal = static_cast<double> ( GetJulianDayFromCalendarStart() ) - 678576.0;

      dRetVal += GetJulianTime();
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "GetModifiedJulianDate()", "Timestamp" );
      throw;
      }
   return dRetVal;
}

/** Returns the time of day in milliseconds since midnight */
unsigned long TreeScan::Timestamp::GetTimeInMilliseconds() const
{
   unsigned long ulRetVal;

   try
      {
      ulRetVal = (unsigned long)GetHour()   * 3600000
               + (unsigned long)GetMinute() * 60000
               + (unsigned long)GetSecond() * 1000
               + (unsigned long)GetMillisecond();
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "GetTimeInMilliseconds()", "Timestamp" );
      throw;
      }
   return ulRetVal;
}

TreeScan::Timestamp TreeScan::Timestamp::min() {
  TreeScan::Timestamp timestamp;

  timestamp.SetYear( 0 );
  timestamp.SetMonth( 1 );
  timestamp.SetDay( 1 );
  timestamp.SetHour( 0 );
  timestamp.SetMinute( 0 );
  timestamp.SetSecond( 0 );
  timestamp.SetMillisecond( 0 );

  return timestamp;
}

TreeScan::Timestamp TreeScan::Timestamp::max() {
  TreeScan::Timestamp timestamp;

  timestamp.SetYear( 9999 );
  timestamp.SetMonth( 12 );
  timestamp.SetDay( 31 );
  timestamp.SetHour( 23 );
  timestamp.SetMinute( 59 );
  timestamp.SetSecond( 59 );
  timestamp.SetMillisecond( 999 );

  return timestamp;
}

#ifdef _WINDOWS_
void TreeScan::Timestamp::MakeLocalTime()
{
   TIME_ZONE_INFORMATION  tzInfo;
   DWORD                  dwRetVal;

   try
      {
      dwRetVal = GetTimeZoneInformation ( &tzInfo );

      if ( dwRetVal == TIME_ZONE_ID_INVALID )
         throw prg_error ( "Unable to retrieve time zone information -- ", "MakeLocalTime" );

      if ( dwRetVal == TIME_ZONE_ID_DAYLIGHT )
         AddMinutes ( -tzInfo.Bias - tzInfo.DaylightBias );
      else if ( dwRetVal == TIME_ZONE_ID_STANDARD )
         AddMinutes ( -tzInfo.Bias - tzInfo.StandardBias );
      else
         AddMinutes ( -tzInfo.Bias );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "MakeLocalTime()", "Timestamp" );
      throw;
      }

}

void TreeScan::Timestamp::MakeUniversalTime()
{
   TIME_ZONE_INFORMATION  tzInfo;
   DWORD                  dwRetVal;

   try
      {
      dwRetVal = GetTimeZoneInformation ( &tzInfo );

      if ( dwRetVal == TIME_ZONE_ID_DAYLIGHT )
         AddMinutes ( tzInfo.Bias + tzInfo.DaylightBias );
      else if ( dwRetVal == TIME_ZONE_ID_STANDARD )
         AddMinutes ( tzInfo.Bias + tzInfo.StandardBias );
      else
         AddMinutes ( tzInfo.Bias );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "MakeUniversalTime()", "Timestamp" );
      throw;
      }

}

void TreeScan::Timestamp::Now ( bool bNoMilliseconds )
{
   SYSTEMTIME        SysTime;

   try
      {
      GetSystemTime ( &SysTime );

      SetYear ( SysTime.wYear );
      SetMonth ( SysTime.wMonth );
      SetDay ( SysTime.wDay );
      SetHour ( SysTime.wHour );
      SetMinute ( SysTime.wMinute );
      SetSecond ( SysTime.wSecond );
      SetMillisecond ( ( bNoMilliseconds ) ? 0 : SysTime.wMilliseconds );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "Now()", "Now" );
      throw;
      }
}
#else
void TreeScan::Timestamp::MakeLocalTime()
{
   time_t tempTime_t(time(0));
   tm * pTm(localtime(&tempTime_t));
   tm tempTm;

   try
      {
      if (!pTm)
        throw prg_error("Could not get system time.", "MakeLocalTime()");

      Clear();
      tempTm = *pTm;
      SetYear(tempTm.tm_year + 1900);
      SetMonth(tempTm.tm_mon + 1);
      SetDay(tempTm.tm_mday);
      SetHour(tempTm.tm_hour);
      SetMinute(tempTm.tm_min);
      SetSecond(tempTm.tm_sec);
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "MakeLocalTime()", "Timestamp" );
      throw;
      }

}

void TreeScan::Timestamp::MakeUniversalTime()
{
   time_t tempTime_t(time(0));
   tm * pTm(gmtime(&tempTime_t));
   tm tempTm;

   try
      {
      if (!pTm)
         throw prg_error("Could not get system time.", "MakeUniversalTime()");

      Clear();
      tempTm = *pTm;
      SetYear(tempTm.tm_year + 1900);
      SetMonth(tempTm.tm_mon + 1);
      SetDay(tempTm.tm_mday);
      SetHour(tempTm.tm_hour);
      SetMinute(tempTm.tm_min);
      SetSecond(tempTm.tm_sec);
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "MakeUniversalTime()", "Timestamp" );
      throw;
      }

}

/** Sets the time to the current UTC. */
void TreeScan::Timestamp::Now ( bool bNoMilli )
{
   struct timeval   tmStruct;

   try
      {
      gettimeofday ( &tmStruct, 0 );

      // Set the date and time to the start of the unix epoch
      memcpy ( gsStamp, "19700101000000000", STAMP_FLD_LEN );

      // Add the seconds
      AddSeconds ( tmStruct.tv_sec );

      if ( !bNoMilli )
         SetMillisecond ( tmStruct.tv_usec / 1000 );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "Now()", "Timestamp" );
      throw;
      }
}
#endif

/** Checks to see if the day entered is a valid one and
    then replaces the current day with the new one. */
void TreeScan::Timestamp::SetDay(unsigned short uwDay)
{
     try
        {
        if (uwDay > mguwDays[GetMonth()-1] )
            throw prg_error("%hd is not a valid day in month %hd", "Timestamp", uwDay, GetMonth() );

        gsStamp[6] = ( uwDay / 10 ) + '0';
        gsStamp[7] = ( uwDay % 10 ) + '0';
        }
    catch (prg_exception& x)
        {
        x.addTrace("SetDay()", "Timestamp");
        throw;
        }
}

/** Sets the hour of the time field. */
void TreeScan::Timestamp::SetHour(unsigned short uwHour)
{
   try
      {
      if ( uwHour > 23 )
         throw prg_error ( "%hd is not a valid hour", "Timestamp", uwHour );

      gsStamp[8] = ( uwHour / 10 ) + '0';
      gsStamp[9] = ( uwHour % 10 ) + '0';
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetHour()", "Timestamp" );
      throw;
      }
}

/** Sets the timestamp using the julian day dTime. dTime is considered to be the
    total number of days since January 1st, 4713 BC 12:00pm */
void TreeScan::Timestamp::SetJulianDate ( double dTime )
{
   try
      {
      SetJulianDayFromCalendarStart ( static_cast<unsigned long> ( floor ( dTime - 1721424.5 ) ) );

      SetJulianTime ( dTime );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetJulianDate()", "Timestamp" );
      throw;
      }
}

/** Sets the timestamp using the julian day dTime. dTime is considered to be the
    total number of days since January 1st, 1 12:00am. */
void TreeScan::Timestamp::SetJulianDateFromCalendarStart ( double dTime )
{
   try
      {
      SetJulianDayFromCalendarStart ( static_cast<unsigned long> ( floor ( dTime ) ) );

      SetJulianTime ( dTime );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetJulianDateFromCalendarStart()", "Timestamp" );
      throw;
      }
}

void TreeScan::Timestamp::SetJulianDayFromCalendarStart ( unsigned long ulJulian )
{
   unsigned short  uwYear;
   unsigned short  uwMonth;
   unsigned short  uwDay;
   bool bIsLeapYear;

   try
      {
      // Correct for one-base of julian day
      --ulJulian;

      uwYear    = static_cast<unsigned short> ( ulJulian / 146097 ) * 400;  // Days in 400 years
      ulJulian  = ulJulian % 146097;

      uwYear   += static_cast<unsigned short> ( ulJulian / 36524 ) * 100;  // Days in 100 years
      ulJulian  = ulJulian % 36524;

      uwYear   += static_cast<unsigned short> ( ulJulian / 1461 ) * 4;  // Days in 4 years
      ulJulian  = ulJulian % 1461;

      // Note that the _last_ year in a group of four ( i.e. x where x % 4 == 3 )
      // is the leap year. This is because we are using zero-based years so far.
      bIsLeapYear = ulJulian > (1461 - 366);
      if (bIsLeapYear)
         --ulJulian;
      uwYear   += static_cast<unsigned short> ( ulJulian / 365 ); // Days in a year
      ulJulian = ulJulian % 365;
      if (bIsLeapYear)
         ++ulJulian;

      // Make year one-based
      ++uwYear;

      uwDay = static_cast<unsigned short> ( ulJulian );
      uwDay++; // shift from 0-based Julian day-of-year to 1-based day-of-month
      uwMonth = 0;
      // Figure out if it is a leap year
      if ( bIsLeapYear )
         {
         // This loop will terminate with the correct _one_-based month
         if ( uwDay > mguwJulianDaysUpToMonth[2] )
            {
            --uwDay;
            uwMonth = 2;

            while ( ( uwMonth < 12 ) && ( uwDay > mguwJulianDaysUpToMonth[uwMonth] ) )
               ++uwMonth;
            }
         else
            uwMonth = ( uwDay > mguwJulianDaysUpToMonth[1] ) ? 2 : 1;
         } // Leap year calculation
      else
         {
         // This loop will terminate with the correct _one_-based month
         while ( ( uwMonth < 12 ) && ( uwDay > mguwJulianDaysUpToMonth[uwMonth] ) )
            uwMonth++;
         } // end of normal year calculation

      uwDay = uwDay - mguwJulianDaysUpToMonth[uwMonth-1];

      SetYear ( uwYear );
      SetMonth ( uwMonth );
      SetDay ( uwDay );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetJulianDayFromCalendarStart()", "Timestamp" );
      throw;
      }
}

/** Sets the minute */
void TreeScan::Timestamp::SetMinute(unsigned short uwMin)
{
   try
      {
      if ( uwMin > 59 )
         throw prg_error ( "%hd is not a valid minute", "Timestamp", uwMin );

      gsStamp[10] = ( uwMin / 10 ) + '0';
      gsStamp[11] = ( uwMin % 10 ) + '0';
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetMinute()", "Timestamp" );
      throw;
      }
}

void TreeScan::Timestamp::SetMillisecond( unsigned short uwMilli )
{
   try
      {
      if ( uwMilli > 999 )
         throw prg_error ( "%hd is not a valid millisecond", "Timestamp", uwMilli );

      gsStamp[14] = ( uwMilli / 100 ) + '0';
      gsStamp[15] = ( ( uwMilli % 100 ) / 10 ) + '0';
      gsStamp[16] = ( uwMilli % 10 ) + '0';
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetMilliseconds()", "Timestamp" );
      throw;
      }
}

/** Sets the timestamp using the julian day dTime. dTime is considered to be the
    total number of days since November 17th, 1858 at midnight. */
void TreeScan::Timestamp::SetModifiedJulianDate ( double dTime )
{
   try
      {
      SetJulianDayFromCalendarStart ( static_cast<unsigned long> ( floor ( dTime + 678576.0 ) ) );

      SetJulianTime ( dTime );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetModifiedJulianDate()", "Timestamp" );
      throw;
      }
}

/** Sets the month to the indicated value. If the current day is larger than the
    maximum day for a month, the day is reset to the last valid day of the month. */
void TreeScan::Timestamp::SetMonth(unsigned short uwMonth)
{
    try
        {
        if(uwMonth > 12)
            throw prg_error("%hd is not a valid month", "Timestamp");

        gsStamp[4] = ( uwMonth / 10 ) + '0';
        gsStamp[5] = ( uwMonth % 10 ) + '0';

        if( GetDay() > mguwDays[uwMonth-1] )
           SetDay ( mguwDays[uwMonth-1] );
        }
    catch (prg_exception& x)
        {
        x.addTrace("SetMonth()", "Timestamp");
        throw;
        }
}

/** Sets the current second */
void TreeScan::Timestamp::SetSecond(unsigned short uwSec)
{
   try
      {
      if ( ( uwSec > 59 ) && ( GetHour() != 23 || GetMinute() != 59 ) )
         throw prg_error ( "%hd is not a valid second", "Timestamp", uwSec );

      gsStamp[12] = ( uwSec / 10 ) + '0';
      gsStamp[13] = ( uwSec % 10 ) + '0';
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetSecond()", "Timestamp" );
      throw;
      }
}

/** Sets the time of day in milliseconds since midnight */
void TreeScan::Timestamp::SetTimeInMilliseconds( unsigned long ulMilli )
{
   try
      {
      SetHour ( static_cast<unsigned short>(ulMilli / 3600000) );
      ulMilli = ulMilli % 3600000;

      SetMinute ( static_cast<unsigned short>(ulMilli / 60000) );
      ulMilli = ulMilli % 60000;

      SetSecond ( static_cast<unsigned short>(ulMilli / 1000) );
      ulMilli = ulMilli % 1000;

      SetMillisecond ( static_cast<unsigned short>(ulMilli) );
      }
   catch ( prg_exception& x )
      {
      x.addTrace ( "SetTimeInMilliseconds()", "Timestamp" );
      throw;
      }
}

/** Sets the year. If the old date was set to leap day and the new year is not a
    leap year, the date will be changed to 2/28. */
void TreeScan::Timestamp::SetYear(unsigned short uwYear)
{
    try
        {
        if( uwYear > 9999)
            throw prg_error("%hd is too large; 9999 is the maximum year", "Timestamp", uwYear);

        gsStamp[0] = ( uwYear / 1000 ) + '0';
        gsStamp[1] = ( ( uwYear % 1000 ) / 100 ) + '0';
        gsStamp[2] = ( ( uwYear % 100 ) / 10 ) + '0';
        gsStamp[3] = ( uwYear % 10 ) + '0';

        if ( ( GetMonth() == 2 ) && ( GetDay() == 29 ) )
           if((uwYear % 4 != 0) || ((uwYear % 100 == 0) && (uwYear % 400 != 0)))
               SetDay ( 28 );
        }
    catch (prg_exception& x)
        {
        x.addTrace("SetYear()", "Timestamp");
        throw;
        }
}

