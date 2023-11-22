//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataTimeRanges.h"
#include "UtilityFunctions.h"
#include "PrjException.h"
#include <boost/tokenizer.hpp>
#include <deque>

/////////////////////////////////// DataTimeRange //////////////////////////////////////////////

/* Parses date time range from string - using DataTimeRange::DatePrecisionType to determine how to parse. 
   If parameter gregorian_start_date is defined, uses that date as base date when translating to time index. */
DataTimeRange DataTimeRange::parse(const std::string& from, DataTimeRange::DatePrecisionType precision, boost::optional<boost::gregorian::date> dataRangeStart) {
    std::string fromMod = from;
    fromMod.erase(std::remove(fromMod.begin(), fromMod.end(), ' '), fromMod.end());
    trimString(trimString(fromMod, "["), "]");

    std::deque<index_t> indexes;
    boost::gregorian::date rangeStart;
    boost::escaped_list_separator<char> separate_indices('\\', ',');
    boost::tokenizer<boost::escaped_list_separator<char> > rangeCSV(fromMod, separate_indices);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itrR=rangeCSV.begin(); itrR != rangeCSV.end(); ++itrR) {
         switch (precision) {
            case YEAR: {
                boost::gregorian::date dateValue = DateStringParser::gregorianFromString((*itrR));
                //dateValue = boost::gregorian::date(dateValue.year(), indexes.size() == 0 ? 1 : 12, indexes.size() == 0 ? 1 : 31);
                //if (indexes.size() == 0 && (dateValue.month() != 1 || dateValue.day() != 1))
                //    throw resolvable_error("The range start date '%s' is invalid for yearly time precision - must be first day of year.", itrR->c_str());
                //else if (indexes.size() == 1 && (dateValue.month() != 12 || dateValue.day() != boost::gregorian::gregorian_calendar::end_of_month_day(dateValue.year(), dateValue.month())))
                //    throw resolvable_error("The range end date '%s' is invalid for yearly time precision - must be last day of year.", itrR->c_str());
                if (!dataRangeStart) dataRangeStart = dateValue;
                indexes.push_back(static_cast<int>(dateValue.year() - dataRangeStart.get().year()));
            } break;
            case MONTH: {
                boost::gregorian::date dateValue = DateStringParser::gregorianFromString((*itrR));
                //dateValue = boost::gregorian::date(dateValue.year(), dateValue.month(), 
                //    indexes.size() == 0 ? 1 : boost::gregorian::gregorian_calendar::end_of_month_day(dateValue.year(), dateValue.month())
                //);
                //if (indexes.size() == 0 && dateValue.day() != 1)
                //    throw resolvable_error("The range start date '%s' is invalid for monthly time precision - must be first day of month.", itrR->c_str());
                //else if (indexes.size() == 1 && dateValue.day() != boost::gregorian::gregorian_calendar::end_of_month_day(dateValue.year(), dateValue.month()))
                //    throw resolvable_error("The range end date '%s' is invalid for monthly time precision - must be last day of month.", itrR->c_str());
                if (!dataRangeStart) dataRangeStart = dateValue;
                indexes.push_back(static_cast<int>((dateValue.year() - dataRangeStart.get().year()) * 12 + (dateValue.month() - dataRangeStart.get().month())));
            } break;
            case DAY: {
                boost::gregorian::date dateValue = DateStringParser::gregorianFromString((*itrR));
                if (!dataRangeStart) dataRangeStart = dateValue;
                indexes.push_back(static_cast<int>((dateValue - dataRangeStart.get()).days()));
            } break;
            case GENERIC:
            default:
                indexes.push_back(0);
                if (!string_to_numeric_type<index_t>((*itrR).c_str(), indexes.back())) {
                    throw resolvable_error("Range value '%s' cannot be read as range index.", (*itrR).c_str());
                } break;
         }
    }
    if  (indexes.size() != 2)
        throw resolvable_error("Range must be defined by two values, got %d.", indexes.size());
    if (indexes.front() > indexes.back())
        throw resolvable_error("Range start cannot come after range end, got %s.", from.c_str());
    return DataTimeRange(indexes.front(), indexes.back(), dataRangeStart);
}

/* Returns data index as formatted data string. */
std::string DataTimeRange::rangeIdxToGregorianString(index_t idx, DatePrecisionType precision) const {
    boost::gregorian::date translatedDate;
    switch (precision) {
        case YEAR:
            translatedDate = boost::gregorian::date(_gregorian_start_date.get().year() + idx, 1, 1);
            break;
        case MONTH:
            translatedDate = boost::gregorian::date(_gregorian_start_date.get().year() + (idx / 12), (idx % 12) + 1, 1);
            break;
        case DAY:
            translatedDate = _gregorian_start_date.get() + boost::gregorian::date_duration(idx);
            break;
        default: throw prg_error("Unknown precision '%d'.", "rangeIdxToGregorianString()", precision);
    };
    return DateStringParser::gregorianToString(translatedDate);
}

