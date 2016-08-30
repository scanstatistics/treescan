
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

struct poisson_multiple_trees_fixture : prm_testset_fixture {
    poisson_multiple_trees_fixture() : prm_testset_fixture("/multiple-trees/poisson.prm") { }
    virtual ~poisson_multiple_trees_fixture() {}

    std::string _results_user_directory;
};

class testRunner : public ScanRunner {
    public:
        testRunner(const Parameters& Parameters, BasePrint& PrintDirection)
            : ScanRunner(Parameters, PrintDirection) {}
        virtual ~testRunner() {}

		void read_tree_file_test_counts() {
			for (Parameters::FileNameContainer_t::const_iterator itr=_parameters.getTreeFileNames().begin(); itr != _parameters.getTreeFileNames().end(); ++itr) {
				if (!readTree(*itr, static_cast<unsigned int>(std::distance(_parameters.getTreeFileNames().begin(), itr) + 1)))
					throw resolvable_error("\nProblem encountered when reading the data from the tree file: \n%s.", itr->c_str());
			}
			std::for_each(_Nodes.begin(), _Nodes.end(), std::mem_fun(&NodeStructure::assignLevel));

			BOOST_CHECK_EQUAL(37 , _Nodes.size());
			std::vector<unsigned int> levels;
	        levels += 3,3,3,3,4,4,3,3,3,3,3,3,3,3,2,2,2,2,2,3,4,5,6,7,3,8,3,3,2,2,3,3,3,1,1,1,1;
			BOOST_CHECK_EQUAL(37 , levels.size());
			for (size_t t=0; t < levels.size(); ++t) {
				BOOST_CHECK_EQUAL(levels[t], _Nodes[t]->getLevel());

				/* check that  level matches smallest parents level */
				if (_Nodes[t]->getParents().size()) {
					unsigned int parent_level = std::numeric_limits<unsigned int>::max();
					for (size_t s=0; s < _Nodes[t]->getParents().size(); ++s) {
						parent_level = std::min(parent_level, _Nodes[t]->getParents()[s]->getLevel());
					}
					BOOST_CHECK_EQUAL(parent_level + 1, _Nodes[t]->getLevel());
				} else {
					BOOST_CHECK_EQUAL(1, _Nodes[t]->getLevel());
				}
			}
		}
};

/* Test Suite for the multiple trees testing. */
BOOST_AUTO_TEST_SUITE( multiple_trees_suite )

/* Tests that node levels are calculated to expected values given multiple trees and multiple parents. */
BOOST_FIXTURE_TEST_CASE( test_node_levels, poisson_multiple_trees_fixture ) {
    // set results file to the user document directory
    std::stringstream filename;
    filename << GetUserTemporaryDirectory(_results_user_directory).c_str() << "\\test.txt";
    _parameters.setOutputFileName(filename.str().c_str());

	testRunner(_parameters, _print).read_tree_file_test_counts();
}

BOOST_AUTO_TEST_SUITE_END()
