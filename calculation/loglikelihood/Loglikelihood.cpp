//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "Loglikelihood.h"
#include "ScanRunner.h"

double AbstractLoglikelihood::UNSET_LOGLIKELIHOOD = -std::numeric_limits<double>::max();

AbstractLoglikelihood* AbstractLoglikelihood::getNewLoglikelihood(const ScanRunner& scanrunner) {
    return AbstractLoglikelihood::getNewLoglikelihood(
        scanrunner.getParameters(), scanrunner.getTotalC(), scanrunner.getTotalN(), 
        scanrunner.isCensoredData(), static_cast<unsigned int>(scanrunner.getSampleSiteIdentifiers().size())
    );
}

/** Returns new randomizer given parameter settings. */
AbstractLoglikelihood * AbstractLoglikelihood::getNewLoglikelihood(const Parameters& parameters, int TotalC, double TotalN, bool censored_data, unsigned int sample_sites) {
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
                case Parameters::SIGNED_RANK:
                    return new SignedRankLoglikelihood(parameters, sample_sites);
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

//////////////////////////////////////// SignedRankLoglikelihood ////////////////////////////////////////

SignedRankLoglikelihood::SignedRankLoglikelihood(const Parameters& parameters, unsigned int num_sample_sites):
    AbstractLoglikelihood(parameters), _num_sample_sites(num_sample_sites) {
    _ranker.resize(num_sample_sites);
}

double SignedRankLoglikelihood::LogLikelihoodRatio(const SampleSiteDifferenceProxy& ssData) const {
    //assert(ssData.size() == _num_sample_sites);
    double diff;
    for (size_t t=0; t < ssData.size(); ++t) {
        _ranker[t].first = diff = ssData.nextDifference();
        _ranker[t].second = std::abs(diff);
    }
    std::sort(_ranker.begin(), _ranker.end(), [](const std::pair<double, double>& a, const std::pair<double, double>& b) {
        return a.second < b.second;
    });
    unsigned int ranksum = 0;
    double llr = 0.0;
    for (auto it=_ranker.begin(); it != _ranker.end(); ++it) {
        if (it->first == 0.0) continue; // skip all zero differences, which are sorted to the beginning and contribute no rank
        auto next = std::next(it);
        auto start_it = it;
        ranksum = static_cast<unsigned int>(std::distance(_ranker.begin(), it) + 1);
        while (next != _ranker.end() && it->second == next->second) { // handle ties by averaging ranks
            ranksum += static_cast<unsigned int>(std::distance(_ranker.begin(), next) + 1);
            it = next;
            ++next;
        }
        double average_rank = static_cast<double>(ranksum)/static_cast<double>(static_cast<unsigned int>(std::distance(start_it, it) + 1));
        for (auto itu=start_it; itu != next; ++itu)
            llr += itu->first > 0.0 ? average_rank : -average_rank;
    }
    return llr;
}