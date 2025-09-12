//******************************************************************************
#ifndef __Loglikelihood_H
#define __Loglikelihood_H
//******************************************************************************
#include "Parameters.h"
#include "PrjException.h"

/** Abstract base log likelihood class. */
class AbstractLoglikelihood {
public:
    typedef bool (AbstractLoglikelihood::*SCANRATE_FUNCPTR) (int, double, int, double) const;
    typedef bool (AbstractLoglikelihood::*SCANRATE_UNCOND_FUNCPTR) (int, double) const;
    typedef bool (AbstractLoglikelihood::*SCANRATE_PROB_FUNCPTR) (int, double, double) const;
    typedef bool (AbstractLoglikelihood::*SCANRATE_WIN_FUNCPTR) (int, double, double) const;

protected:
    const Parameters::ScanRateType _scan_rate;
    const unsigned int _minimum_highrate_nodes_cases;
    const unsigned int _minimum_lowrate_nodes_cases;

public:
    AbstractLoglikelihood(const Parameters& parameters)
        :_scan_rate(parameters.getScanRateType()),
         _minimum_highrate_nodes_cases(parameters.getMinimumHighRateNodeCases()),
         _minimum_lowrate_nodes_cases(parameters.getMinimumLowRateNodeCases()) {}
    virtual ~AbstractLoglikelihood(){}

    virtual double LogLikelihood(int c, double n) const {
        throw prg_error("LogLikelihood(int,int) not implemented.", "LogLikelihood()");
    }
    virtual double LogLikelihood(int c, double n, int bc, double bn) const {
        throw prg_error("LogLikelihood(int,double,int,double) not implemented.", "LogLikelihood()");
    }    
	virtual double LogLikelihood(int c, double n, size_t windowLength) const {
        throw prg_error("LogLikelihood(int,int,size_t) not implemented.", "LogLikelihood()");
    }
    virtual double LogLikelihoodRatio(double logLikelihood) const = 0;
    static AbstractLoglikelihood * getNewLoglikelihood(const Parameters& parameters, int TotalC, double TotalN, bool censored_data);
    static double UNSET_LOGLIKELIHOOD;

    SCANRATE_FUNCPTR _of_interest;
    bool HighRate(int c, double n, int C, double N) const {
        if (n == 0.0) return false;
        return static_cast<unsigned int>(c) >= _minimum_highrate_nodes_cases && static_cast<double>(c) / n > static_cast<double>(C) / N;
    }
    bool LowRate(int c, double n, int C, double N) const {
        if (n == 0.0) return false;
        return static_cast<unsigned int>(c) >= _minimum_lowrate_nodes_cases && static_cast<double>(c) / n < static_cast<double>(C) / N;
    }
    bool HighOrLowRate(int c, double n, int C, double N) const {
        if (n == 0.0) return false;
        return HighRate(c, n, C, N) || LowRate(c, n, C, N);
    }
    SCANRATE_UNCOND_FUNCPTR _uncond_of_interest;
    bool HighRateUnconditioned(int c, double n) const {
        return static_cast<unsigned int>(c) >= _minimum_highrate_nodes_cases && static_cast<double>(c) - n >= 0.0001;
    }
    bool LowRateUnconditioned(int c, double n) const {
        return static_cast<unsigned int>(c) >= _minimum_lowrate_nodes_cases && static_cast<double>(c) - n <= -0.0001;
    }
    bool HighOrLowRateUnconditioned(int c, double n) const {
        return HighRateUnconditioned(c, n) || LowRateUnconditioned(c, n);
    }
    SCANRATE_PROB_FUNCPTR _prob_of_interest;
    bool HighRateProbability(int c, double n, double p) const {
        if (n == 0.0) return false;
        return static_cast<unsigned int>(c) >= _minimum_highrate_nodes_cases && static_cast<double>(c) / n > p;
    }
    bool LowRateProbability(int c, double n, double p) const {
        if (n == 0.0) return false;
        return static_cast<unsigned int>(c) >= _minimum_lowrate_nodes_cases && static_cast<double>(c) / n < p;
    }
    bool HighOrLowRateProbability(int c, double n, double p) const {
        return HighRateProbability(c, n, p) || LowRateProbability(c, n, p);
    }
    SCANRATE_WIN_FUNCPTR _win_of_interest;
    bool HighRateWindow(int c, double n, double r) const {
        if (n == 0.0) return false;

        // TODO:
        // What is the behavior when c == n and therefore c/n == 1.0; passing c/n > r check?

        return static_cast<unsigned int>(c) >= _minimum_highrate_nodes_cases && static_cast<double>(c) / n > r;
    }
    bool LowRateWindow(int c, double n, double r) const {
        if (n == 0.0) return false;
        return static_cast<unsigned int>(c) >= _minimum_lowrate_nodes_cases && static_cast<double>(c) / n < r;
    }
    bool HighOrLowRateWindow(int c, double n, double r) const {
        return HighRateWindow(c, n, r) || LowRateWindow(c, n, r);
    }
};

