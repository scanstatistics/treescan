//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "SampleSiteData.h"
#include "ScanRunner.h"

/* Returns the average baseline and current across all sample sites. */
std::pair<double, double> getAverage(const SampleSiteMap_t& ssData) {
    double total_baseline = 0.0;
    double total_current = 0.0;
    for (const auto& ss : ssData) {
        total_baseline += ss.second.baseline();
        total_current += ss.second.current();
    }
    return std::make_pair(total_baseline / static_cast<double>(ssData.size()), total_current / static_cast<double>(ssData.size()));
}

/* Combines passed sample site map into sample site map accumulation. */
SampleSiteMap_t& combine(SampleSiteMap_t& accumulation, const SampleSiteMap_t& src, bool clear){
    if (clear) accumulation.clear();
    if (accumulation.empty()) {
        accumulation = src;
        return accumulation;
    }
    for (const auto& ss : src) {
        auto itr = accumulation.find(ss.first);
        if (itr != accumulation.end())
            itr->second.add(ss.second);
        else
            accumulation.emplace(ss.first, ss.second);
    }
    return accumulation;
}