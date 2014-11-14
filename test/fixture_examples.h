#ifndef __FIXTURE_EXAMPLES_H
#define __FIXTURE_EXAMPLES_H

// project files
#include "fixture_prm_source.h"

struct poisson_fixture : prm_examples_fixture {
    poisson_fixture() : prm_examples_fixture("Poisson.prm") { }
    virtual ~poisson_fixture() { }
};
struct bernoulli_fixture : prm_examples_fixture {
    bernoulli_fixture() : prm_examples_fixture("Bernoulli.prm") { }
    virtual ~bernoulli_fixture() { }
};
struct tree_temporal_fixture : prm_examples_fixture {
    tree_temporal_fixture() : prm_examples_fixture("TreeTemporal.prm") { }
    virtual ~tree_temporal_fixture() { }
};
#endif
