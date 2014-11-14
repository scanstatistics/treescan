
// project files
#include "fixture_examples.h"

/* Test Suite for the Parameters validation class. */
BOOST_AUTO_TEST_SUITE( sampledata_validation_suite )

/* Tests parameters validation of poisson example date set. */
BOOST_AUTO_TEST_CASE( test_example_poisson ) {
    poisson_fixture f;
}

/* Tests parameters validation of bernoulli example date set. */
BOOST_AUTO_TEST_CASE( test_example_bernoulli ) {
    bernoulli_fixture f;
}

/* Tests parameters validation of tree temporal example date set. */
BOOST_AUTO_TEST_CASE( test_sample_NYCfever ) {
    tree_temporal_fixture f;
}

BOOST_AUTO_TEST_SUITE_END()
