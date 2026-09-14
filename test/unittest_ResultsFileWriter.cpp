// Boost unit test header
#include <boost/test/unit_test.hpp>

#include "ResultsFileWriter.h"
#include "TreeScan.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace {

    /** Builds a RecurrenceInterval_t from a number of years -- the second element is the equivalent number of days,
        which is how ScanRunner::getRecurrenceInterval() pairs the two values. */
    RecurrenceInterval_t recurrenceIntervalOfYears(double years) {
        return std::make_pair(years, std::max(years * AVERAGE_DAYS_IN_YEAR, 1.0));
    }

    /** Returns the formatted recurrence interval for the passed number of years. */
    std::string formatYears(double years) {
        std::string buffer;
        return ResultsFileWriter::getRecurranceIntervalAsString(recurrenceIntervalOfYears(years), buffer);
    }

    /** Returns the number of digits reported after the decimal point of the leading numeric portion of text. */
    unsigned int reportedDecimalPlaces(const std::string& text) {
        std::string::size_type period = text.find('.');
        if (period == std::string::npos) return 0;
        unsigned int places = 0;
        for (std::string::size_type i = period + 1; i < text.size() && isdigit(static_cast<unsigned char>(text[i])); ++i)
            ++places;
        return places;
    }
}

/** Test suite for the precision reported by ResultsFileWriter::getRecurranceIntervalAsString().
    Expected precision of the reported recurrence interval:
      - 10 years or more -- no decimal places
      - less than 10 years -- one decimal place, with a zero tenths digit */
BOOST_AUTO_TEST_SUITE(results_file_writer_recurrence_interval_precision_suite)

/** Recurrence intervals of ten or more are reported as whole years. */
BOOST_AUTO_TEST_CASE(reports_no_decimal_places_at_ten_years_or_more) {
    const double years[] = { 10.6, 45.9, 65.0, 100.0, 100.4, 123.45, 365.25, 999.9, 1000.0, 9999.9 };
    for (size_t i = 0; i < sizeof(years) / sizeof(years[0]); ++i) {
        std::string reported(formatYears(years[i]));
        BOOST_CHECK_MESSAGE(
            reportedDecimalPlaces(reported) == 0,
            "expected no decimal places for " << years[i] << " years, got '" << reported << "'"
        );
    }
}

/** Ten years, with consideration to rounding, is the value reported without decimal places. */
BOOST_AUTO_TEST_CASE(ten_years_is_the_boundary_for_dropping_decimal_places) {
    BOOST_CHECK_EQUAL(formatYears(9.89), std::string("9.9 years"));
    BOOST_CHECK_EQUAL(formatYears(9.9), std::string("9.9 years"));
    BOOST_CHECK_EQUAL(formatYears(9.90), std::string("9.9 years"));
    BOOST_CHECK_EQUAL(formatYears(9.91), std::string("9.9 years"));
    BOOST_CHECK_EQUAL(formatYears(9.94), std::string("9.9 years"));
    BOOST_CHECK_EQUAL(formatYears(9.949), std::string("9.9 years"));
    BOOST_CHECK_EQUAL(formatYears(9.95), std::string("10 years"));
    BOOST_CHECK_EQUAL(formatYears(9.96), std::string("10 years"));
    BOOST_CHECK_EQUAL(formatYears(9.97), std::string("10 years"));
    BOOST_CHECK_EQUAL(formatYears(9.98), std::string("10 years"));
    BOOST_CHECK_EQUAL(formatYears(9.99), std::string("10 years"));
    BOOST_CHECK_EQUAL(formatYears(10.0), std::string("10 years"));
    BOOST_CHECK_EQUAL(formatYears(10.4), std::string("10 years"));
}

/** Large recurrence intervals are humanized ('12 thousand years') and still report no decimal places. */
BOOST_AUTO_TEST_CASE(reports_no_decimal_places_for_humanized_magnitudes) {
    BOOST_CHECK_EQUAL(formatYears(10000.0), std::string("10 thousand years"));
    BOOST_CHECK_EQUAL(formatYears(12345.0), std::string("12 thousand years"));
    BOOST_CHECK_EQUAL(formatYears(1000000.0), std::string("1 million years"));
    BOOST_CHECK_EQUAL(formatYears(1000000000.0), std::string("1 billion years"));
    BOOST_CHECK_EQUAL(formatYears(1000000000000.0), std::string("1 trillion years"));
}

