//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "Loglikelihood.h"

double AbstractLoglikelihood::UNSET_LOGLIKELIHOOD = -std::numeric_limits<double>::max();

/** Returns new randomizer given parameter settings. */
AbstractLoglikelihood * AbstractLoglikelihood::getNewLoglikelihood(const Parameters& parameters, int TotalC, double TotalN, bool censored_data) {
    switch (parameters.getScanType()) {
        case Parameters::TREEONLY: {
            switch (parameters.getModelType()) {
                case Parameters::POISSON: {
                    switch (parameters.getConditionalType()) {
                        case Parameters::UNCONDITIONAL : return new UnconditionalPoissonLoglikelihood(parameters);
                        case Parameters::TOTALCASES : return new PoissonLoglikelihood(TotalC, TotalN, parameters);
                        default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
                    }
                } break;
                case Parameters::BERNOULLI_TREE: {
                    switch (parameters.getConditionalType()) {
                        case Parameters::UNCONDITIONAL : 
                            if (parameters.getVariableCaseProbability())
                                return new UnconditionalPoissonLoglikelihood(parameters);
                            return new UnconditionalBernoulliLogLoglikelihood(parameters);
                        case Parameters::TOTALCASES : return new BernoulliLoglikelihood(TotalC, TotalN, parameters);
                        default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
                    }
                } break;
                default: throw prg_error("Unknown model type (%d).", "getNewLoglikelihood()", parameters.getModelType());
            }
        } break;
        /** Martin's comments: 
            For the likelihood formula, I would like to try two different ones. Eventually we will only have one, unknown which.
            The first one will be the Poisson likelihood function, that we use for the Poisson model. 
            The second one will be a hypergeometric for the conditional tree-temporal scan. I need to work out the exact formulas
            for that, as well as for the ones adjusted for day-of-week (note second was never worked on).
        */
        case Parameters::TREETIME:
            if (parameters.getConditionalType() == Parameters::NODEANDTIME)
                return new PoissonLoglikelihood(TotalC, TotalN, parameters);
        case Parameters::TIMEONLY: {
            switch (parameters.getModelType()) {
                case Parameters::UNIFORM :
                    if (parameters.isPerformingDayOfWeekAdjustment()) {
                        // Martin said to stub this log-likelihood for now. He needs to work through the correct function.
                        return new PoissonLoglikelihood(TotalC, TotalN, parameters);
                    }
                    if (censored_data) {
                        return new PoissonCensoredLoglikelihood(parameters);
                    }
                    switch (parameters.getConditionalType()) {
                        case Parameters::TOTALCASES :
                        case Parameters::NODE :
                            if (!parameters.isPerformingDayOfWeekAdjustment())
                                return new TemporalLoglikelihood(TotalC, TotalN, parameters);
                            return new PoissonLoglikelihood(TotalC, TotalN, parameters);
                        default: throw prg_error("Unknown conditional type (%d).", "getNewLoglikelihood()", parameters.getConditionalType());
                    }
                    break;
				case Parameters::BERNOULLI_TIME:
					return  new BernoulliTimeLoglikelihood(parameters);
				default: throw prg_error("Unknown model type (%d).", "getNewLoglikelihood()", parameters.getModelType());
            }
        } break;
        default : throw prg_error("Unknown scan type (%d).", "getNewLoglikelihood()", parameters.getScanType());
    }
    return 0;
}
