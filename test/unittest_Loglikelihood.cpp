// Boost unit test header
#include <boost/test/unit_test.hpp>

#include "Loglikelihood.h"

/** Test Suite for the AbstractLoglikelihood class. */
BOOST_AUTO_TEST_SUITE( test_abstract_loglikelihood )

BOOST_AUTO_TEST_CASE( test_get_newloglikelihood_treeonly_scan_poisson_model ) {
    Parameters parameters;

    parameters.setScanType(Parameters::TREEONLY);
    parameters.setConditionalType(Parameters::UNCONDITIONAL);
    parameters.setModelType(Parameters::POISSON);

    BOOST_CHECK( dynamic_cast<UnconditionalPoissonLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    parameters.setConditionalType(Parameters::TOTALCASES);
    BOOST_CHECK( dynamic_cast<PoissonLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    parameters.setConditionalType(Parameters::NODE);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);

    parameters.setConditionalType(Parameters::NODEANDTIME);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);
}

BOOST_AUTO_TEST_CASE( test_get_newloglikelihood_treeonly_scan_bernoulli_model ) {
    Parameters parameters;

    parameters.setScanType(Parameters::TREEONLY);
    parameters.setConditionalType(Parameters::UNCONDITIONAL);
    parameters.setModelType(Parameters::BERNOULLI_TREE);

    BOOST_CHECK( dynamic_cast<UnconditionalBernoulliLogLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    parameters.setConditionalType(Parameters::TOTALCASES);
    BOOST_CHECK( dynamic_cast<BernoulliLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    parameters.setConditionalType(Parameters::NODE);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);

    parameters.setConditionalType(Parameters::NODEANDTIME);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);
}

BOOST_AUTO_TEST_CASE( test_get_newloglikelihood_treetime_scan ) {
    Parameters parameters;

    // tree-time scan, conditioned on node, uniform model
    parameters.setScanType(Parameters::TREETIME);
    parameters.setConditionalType(Parameters::NODE);
    parameters.setModelType(Parameters::UNIFORM);
    BOOST_CHECK( dynamic_cast<TemporalLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    // tree-time scan, conditioned on node, poisson/bernoulli/not-applicable models
    parameters.setModelType(Parameters::POISSON);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);
    parameters.setModelType(Parameters::BERNOULLI_TREE);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);
    parameters.setModelType(Parameters::MODEL_NOT_APPLICABLE);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);

    // tree-time scan, conditioned on node, uniform model, day of week adjustment
    parameters.setModelType(Parameters::UNIFORM);
    parameters.setPerformDayOfWeekAdjustment(true);
    BOOST_CHECK( dynamic_cast<PoissonLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    // tree-time scan, conditioned on node and time
    parameters.setPerformDayOfWeekAdjustment(false);
    parameters.setConditionalType(Parameters::NODEANDTIME);
    parameters.setModelType(Parameters::MODEL_NOT_APPLICABLE);
    BOOST_CHECK( dynamic_cast<PoissonLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    // tree-time scan, conditioned on node and time, day of week adjustment
    parameters.setConditionalType(Parameters::NODEANDTIME);
    parameters.setModelType(Parameters::MODEL_NOT_APPLICABLE);
    parameters.setPerformDayOfWeekAdjustment(true);
    BOOST_CHECK( dynamic_cast<PoissonLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );
}

BOOST_AUTO_TEST_CASE( test_get_newloglikelihood_timeonly_scan ) {
    Parameters parameters;

    // time-only scan, conditioned on node, uniform model
    parameters.setScanType(Parameters::TIMEONLY);
    parameters.setConditionalType(Parameters::TOTALCASES);
    parameters.setModelType(Parameters::UNIFORM);
    BOOST_CHECK( dynamic_cast<TemporalLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    // time-only scan, conditioned on node, poisson/bernoulli/not-applicable models
    parameters.setModelType(Parameters::POISSON);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);
    parameters.setModelType(Parameters::BERNOULLI_TREE);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);
    parameters.setModelType(Parameters::MODEL_NOT_APPLICABLE);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false), prg_error);

    // time-only scan, conditioned on node, uniform model, day of week adjustment
    parameters.setModelType(Parameters::UNIFORM);
    parameters.setPerformDayOfWeekAdjustment(true);
    BOOST_CHECK( dynamic_cast<PoissonLoglikelihood*>(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0, false)) != 0 );

    // time-only scan, conditioned on node and time
    /*parameters.setPerformDayOfWeekAdjustment(false);
    parameters.setConditionalType(Parameters::NODEANDTIME);
    BOOST_REQUIRE_THROW(AbstractLoglikelihood::getNewLoglikelihood(parameters, 100, 100.0), prg_error);*/
}
BOOST_AUTO_TEST_SUITE_END()

