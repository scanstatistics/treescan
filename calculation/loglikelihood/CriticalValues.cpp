//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "CriticalValues.h"
#include "PrjException.h"
#include <cstring>
#include <algorithm>
#include <functional>

/** constructor */
CriticalValues::CriticalValues(unsigned int iNumReplications) : _numReplications(iNumReplications) {
    _ratios.resize(static_cast<unsigned int>(ceil((_numReplications+1)*0.05)), 0);
}

/** Adds loglikelihood ratio to array in decending order, if ratio is greater than existing ratio in list. */
bool CriticalValues::add(double llr) {
    container_t::iterator itr = std::upper_bound(_ratios.begin(), _ratios.end(), llr, std::greater<double>());
    if (itr != _ratios.end()) {
        _ratios.insert(itr, llr);
        _ratios.pop_back();
        return true;
    } return false;
}

/** Returns loglikelihood ratio at the top 0.01, relative to the number of replications. */
CriticalValues::alpha_t CriticalValues::getAlpha01()  const {
    double index = floor(static_cast<double>(_numReplications + 1) * 0.01) - 1.0;
    return index >= 0.0 ? CriticalValues::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Returns loglikelihood ratio at the top 0.05, relative to the number of replications. */
CriticalValues::alpha_t CriticalValues::getAlpha05()  const {
    double index = floor(static_cast<double>(_numReplications + 1) * 0.05) - 1.0;
    return index >= 0.0 ? CriticalValues::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Returns loglikelihood ratio at the top 0.001, relative to the number of replications. */
CriticalValues::alpha_t CriticalValues::getAlpha001()  const {
    double index = floor(static_cast<double>(_numReplications + 1) * 0.001) - 1.0;
    return index >= 0.0 ? CriticalValues::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Returns loglikelihood ratio at the top 0.0001, relative to the number of replications. */
CriticalValues::alpha_t CriticalValues::getAlpha0001()  const {
    double index = floor(static_cast<double>(_numReplications + 1) * 0.0001) - 1.0;
    return index >= 0.0 ? CriticalValues::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Returns loglikelihood ratio at the top 0.00001, relative to the number of replications. */
CriticalValues::alpha_t CriticalValues::getAlpha00001()  const {
    double index = floor(static_cast<double>(_numReplications + 1) * 0.00001) - 1.0;
    return index >= 0.0 ? CriticalValues::alpha_t(true,_ratios.at(static_cast<size_t>(index))) : std::make_pair(false,0.0);
}

/** Initialize each ratio of array to zero. */
void CriticalValues::reset() {
    if (_ratios.size()) memset(&_ratios[0], 0, _ratios.size() * sizeof(double));
}