typedef boost::shared_ptr<AbstractLoglikelihood> Loglikelihood_t;

/** Log likelihood class for conditional Poisson. */
class PoissonLoglikelihood : public AbstractLoglikelihood {
protected:
    const int       _totalC;
    const double    _totalN;

public:
    PoissonLoglikelihood(int totalC, double totalN, const Parameters& parameters) : AbstractLoglikelihood(parameters), _totalC(totalC), _totalN(totalN) {
        switch (parameters.getScanRateType()) {
            case Parameters::LOWRATE: _of_interest = &AbstractLoglikelihood::LowRate; break;
            case Parameters::HIGHORLOWRATE: _of_interest = &AbstractLoglikelihood::HighOrLowRate; break;
            case Parameters::HIGHRATE:
            default: _of_interest = &AbstractLoglikelihood::HighRate; break;
        }
    }
    virtual ~PoissonLoglikelihood(){}

    /** Calculates the conditional Poisson log likelihood. */
    virtual double LogLikelihood(int c, double n) const {
        if (!(this->*_of_interest)(c, n, _totalC, _totalN)) return UNSET_LOGLIKELIHOOD;
        if (c == _totalC) return c * log(c/n);
        if (c == 0) return _totalC * log(_totalC / (_totalN - n));
        return c * log(c/n) + (_totalC - c) * log((_totalC - c)/(_totalN - n));
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood - _totalC * log(_totalC/_totalN);
    }
};

/** Log likelihood class for conditional Poisson using censored data. */
class PoissonCensoredLoglikelihood : public AbstractLoglikelihood {
public:
    PoissonCensoredLoglikelihood(const Parameters& parameters) : AbstractLoglikelihood(parameters) {
        switch (parameters.getScanRateType()) {
            case Parameters::LOWRATE: _of_interest = &AbstractLoglikelihood::LowRate; break;
            case Parameters::HIGHORLOWRATE: _of_interest = &AbstractLoglikelihood::HighOrLowRate; break;
            case Parameters::HIGHRATE:
            default: _of_interest = &AbstractLoglikelihood::HighRate; break;
        }
    }
    virtual ~PoissonCensoredLoglikelihood() {}

    /** Calculates the conditional Poisson log likelihood. */
    virtual double  LogLikelihood(int c, double n, int bc, double bn) const {
        // c = cases in the window for the branch under consideration
        // n = expected cases in the window for the branch under consideration
        // bc = total cases for the branch under consideration, inside and outside the window
        // bn = total expected cases for the branch under consideration, inside and outside the window
        if (!(this->*_of_interest)(c, n, bc, bn)) return UNSET_LOGLIKELIHOOD;
        if (c == bc) return c * log(c / n);
        if (c == 0) return bc * log(bc / (bn - n));
        return c * log(c / n) + (bc - c) * log((bc - c) / (bn - n));
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood;
    }
};

/** Log likelihood class for unconditional Poisson. */
class UnconditionalPoissonLoglikelihood : public AbstractLoglikelihood {
public:
    UnconditionalPoissonLoglikelihood(const Parameters& parameters) : AbstractLoglikelihood(parameters)  {
        switch (parameters.getScanRateType()) {
            case Parameters::LOWRATE: _uncond_of_interest = &AbstractLoglikelihood::LowRateUnconditioned; break;
            case Parameters::HIGHORLOWRATE: _uncond_of_interest = &AbstractLoglikelihood::HighOrLowRateUnconditioned; break;
            case Parameters::HIGHRATE:
            default: _uncond_of_interest = &AbstractLoglikelihood::HighRateUnconditioned; break;
        }
    }
    virtual ~UnconditionalPoissonLoglikelihood(){}