/** Test Suite for the PoissonLoglikelihood class. */
BOOST_AUTO_TEST_SUITE( test_poisson_loglikelihood )

/** Tests PoissonLoglikelihood::LogLikelihood with equal observed and expected. */
BOOST_AUTO_TEST_CASE( test_equal_observed_expected ) {
    Parameters parameters;
    PoissonLoglikelihood l(100, 100.0, parameters);
    BOOST_CHECK_EQUAL( l.LogLikelihood(0, 0.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(1, 1.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(10, 10.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(50, 50.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(100, 100.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
}

/** Tests PoissonLoglikelihood::LogLikelihood with more observed than expected. */
BOOST_AUTO_TEST_CASE( test_more_observed_than_expected ) {
    Parameters parameters;
    BOOST_CHECK_CLOSE( PoissonLoglikelihood(100, 100.0, parameters).LogLikelihood(12, 5.35), 3.2830035, 0.0001 );
}

/** Tests PoissonLoglikelihood::LogLikelihood with less observed than expected. */
BOOST_AUTO_TEST_CASE( test_less_observed_than_expected ) {
    Parameters parameters;
    BOOST_CHECK_EQUAL( PoissonLoglikelihood(100, 100.0, parameters).LogLikelihood(12, 15.35), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
}

/** Tests PoissonLoglikelihood::LogLikelihood with observed equal to total cases. */
BOOST_AUTO_TEST_CASE( test_observed_equal_totalcases ) {
    Parameters parameters;
    BOOST_CHECK_CLOSE( PoissonLoglikelihood(100, 100.0, parameters).LogLikelihood(100, 50.0), 69.314718, 0.0001 );
}

/** Tests PoissonLoglikelihood::LogLikelihoodRatio. */
BOOST_AUTO_TEST_CASE( test_loglikelihoodratio ) {
    Parameters parameters;
    PoissonLoglikelihood l(100, 100.0, parameters);
    BOOST_CHECK_EQUAL( l.LogLikelihoodRatio(AbstractLoglikelihood::UNSET_LOGLIKELIHOOD), 0.0 );
    BOOST_CHECK_CLOSE( l.LogLikelihoodRatio(10.13851), 10.13851, 0.0001 );
}

BOOST_AUTO_TEST_SUITE_END()

/** Test Suite for the UnconditionalPoissonLoglikelihood class. */
BOOST_AUTO_TEST_SUITE( test_unconditional_poisson_loglikelihood )

/** Tests PoissonLoglikelihood::LogLikelihood with equal observed and expected. */
BOOST_AUTO_TEST_CASE( test_equal_observed_expected ) {
    Parameters parameters;
    UnconditionalPoissonLoglikelihood l(parameters);
    BOOST_CHECK_EQUAL( l.LogLikelihood(0, 0.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(1, 1.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(10, 10.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(50, 50.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(100, 100.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
}

/** Tests PoissonLoglikelihood::LogLikelihood with more observed than expected. */
BOOST_AUTO_TEST_CASE( test_more_observed_than_expected ) {
    Parameters parameters;
    BOOST_CHECK_CLOSE( UnconditionalPoissonLoglikelihood(parameters).LogLikelihood(12, 5.35), 3.043721, 0.0001 );
}

/** Tests PoissonLoglikelihood::LogLikelihood with less observed than expected. */
BOOST_AUTO_TEST_CASE( test_less_observed_than_expected ) {
    Parameters parameters;
    BOOST_CHECK_EQUAL( UnconditionalPoissonLoglikelihood(parameters).LogLikelihood(12, 15.35), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
}

/** Tests PoissonLoglikelihood::LogLikelihood with observed equal to total cases. */
BOOST_AUTO_TEST_CASE( test_observed_equal_totalcases ) {
    Parameters parameters;
    BOOST_CHECK_CLOSE( UnconditionalPoissonLoglikelihood(parameters).LogLikelihood(100, 50.0), 19.314718, 0.0001 );
}

/** Tests PoissonLoglikelihood::LogLikelihoodRatio. */
BOOST_AUTO_TEST_CASE( test_loglikelihoodratio ) {
    Parameters parameters;
    UnconditionalPoissonLoglikelihood l(parameters);
    BOOST_CHECK_EQUAL( l.LogLikelihoodRatio(AbstractLoglikelihood::UNSET_LOGLIKELIHOOD), 0.0 );
    // should return the same value
    BOOST_CHECK_EQUAL( l.LogLikelihoodRatio(10.13851), 10.13851 );
}

BOOST_AUTO_TEST_SUITE_END()

/** Test Suite for the BernoulliLoglikelihood class. */
BOOST_AUTO_TEST_SUITE( test_bernoulli_loglikelihood )

/** Tests BernoulliLoglikelihood::LogLikelihood with equal observed and expected. */
BOOST_AUTO_TEST_CASE( test_equal_observed_expected ) {
    Parameters parameters;
    BernoulliLoglikelihood l(100, 500.0, parameters);
    BOOST_CHECK_EQUAL( l.LogLikelihood(0, 0.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(1, 1.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_CLOSE( l.LogLikelihood(10, 10.0), -233.689952, 0.0001 );
    BOOST_CHECK_CLOSE( l.LogLikelihood(50, 50.0), -156.974443, 0.0001 );
    BOOST_CHECK_EQUAL( l.LogLikelihood(100, 100.0), 0.0 );
}

/** Tests BernoulliLoglikelihood::LogLikelihood with less observed than expected. */
BOOST_AUTO_TEST_CASE( test_less_observed_than_expected ) {
    Parameters parameters;
    BOOST_CHECK_CLOSE( BernoulliLoglikelihood(100, 500.0, parameters).LogLikelihood(12, 15.35), -237.667492, 0.0001 );
}

/** Tests BernoulliLoglikelihood::LogLikelihood with less observed than expected. */
BOOST_AUTO_TEST_CASE( test_less_rate_than_set_totals ) {
    Parameters parameters;
    BOOST_CHECK_EQUAL( BernoulliLoglikelihood(100, 500.0, parameters).LogLikelihood(1, 7.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
}

/** Tests BernoulliLoglikelihood::LogLikelihood with observed equal to total cases. */
BOOST_AUTO_TEST_CASE( test_observed_equal_totalcases ) {
    Parameters parameters;
    BOOST_CHECK_CLOSE( BernoulliLoglikelihood(100, 500.0, parameters).LogLikelihood(100, 150.0), -95.477125, 0.0001 );
}

/** Tests BernoulliLoglikelihood::LogLikelihoodRatio. */
BOOST_AUTO_TEST_CASE( test_loglikelihoodratio ) {
    Parameters parameters;
    BernoulliLoglikelihood l(100, 500.0, parameters);
    BOOST_CHECK_EQUAL( l.LogLikelihoodRatio(AbstractLoglikelihood::UNSET_LOGLIKELIHOOD), 0.0 );
    BOOST_CHECK_CLOSE( l.LogLikelihoodRatio(-10.13851), 240.062701, 0.0001 );
}

BOOST_AUTO_TEST_SUITE_END()

/** Test Suite for the UnconditionalBernoulliLogLoglikelihood class. */
BOOST_AUTO_TEST_SUITE( test_unconditional_bernoulli_loglikelihood )

/** Tests UnconditionalBernoulliLogLoglikelihood::LogLikelihood with equal observed and expected. */
BOOST_AUTO_TEST_CASE( test_equal_observed_expected ) {
    Parameters parameters;
    UnconditionalBernoulliLogLoglikelihood l(parameters);
    BOOST_CHECK_EQUAL( l.LogLikelihood(0, 0.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
    BOOST_CHECK_EQUAL( l.LogLikelihood(1, 1.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD);
    BOOST_CHECK_CLOSE( l.LogLikelihood(10, 10.0), 6.931471, 0.0001 );
    BOOST_CHECK_CLOSE( l.LogLikelihood(50, 50.0), 34.657359, 0.0001 );
    BOOST_CHECK_CLOSE( l.LogLikelihood(50, 50.0), 34.657359, 0.0001 );
    BOOST_CHECK_CLOSE( l.LogLikelihood(100, 100.0), 69.314718, 0.0001 );
}

/** Tests UnconditionalBernoulliLogLoglikelihood::LogLikelihood with less observed than expected. */
BOOST_AUTO_TEST_CASE( test_less_observed_than_expected ) {
    Parameters parameters;
    BOOST_CHECK_CLOSE( UnconditionalBernoulliLogLoglikelihood(parameters).LogLikelihood(12, 15.35), 2.586083, 0.0001 );
}

/** Tests UnconditionalBernoulliLogLoglikelihood::LogLikelihood with less observed than expected. */
BOOST_AUTO_TEST_CASE( test_less_rate_than_set_totals ) {
    Parameters parameters;
    BOOST_CHECK_EQUAL( UnconditionalBernoulliLogLoglikelihood(parameters).LogLikelihood(1, 7.0), AbstractLoglikelihood::UNSET_LOGLIKELIHOOD );
}

/** Tests UnconditionalBernoulliLogLoglikelihood::LogLikelihood with observed equal to total cases. */
BOOST_AUTO_TEST_CASE( test_observed_equal_totalcases ) {
    Parameters parameters;
    BOOST_CHECK_CLOSE( UnconditionalBernoulliLogLoglikelihood(parameters).LogLikelihood(100, 150.0), 8.494951, 0.0001 );
}

/** Tests UnconditionalBernoulliLogLoglikelihood::LogLikelihoodRatio. */
BOOST_AUTO_TEST_CASE( test_loglikelihoodratio ) {
    Parameters parameters;
    UnconditionalBernoulliLogLoglikelihood l(parameters);
    BOOST_CHECK_EQUAL( l.LogLikelihoodRatio(AbstractLoglikelihood::UNSET_LOGLIKELIHOOD), 0.0 );
    BOOST_CHECK_CLOSE( l.LogLikelihoodRatio(-10.13851), -10.13851, 0.0001 );
}

BOOST_AUTO_TEST_SUITE_END()

/** Test Suite for the PoissonCensoredLoglikelihood class. */
BOOST_AUTO_TEST_SUITE(test_poission_censored_loglikelihood)

/** Tests results with TemporalLoglikelihood with PoissonCensoredLoglikelihood. */
BOOST_AUTO_TEST_CASE( test_loglikelihood_to_temporal ) {
    Parameters parameters;
    parameters.setDataTimeRangeSet(DataTimeRangeSet("[1,28]", parameters.getDatePrecisionType(), boost::optional<boost::gregorian::date>()));
    parameters.setMaximumWindowType(Parameters::FIXED_LENGTH);
    parameters.setMaximumWindowLength(14);
    TemporalLoglikelihood temporal(98, 98.0, parameters);
    PoissonCensoredLoglikelihood censored(parameters);

    // For this test situation, there were 98 total cases, time period 1-28, 21 cases in window, 46 on node.
    BOOST_CHECK_CLOSE(temporal.LogLikelihood(21, 46, 2), censored.LogLikelihood(21, 46.0/28.0 * 2.0, 46, 46), 0.0001);
}

/** Tests PoissonCensoredLoglikelihood::LogLikelihoodRatio. */
BOOST_AUTO_TEST_CASE(test_loglikelihoodratio) {
    Parameters parameters;
    PoissonCensoredLoglikelihood l(parameters);
    BOOST_CHECK_EQUAL(l.LogLikelihoodRatio(AbstractLoglikelihood::UNSET_LOGLIKELIHOOD), 0.0);
    BOOST_CHECK_CLOSE(l.LogLikelihoodRatio(-10.13851), -10.13851, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()
