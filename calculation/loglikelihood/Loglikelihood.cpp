//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "Loglikelihood.h"

double AbstractLoglikelihood::UNSET_LOGLIKELIHOOD = -std::numeric_limits<double>::max();

/** returns new randomizer given parameter settings. */
AbstractLoglikelihood * AbstractLoglikelihood::getNewLoglikelihood(const Parameters& parameters, int TotalC, double TotalN) {
    switch (parameters.getModelType()) {
        case Parameters::POISSON: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL : return new UnconditionalPoissonLoglikelihood();
                case Parameters::TOTALCASES : return new PoissonLoglikelihood(TotalC, TotalN);
                case Parameters::CASESEACHBRANCH :
                default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
            }
        } break;
        case Parameters::BERNOULLI: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL : new UnconditionalBernoulliLogLoglikelihood(parameters.getProbability());
                case Parameters::TOTALCASES : return new BernoulliLoglikelihood(TotalC, TotalN);
                case Parameters::CASESEACHBRANCH :
                default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
            }
        } break;
        case Parameters::TEMPORALSCAN: { 
            switch (parameters.getConditionalType()) {
                case Parameters::CASESEACHBRANCH : return new TemporalLoglikelihood(TotalC, TotalN, parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
                case Parameters::UNCONDITIONAL :
                case Parameters::TOTALCASES :
                default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
            }
        } break;
        default: throw prg_error("Unknown model type (%d).", "getNewLoglikelihood()", parameters.getModelType());
    }
}
