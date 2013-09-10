//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataTimeRanges.h"
#include "UtilityFunctions.h"
#include "PrjException.h"
#include <boost/tokenizer.hpp>
#include <deque>

DataTimeRange DataTimeRange::parse(const std::string& from) {
    boost::escaped_list_separator<char> separate_indices('\\', ',');
    boost::tokenizer<boost::escaped_list_separator<char> > rangeCSV(from, separate_indices);
    std::deque<index_t> values;
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itrR=rangeCSV.begin(); itrR != rangeCSV.end(); ++itrR) {
         values.push_back(0);
         if  (!string_to_type<index_t>((*itrR).c_str(), values.back())) {
             throw prg_exception("Value '%s' cannot be read as range index.","parse()",(*itrR).c_str());
         }
    }
    if  (values.size() != 2) {
        throw prg_exception("Range must be defined by two indices, got %u.","parse()", values.size());
    }
    DataTimeRange time_range(values.front(), values.back());
    if  (time_range.getStart() > time_range.getEnd()) {
        throw prg_exception("Range start index must be before or on the end index, got [%u,%u].","parse()", time_range.getStart(), time_range.getEnd());
    }
    return time_range;
}

std::string & DataTimeRange::toString(std::string& s) const {
    s.clear();
    std::stringstream worker;
    worker << "[" << _range.first << "," << _range.second << "]";
    s = worker.str();
    return s;
}

DataTimeRangeSet::rangeset_t DataTimeRangeSet::parse(const std::string& from) {
    rangeset_t rangeset;
    boost::escaped_list_separator<char> separate_ranges('\\', ';');
    boost::tokenizer<boost::escaped_list_separator<char> > range_csv(from, separate_ranges);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itrCSV=range_csv.begin(); itrCSV != range_csv.end(); ++itrCSV) {
        std::string val = *itrCSV;
        trimString(val, "[");
        trimString(val, "]");
        rangeset.push_back(DataTimeRange::parse(val));
    }
    return rangeset;
}

std::string & DataTimeRangeSet::toString(std::string& s) const {
    s.clear();

    std::string buffer;
    std::stringstream worker;
    for (rangeset_t::const_iterator itr=_rangesets.begin(); itr != _rangesets.end(); ++itr) {
        worker << itr->toString(buffer) << ((itr + 1) != _rangesets.end()) ? ";" : "";
    }
    s = worker.str();
    return s;
}
