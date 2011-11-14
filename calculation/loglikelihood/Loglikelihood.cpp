//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "Loglikelihood.h"

/** returns new randomizer given parameter settings. */
AbstractLoglikelihood * AbstractLoglikelihood::getNewLoglikelihood(const Parameters& parameters, int TotalC, double TotalN) {
    switch (parameters.getModelType()) {
    case Parameters::POISSON: 
        if (parameters.isConditional()) return new PoissonLoglikelihood(TotalC, TotalN);
        else return new UnconditionalPoissonLoglikelihood();
    case Parameters::BERNOULLI: 
        if (parameters.isConditional()) return new BernoulliLoglikelihood(TotalC, TotalN);
        else return new UnconditionalBernoulliLogLoglikelihood(parameters.getProbability());
    default: throw prg_error("Unknown model type (%d).", "getNewLoglikelihood()", parameters.getModelType());
    }
}