/** Recurrence intervals of less than ten years are reported to a tenth of a year. */
BOOST_AUTO_TEST_CASE(reports_tenths_of_a_year_below_ten_years) {
    const double years[] = { 1.1, 1.5, 2.34, 7.62, 9.9 };
    for (size_t i = 0; i < sizeof(years) / sizeof(years[0]); ++i) {
        std::string reported(formatYears(years[i]));
        BOOST_CHECK_MESSAGE(
            reportedDecimalPlaces(reported) == 1,
            "expected one decimal place for " << years[i] << " years, got '" << reported << "'"
        );
    }
    BOOST_CHECK_EQUAL(formatYears(2.34), std::string("2.3 years"));
    BOOST_CHECK_EQUAL(formatYears(7.62), std::string("7.6 years"));
    BOOST_CHECK_EQUAL(formatYears(9.9), std::string("9.9 years"));
}

/** A whole number of years below 10 reports the tenths digit, even zero. */
BOOST_AUTO_TEST_CASE(reports_a_zero_tenths_digit_for_whole_years_below_ten) {
    const double years[] = { 1.0, 2.0, 5.0, 9.0 };
    for (size_t i = 0; i < sizeof(years) / sizeof(years[0]); ++i) {
        std::string reported(formatYears(years[i]));
        BOOST_CHECK_MESSAGE(
            reportedDecimalPlaces(reported) == 1,
            "expected no decimal places for whole year value " << years[i] << ", got '" << reported << "'"
        );
    }
    BOOST_CHECK_EQUAL(formatYears(1.0), std::string("1.0 year"));
    BOOST_CHECK_EQUAL(formatYears(2.0), std::string("2.0 years"));
    BOOST_CHECK_EQUAL(formatYears(2.2), std::string("2.2 years"));
    BOOST_CHECK_EQUAL(formatYears(2.7), std::string("2.7 years"));
    BOOST_CHECK_EQUAL(formatYears(3.0), std::string("3.0 years"));
    BOOST_CHECK_EQUAL(formatYears(3.09), std::string("3.1 years"));
    BOOST_CHECK_EQUAL(formatYears(4.0), std::string("4.0 years"));
    BOOST_CHECK_EQUAL(formatYears(4.009), std::string("4.0 years"));
    BOOST_CHECK_EQUAL(formatYears(5.0), std::string("5.0 years"));
    BOOST_CHECK_EQUAL(formatYears(5.9), std::string("5.9 years"));
    BOOST_CHECK_EQUAL(formatYears(5.99), std::string("6.0 years"));
    BOOST_CHECK_EQUAL(formatYears(6.0), std::string("6.0 years"));
    BOOST_CHECK_EQUAL(formatYears(7.0), std::string("7.0 years"));
    BOOST_CHECK_EQUAL(formatYears(8.0), std::string("8.0 years"));
    BOOST_CHECK_EQUAL(formatYears(9.0), std::string("9.0 years"));
    BOOST_CHECK_EQUAL(formatYears(9.99), std::string("10 years"));
    BOOST_CHECK_EQUAL(formatYears(10.0), std::string("10 years"));
    BOOST_CHECK_EQUAL(formatYears(11.0), std::string("11 years"));
    BOOST_CHECK_EQUAL(formatYears(25.0), std::string("25 years"));
    BOOST_CHECK_EQUAL(formatYears(50.0), std::string("50 years"));
    BOOST_CHECK_EQUAL(formatYears(100.0), std::string("100 years"));
    BOOST_CHECK_EQUAL(formatYears(101.0), std::string("101 years"));
}

/** Recurrence intervals of less than one year are reported as whole days, not years. */
BOOST_AUTO_TEST_CASE(reports_whole_days_below_one_year) {
    BOOST_CHECK_EQUAL(formatYears(1.0 / AVERAGE_DAYS_IN_YEAR), std::string("1 day"));
    BOOST_CHECK_EQUAL(formatYears(0.5), std::string("183 days"));
    BOOST_CHECK_EQUAL(formatYears(0.99), std::string("362 days"));
}

BOOST_AUTO_TEST_SUITE_END()