/* Returns string pair that is the formatted dates for passed start and end date indexes. */
std::pair<std::string, std::string> DataTimeRange::rangeToGregorianStrings(int startIdx, int endIdx, DatePrecisionType precision) const {
    boost::gregorian::date translatedStart, translatedEnd;
    switch (precision) {
        case YEAR:
            translatedStart = boost::gregorian::date(_gregorian_start_date.get().year() + startIdx, 1, 1);
            translatedEnd = boost::gregorian::date(_gregorian_start_date.get().year() + endIdx, 12, 31);
            break;
        case MONTH: {
            boost::gregorian::date temp = _gregorian_start_date.get() + boost::gregorian::years(startIdx / 12) + boost::gregorian::months(startIdx % 12);
            translatedStart = boost::gregorian::date(temp.year(), temp.month(), 1);
            temp = _gregorian_start_date.get() + boost::gregorian::years(endIdx / 12) + boost::gregorian::months(endIdx % 12);
            translatedEnd = boost::gregorian::date(temp.year(), temp.month(), boost::gregorian::gregorian_calendar::end_of_month_day(temp.year(), temp.month()));
        } break;
        case DAY:
            translatedStart = _gregorian_start_date.get() + boost::gregorian::date_duration(startIdx);
            translatedEnd = _gregorian_start_date.get() + boost::gregorian::date_duration(endIdx);
            break;
        default: throw prg_error("Unknown precision '%d'.", "rangeIdxToGregorianString()", precision);
    };
    return std::make_pair(DateStringParser::gregorianToString(translatedStart), DateStringParser::gregorianToString(translatedEnd));
}

std::string & DataTimeRange::toString(std::string& s, DataTimeRange::DatePrecisionType precision) const {
    s.clear();
    std::stringstream worker;
    if (precision == DataTimeRange::GENERIC)
        worker << "[" << _range.first << "," << _range.second << "]";
    else {
        std::pair<std::string, std::string> rangeDates = rangeToGregorianStrings(_range.first, _range.second, precision);
        worker << "[" << rangeDates.first << "," << rangeDates.second << "]";
    }
    s = worker.str();
    return s;
}

/////////////////////////////////// DateStringParser //////////////////////////////////////////////

const unsigned int DateStringParser::DEFAULT_DAY = 1;
const unsigned int DateStringParser::DEFAULT_MONTH = 1;
const char * DateStringParser::UNSPECIFIED = "unspecified";

/** constructor */
DateStringParser::DateStringParser(DataTimeRange::DatePrecisionType eTimePrecision) : geTimePrecision(eTimePrecision) {}

/* Converts boost::gregorian::date object to string in format yyy/mm/dd. */
std::string DateStringParser::gregorianToString(boost::gregorian::date dateObj) {
    const std::locale fmt(std::locale::classic(), new boost::gregorian::date_facet("%Y/%m/%d"));
    std::ostringstream os;
    os.imbue(fmt);
    os << dateObj;
    return os.str();
}

/* Attempts convert string to boost::gregorian::date object. */
boost::gregorian::date DateStringParser::gregorianFromString(const std::string& s) {
    unsigned int month, day, year;
    if (sscanf(s.c_str(), "%u/%u/%u", &year, &month, &day) < 3)
        throw resolvable_error("Unable to parse string to date: '%s'.", s.c_str());
    boost::gregorian::date thisDate;
    try {
        thisDate = boost::gregorian::date(year, month, day);
        if (thisDate.is_special())
            throw resolvable_error("Unable to parse string to date: '%s'.", s.c_str());
    } catch (std::logic_error& e) {
        throw resolvable_error("Unable to parse string to date: '%s'. %s", s.c_str(), e.what());
    }
    return thisDate;
}

