//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************

#include "HypergeometricProbabilityLookup.h"
#include <fstream>

const double HypergeometricProbabilityLookup::PROBABILITY_UNSET = -99;

// Macro for accessing natural log calculations from precomputed vector or standard log function
#define STD_LOG(c) caseLog[(unsigned int)c] // STD_LOG(c) log(static_cast<double>(c))

void HypergeometricProbabilityLookup::calculateHG(Parameters::ScanRateType scanrate, const std::set<count_t>& caseWindows, count_t C) {
	// Precompute natural logarithm values for 0..C to avoid repeated calls to log() function.
    std::vector<double> caseLog(C + 1, 0.0);
    for (size_t t = 1; t < caseLog.size(); ++t) caseLog[t] = log(t);

    auto calc_hg_0 = [&caseLog](count_t C, count_t T, count_t S) { // probability of 0 cases in cylinder, based on hypergeometric distribution
        double hg = STD_LOG(C - S) - STD_LOG(C);
        for (count_t i = 1; i <= T - 1; ++i)
            hg += STD_LOG(C - S - i) - STD_LOG(C - i);
        return hg;
    };
    auto calc_hg_t = [&caseLog](count_t C, count_t T, count_t S) { // probability of x cases in cylinder, based on hypergeometric distribution
        double hg = STD_LOG(T) - STD_LOG(C);
        for (count_t i = 1; i <= C - S - 1; ++i) {
            hg += STD_LOG(T - i) - STD_LOG(C - i);
            if (hg == 0) break;
        }
        return hg;
    };
    if (caseWindows.empty()) return; // What do we do here, which might happen with multiple data sets?
    // Find the largest case window value - so we know how large to allocate translation array.
    _T_index.resize(*caseWindows.rbegin() + 1);
    _spatial_cases.resize(C + 1); // create SpatialCases vector for all C
    std::vector<double> negativeProbabilities(C + 1, PROBABILITY_UNSET); // temporary storage for negative probabilities
    size_t i = 0;
    for (auto T : caseWindows) {
        if (!T) continue; // skip T=0, which shouldn't be here anyway
        //std::cout << "Calculating HG for T=" << T << " (" << i + 1 << " of " << vT.size() << ")" << std::endl;
        for (count_t S = 2; S <= C - 2; ++S) {
            std::fill(negativeProbabilities.begin(), negativeProbabilities.end(), PROBABILITY_UNSET);
            count_t min = std::max((count_t)0, S + T - C);
            count_t max = std::min(S, T);
            count_t mean = static_cast<count_t>(ceil((double)S * (double)T / (double)C));
            // The natural logarithm of the probability of x cases in cylinder, based on hypergeometric distribution,
            // to be temporarily stored in a one-dimensional array. To get the actial probability, take exp[hgp(x)].
            std::vector<double> hgp(max + 1, 0);
            if (S + T - C <= 0)
                hgp.front() = calc_hg_0(C, T, S);
            else
                hgp[S + T - C] = calc_hg_t(C, T, S);
            for (count_t x = min + 1; x <= max; ++x)
                hgp[x] = hgp[x - 1] + (STD_LOG(S - x + 1) - STD_LOG(x)) + (STD_LOG(T - x + 1) - STD_LOG(C - S - T + x));
            if (scanrate == Parameters::HIGHRATE || scanrate == Parameters::HIGHORLOWRATE) {
                // For high rates nHG(x|S,T) = negative of the cumulative probability of x or more cases, x>=2. 
                negativeProbabilities[max] = -exp(hgp[max]);
                for (count_t x = max - 1; x >= mean; --x)
                    negativeProbabilities[x] = negativeProbabilities[x + 1] - exp(hgp[x]);
            }
            if (scanrate == Parameters::LOWRATE || scanrate == Parameters::HIGHORLOWRATE) {
                // For low rates nHG(x|S,T) = negative of the cumulative probability of x or less cases, x >= 0.
                negativeProbabilities[min] = -exp(hgp[min]);
                for (count_t x = min + 1; x <= mean - 1; ++x)
                    negativeProbabilities[x] = negativeProbabilities[x - 1] - exp(hgp[x]);
            }
            _spatial_cases[S].addNegativeProbabilitiesFor(i, negativeProbabilities);
        }
        _T_index[T] = static_cast<unsigned int>(i); // map T to index in HG table
        ++i;
    }
}

/** Adds negative probabilities for a given index T, reducing the storage by removing PROBABILITY_UNSET values. */
void HypergeometricProbabilityLookup::SpatialCases::addNegativeProbabilitiesFor(size_t idx_T, const std::vector<double>& np) {
    // We're expecting idx_T to be added in order, so we can just check the size.
    if (_probabilities_of_x_at_T.size() != idx_T)
        throw prg_error("SpatialCases::addFor: idx_T out of order.", "HypergeometricProbabilityLookup::SpatialCases");
    // Figure out what will be the zero-based index for x.
    size_t zero_base = std::distance(np.begin(), std::find_if_not(np.begin(), np.end(), [](double d) {return d == PROBABILITY_UNSET; }));
    // Figure out how big the array will be after removing PROBABILITY_UNSET values.
    auto setnp_front = np.begin();
    for (; setnp_front != np.end(); ) {
        if (*setnp_front != PROBABILITY_UNSET) break;
        ++setnp_front;
    }
    auto in_front = std::distance(np.begin(), setnp_front);
    auto setnp_back = np.rbegin();
    for (; setnp_back != np.rend(); ) {
        if (*setnp_back != PROBABILITY_UNSET) break;
        ++setnp_back;
    }
    auto in_back = std::distance(setnp_back.base(), np.rbegin().base());
    // Emplace the data to the back of vector - which is at index idx_T.
    _probabilities_of_x_at_T.emplace_back(zero_base, MinimalGrowthArray<double>(np.size() - in_front - in_back, PROBABILITY_UNSET));
    auto& data = _probabilities_of_x_at_T.back().second;
    int i = 0;
    for (std::vector<double>::const_iterator it = setnp_front; it != (setnp_front + data.size()); ++it, ++i)
        data[i] = *it;
}

/** Prints HG lookup table to screen. */
void HypergeometricProbabilityLookup::printHG(const std::set<count_t>& caseWindows) const {
    std::ofstream outfile("HG_lookup.txt");
    for (auto T : caseWindows) {
        if (!T) continue;
        outfile << std::endl;
        for (unsigned int S = 2; S < _spatial_cases.size(); ++S) {
            for (unsigned int x = 0; x < _spatial_cases.size(); ++x) {
                double probability = getProbabilityFor_Checked(T, S, x);
                outfile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);
                if (probability == PROBABILITY_UNSET) {
                    outfile << "T = " << T << ", S = " << S << ", x = " << x << ", HG = " << PROBABILITY_UNSET << " (unset)" << std::endl;
                } else {
                    outfile << "T = " << T << ", S = " << S << ", x = " << x << ", HG = " << probability << " : ";
                    outfile << std::scientific << probability << std::endl;
                }
            }
            outfile << std::endl;
        }
        outfile << std::endl << std::endl;
    }
}
