//***************************************************************************
#ifndef __DataTimeRanges_H
#define __DataTimeRanges_H
//***************************************************************************
#include <boost/iterator/counting_iterator.hpp>

class DataTimeRange {
    public:
        typedef int index_t;
        typedef std::pair<index_t, index_t> range_t;
        typedef boost::counting_iterator<int> iterator_t;

    private:
        range_t _range;

    public:
        DataTimeRange() : _range(0,0) {}
        DataTimeRange(index_t start, index_t end) : _range(start,end) {}
        DataTimeRange(const std::string& from);

        iterator_t begin() {return boost::counting_iterator<int>(_range.first);}
        iterator_t end() {return boost::counting_iterator<int>(_range.second);}

        static DataTimeRange parse(const std::string& from);

        bool operator==(const DataTimeRange& rhs) const {return _range == rhs._range;}
        bool operator!=(const DataTimeRange& rhs) const {return _range != rhs._range;}

        void assign(const std::string& from) {*this = parse(from);}
        index_t getStart() const {return _range.first;}
        index_t getEnd() const {return _range.second;}
        std::string & toString(std::string& s) const;
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
};

class DataTimeRangeSet {
    public:
        typedef std::vector<DataTimeRange> rangeset_t;
        typedef std::pair<bool,size_t> rangeset_index_t;

    private:
        rangeset_t  _rangesets;

    public:
        DataTimeRangeSet() {}
        DataTimeRangeSet(const std::string& from) : _rangesets(parse(from)) {}

        static rangeset_t parse(const std::string& from);

        bool operator==(const DataTimeRangeSet& rhs) const {return _rangesets == rhs._rangesets;}
        bool operator!=(const DataTimeRangeSet& rhs) const {return _rangesets != rhs._rangesets;}

        void assign(const std::string& from) {_rangesets = parse(from);}
        const rangeset_t & getDataTimeRangeSets() const {return _rangesets;}
        std::string & toString(std::string& s) const;
        rangeset_index_t getDataTimeRangeIndex(DataTimeRange::index_t idx) const {
            rangeset_index_t rangeIdx(false,0);
            for (rangeset_t::const_iterator itr=_rangesets.begin(); rangeIdx.first == false && itr != _rangesets.end(); ++itr) {
                rangeIdx.first |= itr->within(idx);
            }
            return rangeIdx;
        }
        DataTimeRange getMinMax() const {
            // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
            DataTimeRange::index_t min=std::numeric_limits<DataTimeRange::index_t>::max(), 
                                   max=std::numeric_limits<DataTimeRange::index_t>::min();
            for (DataTimeRangeSet::rangeset_t::const_iterator itr=_rangesets.begin(); itr != _rangesets.end(); ++itr) {
                min = std::min(min, itr->getStart());
                max = std::max(max, itr->getEnd());
            }
            return DataTimeRange(min,max);
        }
        size_t getTotalDaysAcrossRangeSets() const {
            // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
            DataTimeRange min_max = getMinMax();
            return static_cast<size_t>(min_max.getEnd() - min_max.getStart() + 1);
        }
};
//***************************************************************************
#endif