    /** Calculates the unconditional Poisson log likelihood. */
    virtual double  LogLikelihood(int c, double n) const {
        if (!(this->*_uncond_of_interest)(c, n)) return UNSET_LOGLIKELIHOOD;
        return (n - c) + c * log(c/n);
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood;
    }
};

/* Log likelihood class for conditional Bernoulli. */
class BernoulliLoglikelihood : public AbstractLoglikelihood {
protected:
    const int       _totalC;
    const double    _totalN;

public:
    BernoulliLoglikelihood(int totalC, double totalN, const Parameters& parameters) : AbstractLoglikelihood(parameters), _totalC(totalC), _totalN(totalN)  {
        switch (parameters.getScanRateType()) {
            case Parameters::LOWRATE: _of_interest = &AbstractLoglikelihood::LowRate; break;
            case Parameters::HIGHORLOWRATE: _of_interest = &AbstractLoglikelihood::HighOrLowRate; break;
            case Parameters::HIGHRATE:
            default: _of_interest = &AbstractLoglikelihood::HighRate; break;
        }
    }
    virtual ~BernoulliLoglikelihood(){}

    /** Calculates the conditional Bernoulli log likelihood. */
    virtual double  LogLikelihood(int c, double n) const {
        if (!(this->*_of_interest)(c, n, _totalC, _totalN)) return UNSET_LOGLIKELIHOOD;
        double nLL_A = 0.0, nLL_B = 0.0, nLL_C = 0.0, nLL_D = 0.0;
        if (c != 0)
            nLL_A = c*log(c/n);
        if (c != n)
            nLL_B = (n-c)*log(1-(c/n));
        if (_totalC-c != 0)
            nLL_C = (_totalC-c)*log((_totalC-c)/(_totalN-n));
        if (_totalC-c != _totalN-n)
            nLL_D = ((_totalN-n)-(_totalC-c))*log(1-((_totalC-c)/(_totalN-n)));
        return nLL_A + nLL_B + nLL_C + nLL_D;
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood - (_totalC * log(_totalC/_totalN) + (_totalN - _totalC) * log((_totalN - _totalC)/_totalN));
    }
};

/** Log likelihood class for unconditional Bernoulli. */
class UnconditionalBernoulliLogLoglikelihood : public AbstractLoglikelihood {
protected:
    double _event_probability;

public:
    UnconditionalBernoulliLogLoglikelihood(const Parameters& parameters) : AbstractLoglikelihood(parameters), _event_probability(parameters.getProbability())  {
        switch (parameters.getScanRateType()) {
            case Parameters::LOWRATE: _prob_of_interest = &AbstractLoglikelihood::LowRateProbability; break;
            case Parameters::HIGHORLOWRATE: _prob_of_interest = &AbstractLoglikelihood::HighOrLowRateProbability; break;
            case Parameters::HIGHRATE:
            default: _prob_of_interest = &AbstractLoglikelihood::HighRateProbability; break;
        }
    }
    virtual ~UnconditionalBernoulliLogLoglikelihood(){}

    /** Calculates the unconditional Bernoulli log likelihood. */
    virtual double  LogLikelihood(int c, double n) const {
        if (!(this->*_prob_of_interest)(c, n, _event_probability)) return UNSET_LOGLIKELIHOOD;
        double dLogLikelihood=0;
        if (n - static_cast<double>(c) > 0.0) {
            dLogLikelihood = c * log(c / n) + (n - c) * log((n - c) / n);
        } else if (c == 0.0) {
            dLogLikelihood = n * log((n) / n);
        } else {
            dLogLikelihood = c * log(c/n);
        }
        return dLogLikelihood - (c * log(_event_probability) + (n-c) * log(1-_event_probability)); // actually return the loglikelihood ratio
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood;
    }
};

/** Log likelihood class for conditional Bernoulli time model. */
class BernoulliTimeLoglikelihood : public AbstractLoglikelihood {
public:
	BernoulliTimeLoglikelihood(const Parameters& parameters) : AbstractLoglikelihood(parameters) {
        switch (parameters.getScanRateType()) {
            case Parameters::LOWRATE: _of_interest = &AbstractLoglikelihood::LowRate; break;
            case Parameters::HIGHORLOWRATE: _of_interest = &AbstractLoglikelihood::HighOrLowRate; break;
            case Parameters::HIGHRATE:
            default: _of_interest = &AbstractLoglikelihood::HighRate; break;
        }
    }
	virtual ~BernoulliTimeLoglikelihood() {}

