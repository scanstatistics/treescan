//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "Loglikelihood.h"

double AbstractLoglikelihood::UNSET_LOGLIKELIHOOD = -std::numeric_limits<double>::max();

/** returns new randomizer given parameter settings. */
AbstractLoglikelihood * AbstractLoglikelihood::getNewLoglikelihood(const Parameters& parameters, int TotalC, double TotalN, bool censored_data) {
    switch (parameters.getScanType()) {
        case Parameters::TREEONLY: {
            switch (parameters.getModelType()) {
                case Parameters::POISSON: {
                    switch (parameters.getConditionalType()) {
                        case Parameters::UNCONDITIONAL : return new UnconditionalPoissonLoglikelihood();
                        case Parameters::TOTALCASES : return new PoissonLoglikelihood(TotalC, TotalN);
                        default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
                    }
                } break;
                case Parameters::BERNOULLI: {
                    switch (parameters.getConditionalType()) {
                        case Parameters::UNCONDITIONAL : return new UnconditionalBernoulliLogLoglikelihood(parameters.getProbability());
                        case Parameters::TOTALCASES : return new BernoulliLoglikelihood(TotalC, TotalN);
                        default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
                    }
                } break;
                default: throw prg_error("Unknown model type (%d).", "getNewLoglikelihood()", parameters.getModelType());
            }
        } break;
        /* TODO:
           For the likelihood formula, I would like to try two different ones. Eventually we will only have one, unknown which,
           but I would like to have both available for my own use as a hidden feature in the parameter file.

           The first one will be the Poisson likelihood function, that we use for the Poisson model, but using the expected
           counts that I sent in the prior email. The second one will be a hypergeometric for the conditional tree-temporal scan.
           I need to work out the exact formulas for that, as well as for the ones adjusted for day-of-week.
        */
        case Parameters::TREETIME:
            if (parameters.getConditionalType() == Parameters::NODEANDTIME)
                return new PoissonLoglikelihood(TotalC, TotalN);
        case Parameters::TIMEONLY: {
            switch (parameters.getModelType()) {
                case Parameters::UNIFORM :
                    if (parameters.isPerformingDayOfWeekAdjustment()) {
                        /* TODO: Martin said to stud this log-likelihood for now. He needs to work through the correct function. */
                        return new PoissonLoglikelihood(TotalC, TotalN);
                    }
                    if (censored_data) {
                        return new PoissonCensoredLoglikelihood();
                    }
                    switch (parameters.getConditionalType()) {
                        case Parameters::TOTALCASES :
                        case Parameters::NODE :
                            if (!parameters.isPerformingDayOfWeekAdjustment())
                                return new TemporalLoglikelihood(TotalC, TotalN, parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets(), parameters.getMaximumWindowInTimeUnits());
                            return new PoissonLoglikelihood(TotalC, TotalN);
                        default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
                    }
                    break;
                default: throw prg_error("Unknown model type (%d).", "getNewLoglikelihood()", parameters.getModelType());
            }
        } break;
        default : throw prg_error("Unknown scan type (%d).", "getNewLoglikelihood()", parameters.getScanType());
    }
    return 0;
}
