#ifndef __HYPERGEOMETRICPROBABILITYLOOKUP_H
#define __HYPERGEOMETRICPROBABILITYLOOKUP_H

#include "MinimalGrowthArray.h"
#include "PrjException.h"
#include "Parameters.h"
#include <vector>
#include <set>
#include <algorithm>
#include <stdexcept>

/** Provides a quick lookup table for the hypergeometric probabilities. */
class HypergeometricProbabilityLookup {
public:
    const static double PROBABILITY_UNSET;

    class SpatialCases {
    public:
        typedef std::vector<std::pair<size_t, MinimalGrowthArray<double>>> SpatialCasesData_t;

    private:
        SpatialCasesData_t _probabilities_of_x_at_T;

    public:
        const SpatialCasesData_t& getProbabilitiesOfXAtT() const {
            return _probabilities_of_x_at_T;
        }
        void addNegativeProbabilitiesFor(size_t idx_T, const std::vector<double>& np);
        double getProbabilityAt(size_t idx_T, size_t x) const {
            const auto& pair = _probabilities_of_x_at_T[idx_T]; // get probabilities for T
            return pair.second.getArray()[x - pair.first]; // get probability at x, with consideration of zero base

        }
        double getProbabilityAtChecked(size_t idx_T, size_t x) const {
            if (!_probabilities_of_x_at_T.size()) {
                std::cout << "_probabilities_of_x_at_T.size() == 0" << std::endl;
                return PROBABILITY_UNSET;
            }
            const auto& pair = _probabilities_of_x_at_T.at(idx_T);
            if (x < pair.first || x >= pair.first + pair.second.size()) {
                std::cout << "out of range: x=" << x << ", pair.first=" << pair.first << ", pair.second.size()==" << pair.second.size() << std::endl;
                return PROBABILITY_UNSET;
            }
            return pair.second.at(x - pair.first);
        }
    };

protected:
    std::vector<SpatialCases> _spatial_cases; // spatial cases dimension
    mutable std::vector<unsigned int> _T_index; // index mapping for T values

public:
    HypergeometricProbabilityLookup() = default;
    HypergeometricProbabilityLookup(const HypergeometricProbabilityLookup& other) {
        throw prg_error("copy constructor not implemented.", "HypergeometricProbabilityLookup");
    }

    void calculateHG(Parameters::ScanRateType scanrate, const std::set<count_t>& caseWindows, count_t C);
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityFor(count_t T, const SpatialCases& S, count_t x) const {
        return S.getProbabilityAt(_T_index[T], x);
    }
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityForChecked(count_t T, const SpatialCases& S, count_t x) const {
        return S.getProbabilityAtChecked(_T_index.at(T), x);
    }
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityFor(count_t T, count_t S, count_t x) const {
        return _spatial_cases[S].getProbabilityAt(_T_index[T], x);
    }
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityFor_Checked(count_t T, count_t S, count_t x) const {
        if (!_spatial_cases.size() || S > _spatial_cases.size() - 1) return PROBABILITY_UNSET;
        return _spatial_cases[S].getProbabilityAtChecked(_T_index[T], x);
    }
    const SpatialCases& getSpatialCases(size_t S) const {
        return _spatial_cases[S];
    }
    void printHG(const std::set<count_t>& caseWindows) const;
    void clear() {
        _spatial_cases.clear();
        _T_index.clear();
    }
};


#endif
