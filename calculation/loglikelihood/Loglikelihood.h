//******************************************************************************
#ifndef __Loglikelihood_H
#define __Loglikelihood_H
//******************************************************************************
#include "Parameters.h"
#include "PrjException.h"

class AbstractLoglikelihood {
public:
    AbstractLoglikelihood(){}
    virtual ~AbstractLoglikelihood(){}

    virtual double LogLikelihood(int c, double n) const {
        throw prg_error("LogLikelihood(int,int) not implemented.", "LogLikelihood()");
    }
    virtual double LogLikelihood(int c, double n, size_t windowLength) const {
        throw prg_error("LogLikelihood(int,int,size_t) not implemented.", "LogLikelihood()");
    }
    virtual double LogLikelihoodRatio(double logLikelihood) const = 0;
    static AbstractLoglikelihood * getNewLoglikelihood(const Parameters& parameters, int TotalC, double TotalN);
    static double UNSET_LOGLIKELIHOOD;
};

class PoissonLoglikelihood : public AbstractLoglikelihood {
protected:
    const int       _totalC;
    const double    _totalN;

public:
    PoissonLoglikelihood(int totalC, double totalN) : AbstractLoglikelihood(), _totalC(totalC), _totalN(totalN)  {}
    virtual ~PoissonLoglikelihood(){}

    /* Calculates the conditional Poisson loglikelihood. */
    virtual double  LogLikelihood(int c, double n) const {
        if (c - n < 0.0001) return UNSET_LOGLIKELIHOOD;
        if (c == _totalC) return c * log(c/n);
        return c * log(c/n) + (_totalC - c) * log((_totalC - c)/(_totalN - n));
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood - _totalC * log(_totalC/_totalN);
    }
};


class UnconditionalPoissonLoglikelihood : public AbstractLoglikelihood {
public:
    UnconditionalPoissonLoglikelihood() : AbstractLoglikelihood()  {}
    virtual ~UnconditionalPoissonLoglikelihood(){}

    /* Calculates the unconditional Poisson loglikelihood */
    virtual double  LogLikelihood(int c, double n) const {
        if(c - n < 0.0001) return UNSET_LOGLIKELIHOOD;
        return (n - c) + c * log(c/n);
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood;
    }
};

class BernoulliLoglikelihood : public AbstractLoglikelihood {
protected:
    const int       _totalC;
    const double    _totalN;

public:
    BernoulliLoglikelihood(int totalC, double totalN) : AbstractLoglikelihood(), _totalC(totalC), _totalN(totalN)  {}
    virtual ~BernoulliLoglikelihood(){}

    /* Calculates the conditional Bernoulli loglikelihood. */
    virtual double  LogLikelihood(int c, double n) const {
        if(c/n > _totalC/_totalN) { 
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
        return -std::numeric_limits<double>::max();
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood - (_totalC * log(_totalC/_totalN) + (_totalN - _totalC) * log((_totalN - _totalC)/_totalN));
    }
};

class UnconditionalBernoulliLogLoglikelihood : public AbstractLoglikelihood {
protected:
    double _event_probability;

public:
    UnconditionalBernoulliLogLoglikelihood(double event_probability) : AbstractLoglikelihood(), _event_probability(event_probability)  {}
    virtual ~UnconditionalBernoulliLogLoglikelihood(){}

    /* Calculates the unconditional Bernoulli loglikelihood */
    virtual double  LogLikelihood(int c, double n) const {
		if (c/n > _event_probability) {// currently scanning for high rates only
            double dLogLikelihood=0;
			if (n - static_cast<double>(c) > 0.0) {
				dLogLikelihood = c * log(c/n) + (n-c) * log((n-c)/n);
			} else {
                dLogLikelihood = c * log(c/n);
            }
            return dLogLikelihood - (c * log(_event_probability) + (n-c) * log(1-_event_probability)); // actually return the loglikelihood ratio
		}
        return UNSET_LOGLIKELIHOOD;
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        return logLikelihood;
    }
};

class TemporalLoglikelihood : public AbstractLoglikelihood {
protected:
    const int       _totalC;
    const double    _totalN;
    size_t          _totalDaysInRange;

public:
    TemporalLoglikelihood(int totalC, double totalN, size_t totalDaysInRange) 
        : AbstractLoglikelihood(), _totalC(totalC), _totalN(totalN), _totalDaysInRange(totalDaysInRange) {}
    virtual ~TemporalLoglikelihood(){}

    /* Calculates the conditional temporal loglikelihood. */
    virtual double  LogLikelihood(int c, double n, size_t windowLength) const {
        /* c = cases in the window for the branch under consideration, c>=0 */ 
        /* n = total cases for the branch under consideration, inside and outside the window, n>=c */ 
        double r = static_cast<double>(windowLength) / static_cast<double>(_totalDaysInRange);
        if (n > 0.0 && static_cast<double>(c)/n > r) { 
            double loglikelihood = 0;
            if (c != 0) {
                loglikelihood = static_cast<double>(c) * log(static_cast<double>(c)/n);
            } 
            if (c != n) {
                loglikelihood = loglikelihood + (n - static_cast<double>(c)) * log(1.0 - (static_cast<double>(c)/n));
            }
            // we're calculating the full loglikelihood ratio here
            return loglikelihood - static_cast<double>(c) * log(r) - (n - static_cast<double>(c)) * log(1.0 - r);
        }
        return -std::numeric_limits<double>::max();
    }
    virtual double  LogLikelihoodRatio(double logLikelihood) const {
        if (logLikelihood == UNSET_LOGLIKELIHOOD) return 0.0;
        // The full loglikelihood ratio is calculated in LogLikelihood() call.
        return logLikelihood;
    }
};
//******************************************************************************
#endif
