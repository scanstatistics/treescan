//******************************************************************************
#ifndef __Loglikelihood_H
#define __Loglikelihood_H
//******************************************************************************

class AbstractLoglikelihood {
public:
    AbstractLoglikelihood(){}
    virtual ~AbstractLoglikelihood(){}

    virtual double  LogLikelihood(int c, double n) const = 0;
};

class PoissonLoglikelihood : public AbstractLoglikelihood {
protected:
    const int       _totalC;
    const double    _totalN;

public:
    PoissonLoglikelihood(int totalC, double totalN) : AbstractLoglikelihood(), _totalC(totalC), _totalN(totalN)  {}
    virtual ~PoissonLoglikelihood(){}

    /* Calculates the conditional Poisson log likelihood. */
    virtual double  LogLikelihood(int c, double n) const {
        if (c - n < 0.0001) return 0;
        if (c == _totalC) return c * log(c/n);
        return c * log(c/n) + (_totalC - c) * log((_totalC - c)/(_totalN - n));
    }
};


class UnconditionalPoissonLoglikelihood : public AbstractLoglikelihood {
public:
    UnconditionalPoissonLoglikelihood() : AbstractLoglikelihood()  {}
    virtual ~UnconditionalPoissonLoglikelihood(){}

    /* Calculates the unconditional Poisson log likelihood */
    virtual double  LogLikelihood(int c, double n) const {
        if(c - n < 0.0001) return 0;
        return (n - c) + c * log(c/n);
    }
};
//******************************************************************************
#endif