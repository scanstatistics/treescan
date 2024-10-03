
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "BasePrint.h"
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

/** Test Suite for variable case probability -- https://www.squishlist.com/ims/treescan/220/. */
BOOST_AUTO_TEST_SUITE( test_variable_case_suite )

/** Tests the relative risk calculation when using self-control design. */
BOOST_FIXTURE_TEST_CASE( test_relative_risk_self_control, bernoulli_fixture) {
    ScanRunner runner(_parameters, _print);
    auto alternativeAlgorithm = [](const ScanRunner& scanner, int nodeID, int _C, const MatchedSets& matchedsets, double converge) {
        // This is the initial algorithm Martin provided. When it successfully converges, the value is correct.
        // Use this as a check on new algorithm.
        double c = _C, n = static_cast<double>(matchedsets.get().size());
        double RR = 0.0;
        // Step 1
        auto a = [](double probability) { return (1.0 / probability) - 1.0; };
        auto b = [](double probability) { return probability / (1.0 - probability); };
        // Step 2
        double A = 0.0, B = 0.0;
        for (auto probability : matchedsets.get()) {
            A += a(probability);
            B += b(probability);
        }
        // Step 3
        double X1 = (c / B) / ((n - c) / n), X2 = (c / n) / ((n - c) / A);
        auto getZ = [c, &matchedsets, &a, &b](double X1, double X2) {
            // Step 4
            double sumX1 = 0.0, sumX2 = 0.0;
            double xx = 0;
            for (auto probability : matchedsets.get()) {
                sumX1 += (probability * X1) / (1.0 - probability + probability * X1);
                sumX2 += X2 / (X2 + a(probability));
            }
            // Step 5
            double Y1 = c - sumX1, Y2 = c - sumX2;
            // Step 6a - return Z
            return X1 + Y1 * (X2 - X1) / (Y1 - Y2);
        };
        unsigned int iterations = 0, maxiterations = 30;
        bool keepgoing = true;
        do {
            if (std::abs(X1 - X2) < converge) {
                RR = X1;
                break;
            }
            double S = 0.0, Z = 0.0;
            // Step 6b
            Z = getZ(X1, X2);
            // Step 7 / 8
            for (auto probability : matchedsets.get())
                S += Z / (Z + a(probability));
            // Step 9
            if (std::abs(S - c) < converge) {
                RR = Z;
                keepgoing = false;
            } else {
                X1 = Z;
            }
            ++iterations;
        } while (keepgoing && iterations < maxiterations);
        if (iterations >= maxiterations)
            return -1.0;
        return RR;
    };
    auto testMatchSet = [&runner, &alternativeAlgorithm](int c, const MatchedSets& matchedsets, double expectedRR) {
        double rr = getRelativeRiskFor(runner, 0, c, matchedsets);
        if (rr == std::numeric_limits<double>::infinity())
            BOOST_CHECK(expectedRR == std::numeric_limits<double>::infinity());
        else {
            BOOST_REQUIRE_CLOSE(expectedRR, rr, 0.001);
            double rrAlternative = alternativeAlgorithm(runner, 0, c, matchedsets, 0.00001);
            if (rrAlternative >= 0)
                BOOST_REQUIRE_CLOSE(expectedRR, rrAlternative, 0.001);
        }
    };
    testMatchSet(4, MatchedSets({ 0.2, 0.2, 0.2, 0.2, 0.2, 0.2 }), 8.0);
    testMatchSet(4, MatchedSets({ 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 }), 2.0);
    testMatchSet(4, MatchedSets({ 0.5, 0.5, 0.2, 0.2, 0.2, 0.2 }), 5.4641);
    testMatchSet(4, MatchedSets({ 0.9, 0.12, 0.66, 0.71, 0.09, 0.01 }), 9.05949);
    testMatchSet(4, MatchedSets({ 0.01, 0.02, 0.03, 0.04, 0.05, 0.06 }), 68.0217);
    testMatchSet(4, MatchedSets({ 0.360, 0.976, 0.548, 0.534, 0.950, 0.109 }), 1.79808);
    testMatchSet(4, MatchedSets({ 0.052, 0.048, 0.028, 0.037, 0.089, 0.021 }), 48.2798);
    testMatchSet(4, MatchedSets({ 0.067, 0.081, 0.04, 0.04, 0.025, 0.023 }), 48.0182);
    testMatchSet(4, MatchedSets({ 0.097, 0.066, 0.02, 0.011, 0.015, 0.063 }), 65.1732);
    testMatchSet(4, MatchedSets({ 0.017, 0.093, 0.008, 0.096, 0.029, 0.065 }), 59.5466);
    testMatchSet(4, MatchedSets({ 0.087, 0.041, 0.096, 0.1, 0.052, 0.026 }), 32.4722);
    testMatchSet(4, MatchedSets({ 0.001, 0.067, 0.033, 0.014, 0.065, 0.086 }), 87.8199);
    testMatchSet(4, MatchedSets({ 0.088, 0.007, 0.043, 0.016, 0.053, 0.055 }), 64.371);
    testMatchSet(1, MatchedSets({ 0.2, 0.2}), 4.0);
    testMatchSet(1, MatchedSets({ 0.002, 0.002 }), 499.0);
    testMatchSet(10, MatchedSets(
        { 0.046, 0.039, 0.087, 0.093, 0.093, 0.042, 0.085, 0.093, 0.018, 0.083, 0.043, 0.076, 0.038, 0.009, 0.054, 0.098, 0.086, 0.056, 0.046, 0.001, 0.071, 0.064, 0.058, 0.007, 0.009, 0.023, 0.029, 0.059, 0.048, 0.059, 0.091, 0.053, 0.036, 0.098, 0.01, 0.043, 0.02, 0.093, 0.088, 0.034, 0.095, 0.051, 0.052, 0.032, 0.067, 0.005, 0.054, 0.087, 0.048, 0.079, 0.008, 0.022, 0.093, 0.009, 0.077, 0.033, 0.097, 0.042, 0.066, 0.05, 0.06, 0.063, 0.003, 0.037, 0.035, 0.015, 0.035, 0.042, 0.075, 0.014, 0.0, 0.063, 0.047, 0.038, 0.028, 0.004, 0.083, 0.04, 0.012, 0.017, 0.06, 0.061, 0.02, 0.035, 0.007, 0.033, 0.005, 0.052, 0.025, 0.042, 0.062, 0.082, 0.013, 0.08, 0.029, 0.074, 0.077, 0.078, 0.094, 0.1 }
    ), 2.15899);
    testMatchSet(50, MatchedSets(
        { 0.03, 0.05, 0.09, 0.01, 0.05, 0.05, 0.05, 0.04, 0.06, 0.1, 0.09, 0.0, 0.02, 0.04, 0.07, 0.01, 0.06, 0.02, 0.01, 0.06, 0.07, 0.04, 0.01, 0.09, 0.01, 0.03, 0.09, 0.1, 0.09, 0.03, 0.06, 0.09, 0.02, 0.02, 0.08, 0.06, 0.09, 0.06, 0.03, 0.08, 0.06, 0.06, 0.08, 0.04, 0.01, 0.03, 0.1, 0.0, 0.09, 0.02, 0.08, 0.08, 0.01, 0.0, 0.09, 0.01, 0.07, 0.06, 0.02, 0.02, 0.08, 0.06, 0.04, 0.03, 0.09, 0.03, 0.09, 0.0, 0.07, 0.08, 0.0, 0.01, 0.03, 0.09, 0.01, 0.04, 0.03, 0.06, 0.08, 0.07, 0.03, 0.02, 0.05, 0.06, 0.03, 0.07, 0.06, 0.02, 0.05, 0.01, 0.09, 0.07, 0.03, 0.01, 0.09, 0.04, 0.09, 0.05, 0.0, 0.07 }
    ), 25.2159);
    testMatchSet(29, MatchedSets(
        { 0.02, 0.041, 0.062, 0.067, 0.089, 0.006, 0.073, 0.07, 0.067, 0.007, 0.06, 0.087, 0.017, 0.095, 0.024, 0.029, 0.093, 0.05, 0.06, 0.079, 0.071, 0.082, 0.07, 0.037, 0.085, 0.018, 0.068, 0.046, 0.078, 0.081, 0.01, 0.0, 0.032, 0.028, 0.04, 0.042, 0.051, 0.019, 0.045, 0.075, 0.054, 0.063, 0.052, 0.019, 0.017, 0.009, 0.097, 0.054, 0.094 }
    ), 34.2715);
    testMatchSet(1, MatchedSets({ 0.9, 0.8, 0.7, 0.6, 0.5, 0.4 }), 0.0748793);
    testMatchSet(3, MatchedSets(
        { 0.283, 0.886, 0.295, 0.564, 0.353, 0.732, 0.71, 0.014, 0.937, 0.817, 0.232, 0.347, 0.066, 0.575, 0.649, 0.981, 0.221, 0.69, 0.375, 0.673, 0.759, 0.504, 0.371, 0.803, 0.563, 0.146, 0.275, 0.053, 0.659, 0.62, 0.352, 0.707, 0.559, 0.499, 0.174, 0.913, 0.511, 0.802, 0.981, 0.992, 0.612, 0.071, 0.839, 0.173, 0.668, 0.695, 0.213, 0.931, 0.963 }
    ), 0.0134694);
    testMatchSet(3, MatchedSets({ 0.844, 0.878, 0.887, 0.833, 0.874, 0.859, 0.896, 0.863, 0.869, 0.821, 0.83, 0.826, 0.872, 0.804, 0.851, 0.886, 0.843, 0.875, 0.895, 0.851, 0.876, 0.822, 0.9, 0.875, 0.836, 0.845, 0.826, 0.856, 0.835, 0.885, 0.885, 0.851, 0.897, 0.86, 0.847, 0.862, 0.841, 0.814, 0.843, 0.831, 0.855, 0.822, 0.848, 0.849, 0.812, 0.803, 0.881, 0.82, 0.878 }
    ), 0.0108204);
    testMatchSet(6, MatchedSets({ 0.9, 0.8, 0.7, 0.6, 0.5, 0.4 }), std::numeric_limits<double>::infinity());
    testMatchSet(0, MatchedSets({ 0.9, 0.8, 0.7, 0.6, 0.5, 0.4 }), 0.0);
    testMatchSet(3, MatchedSets({ 0.000019, 0.00002, 0.00001, 0.00005, 0.00007, 0.00007, 0.00007, 0.00003, 0.00003, 0.00003 }), 11847.22574);
}

BOOST_AUTO_TEST_SUITE_END()