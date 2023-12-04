//***************************************************************************
#ifndef __DataTimeRanges_H
#define __DataTimeRanges_H
//***************************************************************************
#include <boost/iterator/counting_iterator.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/optional.hpp>

class DataTimeRange {
    public:
        /** Date precision units */
        enum DatePrecisionType { NONE = 0, GENERIC, YEAR, MONTH, DAY };
        typedef int index_t;
        typedef std::pair<index_t, index_t> range_t;
        typedef boost::counting_iterator<int> iterator_t;

    private:
        range_t _range;
        boost::optional<boost::gregorian::date> _gregorian_start_date;

    public:
        DataTimeRange() : _range(0,0) {}
        DataTimeRange(index_t start, index_t end, boost::optional<boost::gregorian::date> gregorian_start_date = boost::optional<boost::gregorian::date>()) : _range(start,end), _gregorian_start_date(gregorian_start_date) {}
        DataTimeRange(const std::string& from, DatePrecisionType precision, boost::optional<boost::gregorian::date> gregorian_start_date) {
            assign(from, precision, gregorian_start_date);
        }

        iterator_t begin() {return boost::counting_iterator<int>(_range.first);}
        iterator_t end() {return boost::counting_iterator<int>(_range.second);}

        static DataTimeRange parse(const std::string& from, DatePrecisionType precision, boost::optional<boost::gregorian::date> dataRangeStart);

        bool operator==(const DataTimeRange& rhs) const {return _range == rhs._range;}
        bool operator!=(const DataTimeRange& rhs) const {return _range != rhs._range;}

        std::string rangeIdxToGregorianString(index_t idx, DatePrecisionType precision) const;
        std::pair<std::string, std::string> rangeToGregorianStrings(int startIdx, int endIdx, DatePrecisionType precision) const;

        void assign(const std::string& from, DatePrecisionType precision, boost::optional<boost::gregorian::date> gregorian_start_date) {
            *this = parse(from, precision, gregorian_start_date);
        }
        index_t getStart() const {return _range.first;}
        index_t getEnd() const {return _range.second;}
        std::string & toString(std::string& s, DatePrecisionType precision) const;
        bool overlaps(const DataTimeRange& other) const {
            // start index is within other's range
            if (getStart() >= other.getStart() && getStart() <= other.getEnd()) return true;
            // end index is within other's range
            if (getEnd() >= other.getStart() && getEnd() <= other.getEnd()) return true;
            // range encloses other range
            if (getStart() < other.getStart() && getEnd() > other.getEnd()) return true;
            return false;
        }
        bool encloses(const DataTimeRange& other) const {
            return getStart() <= other.getStart() && other.getEnd() <= getEnd();
        }
        bool within(index_t idx) const {
            return getStart() <= idx && idx <= getEnd();
        }
        size_t numDaysInRange() const {
            return static_cast<size_t>(getEnd() - getStart() + 1);
        }
        size_t numDaysInPositiveRange() const {
            return static_cast<size_t>(std::max(getEnd(), 1) - std::max(getStart(), 1) + 1);
        }
        boost::optional<boost::gregorian::date> getDateStart() const {
            return _gregorian_start_date; 
        }
};

/** This class provides functionality for parsing strings that represent dates that are formatted in ways TreeScan claims to accept
    and returns Julian date equivalence. Any errors parsing string into date are indicated by return type DateStringParser::ParserStatus.
    This class adds the ability to have dates separated by any of the characters: '/', '-', '*' and '.'. This class also adds the ability
    to have dates that are also formatted like: "02/1995" and "02/05/1995" (note that two digit years are not supported in new format). */
class DateStringParser {
public:
    enum                                ParserStatus { VALID_DATE = 0, INVALID_DATE, LESSER_PRECISION };
    enum                                DateFormat { MDY = 0, YMD };
    static const char                 * UNSPECIFIED;
    typedef unsigned long Julian;

protected:
    DataTimeRange::DatePrecisionType    geTimePrecision;
    static const unsigned int           DEFAULT_DAY;
    static const unsigned int           DEFAULT_MONTH;

    DateStringParser::ParserStatus      GetInParts(const char * s, unsigned int& iYear, unsigned int& iMonth, unsigned int& iDay, DataTimeRange::DatePrecisionType& ePrecision);

public:
    DateStringParser(DataTimeRange::DatePrecisionType eTimePrecision);

    static std::string gregorianToString(boost::gregorian::date dateObj);
    static boost::gregorian::date gregorianFromString(const std::string& s);

    DateStringParser::ParserStatus      Parse(const char * sDateString, int& dateIdx, boost::optional<boost::gregorian::date> startdate);

    /** Returns whether a date is valid or not. */
    static bool IsDateValid(unsigned int year, unsigned int month, unsigned int day) {
        try {
            boost::gregorian::date test(year, month, day);
            return !test.is_special();
        } catch (std::logic_error&) {
            return false;
        } catch (...) {
            return false;
        }
    }
};

class DataTimeRangeSet {
    public:
        typedef std::vector<DataTimeRange> rangeset_t;
        typedef std::pair<bool,size_t> rangeset_index_t;

    private:
        rangeset_t  _rangesets;

    public:
        DataTimeRangeSet() {}
        DataTimeRangeSet(const std::string& from, DataTimeRange::DatePrecisionType precision, boost::optional<boost::gregorian::date> gregorian_start_date)
            : _rangesets(parse(from, precision, gregorian_start_date)) {}

        static rangeset_t parse(const std::string& from, DataTimeRange::DatePrecisionType precision, boost::optional<boost::gregorian::date> gregorian_start_date);

        bool operator==(const DataTimeRangeSet& rhs) const {return _rangesets == rhs._rangesets;}
        bool operator!=(const DataTimeRangeSet& rhs) const {return _rangesets != rhs._rangesets;}

        void add(const DataTimeRange& range) {_rangesets.push_back(range);}
        void assign(const std::string& from, DataTimeRange::DatePrecisionType precision, boost::optional<boost::gregorian::date> gregorian_start_date) {
            _rangesets = parse(from, precision, gregorian_start_date);
        }
        const rangeset_t & getDataTimeRangeSets() const {return _rangesets;}
        std::string & toString(std::string& s, DataTimeRange::DatePrecisionType precision) const;
        rangeset_index_t getDataTimeRangeIndex(DataTimeRange::index_t idx) const {
            rangeset_index_t rangeIdx(false,0);
            for (rangeset_t::const_iterator itr=_rangesets.begin(); rangeIdx.first == false && itr != _rangesets.end(); ++itr) {
                rangeIdx.first |= itr->within(idx);
            }
            return rangeIdx;
        }
        DataTimeRange getMinMax() const {
            DataTimeRange::index_t min=std::numeric_limits<DataTimeRange::index_t>::max(), 
                                   max=std::numeric_limits<DataTimeRange::index_t>::min();
            for (DataTimeRangeSet::rangeset_t::const_iterator itr=_rangesets.begin(); itr != _rangesets.end(); ++itr) {
                min = std::min(min, itr->getStart());
                max = std::max(max, itr->getEnd());
            }
            return DataTimeRange(min,max);
        }
        size_t getTotalDaysAcrossRangeSets() const {
            DataTimeRange min_max = getMinMax();
            return min_max.numDaysInRange();
        }
};
//***************************************************************************
#endif