/** Parses passed date string. */
DateStringParser::ParserStatus DateStringParser::Parse(const char * sDateString, int& dateIdx, boost::optional<boost::gregorian::date> startdate) {
    // if time precision is generic, conversion is simple.
    if (geTimePrecision == DataTimeRange::GENERIC) {
        if (!string_to_numeric_type<int>(sDateString, dateIdx))
            return INVALID_DATE;
        return VALID_DATE;
    }
    // Otherwise convert based on time precision setting.
    DataTimeRange::DatePrecisionType readPrecision;
    boost::gregorian::date thisDate;
    unsigned int iYear, iMonth, iDay;
    ParserStatus eParserStatus = GetInParts(sDateString, iYear, iMonth, iDay, readPrecision);
    if (eParserStatus != VALID_DATE)
        return eParserStatus;
    if (readPrecision < geTimePrecision)
        return LESSER_PRECISION;
    switch (readPrecision) {
        case DataTimeRange::YEAR:
            if (!IsDateValid(iYear, DEFAULT_MONTH, DEFAULT_DAY)) return INVALID_DATE;
            thisDate = boost::gregorian::date(iYear, DEFAULT_MONTH, DEFAULT_DAY);
            dateIdx = static_cast<int>(thisDate.year() - startdate.get().year());
            break;
        case DataTimeRange::MONTH:
            if (!IsDateValid(iYear, iMonth, DEFAULT_DAY)) return INVALID_DATE;
            thisDate = boost::gregorian::date(iYear, iMonth, DEFAULT_DAY);
            dateIdx = (thisDate.year() - startdate.get().year()) * 12 + (thisDate.month() - startdate.get().month());
            break;
        case DataTimeRange::DAY:
            if (!IsDateValid(iYear, iMonth, iDay)) return INVALID_DATE;
            thisDate = boost::gregorian::date(iYear, iMonth, iDay);
            dateIdx = static_cast<int>((thisDate - startdate.get()).days());
            break;
        default: return INVALID_DATE;
    };
    return VALID_DATE;
}

/* Parses passed string into date components, noting the determined precision and format. This function supports dates of formats: yyyy/mm/dd or
   mm/dd/yyyy with all lesser precisions (i.e. yyyy/mm or yyyy). This funciton also permits the date string use separators '/', '-', '.', '*'. */
DateStringParser::ParserStatus DateStringParser::GetInParts(const char * s,  unsigned int& iYear, unsigned int& iMonth, unsigned int& iDay, DataTimeRange::DatePrecisionType& ePrecision) {
    //determine precision
    size_t iLength, iCount = 1;
    std::string sFormat("%u");
    const char * ptr = s;
    while ((iLength = strcspn(ptr, "/-.*")) < strlen(ptr)) {
        ptr += iLength;
        if (++iCount > 3)
            return INVALID_DATE;
        sFormat += *ptr;
        sFormat += "%u";
        ptr += 1;
    }
    ePrecision = (DataTimeRange::DatePrecisionType)(iCount + 1);
    //scan into parts - determined by precision
    switch (ePrecision) {
        case DataTimeRange::YEAR: 
            if (sscanf(s, sFormat.c_str(), &iYear) != 1) return INVALID_DATE;
            break;
        case DataTimeRange::MONTH: // yyyy/mm or mm/yyyy
            if (sscanf(s, sFormat.c_str(), &iYear, &iMonth) != 2) return INVALID_DATE;
            if (iMonth > 12) std::swap(iYear, iMonth); //
            break;
        case DataTimeRange::DAY: // yyyy/mm/dd or mm/dd/yyyy
            if (sscanf(s, sFormat.c_str(), &iYear, &iMonth, &iDay) != 3) return INVALID_DATE;
            if (iYear < 13) { std::swap(iYear, iDay); std::swap(iMonth, iDay); }
            break;
        case DataTimeRange::GENERIC:
        default: return INVALID_DATE;
    };
    return VALID_DATE;
}

/////////////////////////////////// DataTimeRangeSet //////////////////////////////////////////////

DataTimeRangeSet::rangeset_t DataTimeRangeSet::parse(const std::string& from, DataTimeRange::DatePrecisionType precision, boost::optional<boost::gregorian::date> gregorian_start_date) {
    rangeset_t rangeset;
    boost::escaped_list_separator<char> separate_ranges('\\', ';');
    boost::tokenizer<boost::escaped_list_separator<char> > range_csv(from, separate_ranges);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itrCSV=range_csv.begin(); itrCSV != range_csv.end(); ++itrCSV) {
        std::string val = *itrCSV;
        rangeset.push_back(DataTimeRange::parse(val, precision, gregorian_start_date));
    }
    return rangeset;
}

std::string & DataTimeRangeSet::toString(std::string& s, DataTimeRange::DatePrecisionType precision) const {
    s.clear();

    std::string buffer;
    std::stringstream worker;
    for (rangeset_t::const_iterator itr=_rangesets.begin(); itr != _rangesets.end(); ++itr) {
        worker << itr->toString(buffer, precision).c_str() << ((itr + 1) != _rangesets.end() ? ";" : "");
    }
    s = worker.str();
    return s;
}