	/** Calculates the conditional Bernoulli log likelihood. */
	virtual double LogLikelihood(int c, double n, int C, double N) const {
		// c = cases in the window for the branch under consideration
        // n = cases and controls in the window for the branch under consideration
        // C = total cases for the branch under consideration, inside and outside the window
        // N = total cases and controls for the branch under consideration, inside and outside the window
        if (!(this->*_of_interest)(c, n, C, N)) return UNSET_LOGLIKELIHOOD;
        double nLL_A = 0.0, nLL_B = 0.0, nLL_C = 0.0, nLL_D = 0.0;
        if (c != 0)
            nLL_A = c * log(c / n);
        if (c != n)
            nLL_B = (n - c) * log(1 - (c / n));
        if (C - c != 0)
            nLL_C = (C - c) * log((C - c) / (N - n));
        if (C - c != N - n)
            nLL_D = ((N - n) - (C - c)) * log(1 - ((C - c) / (N - n)));
        return (nLL_A + nLL_B + nLL_C + nLL_D) - (C * log(C / N) + (N - C) * log((N - C) / N))/* under null */;
	}

	virtual double  LogLikelihoodRatio(double logLikelihood) const {
		if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
		return logLikelihood;
	}
};

/** Log likelihood class for conditional uniform time model. */
class TemporalLoglikelihood : public AbstractLoglikelihood {
protected:
    const int       _totalC;
    const double    _totalN;
    size_t          _totalDaysInRange;
    std::vector<double> _log1;
    std::vector<double> _log2;

public:
    TemporalLoglikelihood(int totalC, double totalN, const Parameters& parameters)
        : AbstractLoglikelihood(parameters), _totalC(totalC), _totalN(totalN), _totalDaysInRange(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets()) {
        unsigned int max_windowlength = parameters.getMaximumWindowInTimeUnits();
        // If applying window exclusions with the uniform time model, reduce the total days in range by excluded ranges.
        if (parameters.isApplyingExclusionTimeRanges() && parameters.getModelType() == Parameters::UNIFORM) {
            for (const auto& excluded : parameters.getExclusionTimeRangeSet().getDataTimeRangeSets())
                _totalDaysInRange -= excluded.getEnd() - excluded.getStart() + 1;
            max_windowlength = std::min(max_windowlength, static_cast<unsigned int>(_totalDaysInRange));
        }
        _log1.resize(max_windowlength + 1, 0.0);
        _log2.resize(max_windowlength + 1, 0.0);
        for (size_t i=1; i <= max_windowlength; ++i) {
            _log1[i] = log(static_cast<double>(i) / static_cast<double>(_totalDaysInRange));
            _log2[i] = log(1.0 -  static_cast<double>(i) / static_cast<double>(_totalDaysInRange));
        }
        switch (parameters.getScanRateType()) {
            case Parameters::LOWRATE: _win_of_interest = &AbstractLoglikelihood::LowRateWindow; break;
            case Parameters::HIGHORLOWRATE: _win_of_interest = &AbstractLoglikelihood::HighOrLowRateWindow; break;
            case Parameters::HIGHRATE:
            default: _win_of_interest = &AbstractLoglikelihood::HighRateWindow; break;
        }
    }
    virtual ~TemporalLoglikelihood(){}

    /** Calculates the conditional temporal log likelihood. */
    virtual double  LogLikelihood(int c, double n, size_t windowLength) const {
        // c = cases in the window for the branch under consideration, c>=0
        // n = total cases for the branch under consideration, inside and outside the window, n>=c
        double r = static_cast<double>(windowLength) / static_cast<double>(_totalDaysInRange);
        if (!(this->*_win_of_interest)(c, n, r)) return UNSET_LOGLIKELIHOOD;
        double loglikelihood = 0;
        if (c != 0) {
            loglikelihood = static_cast<double>(c) * log(static_cast<double>(c)/n);
        }
        if (c != n) {
            loglikelihood = loglikelihood + (n - static_cast<double>(c)) * log(1.0 - (static_cast<double>(c)/n));
        }
        // we're calculating the full loglikelihood ratio here
        return loglikelihood - static_cast<double>(c) * _log1[windowLength] - (n - static_cast<double>(c)) * _log2[windowLength];
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        // The full log likelihood ratio is calculated in LogLikelihood() call.
        return logLikelihood;
    }
};
//******************************************************************************
#endif
