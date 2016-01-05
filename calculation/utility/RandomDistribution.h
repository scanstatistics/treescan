//*****************************************************************************
#ifndef __RANDOMDISTRIBUTION_H
#define __RANDOMDISTRIBUTION_H
//*****************************************************************************
#include "RandomNumberGenerator.h"
#include <boost/math/distributions/binomial.hpp>
//using boost::math::binomial;

/**********************************************************************
 file: RandomDistribution.h
 Header file for RandomDistribution.cpp
 **********************************************************************/
long    Bernoulli(float p, RandomNumberGenerator & rng);
long    Equilikely(long a, long b, RandomNumberGenerator & rng);
double  Equilikely(double a, double b, RandomNumberGenerator & rng);
double  gammln(double xx);

/** Generates binomial(n, p) distributed variable. */
class BinomialGenerator {
  private:
    //double      pold;
    //double      pc;
    //double      plog;
    //double      pclog;
    //double      en;
    //double      oldg;
    //int         nold;

    ///void        Init() {pold = -1.0; nold = -1;}

  public:
    BinomialGenerator() {/*Init();*/}
    virtual ~BinomialGenerator(){}

    long        GetBinomialDistributedVariable(long n, float pp, RandomNumberGenerator & rng);
    double      getBinomialDistributionProbability(unsigned int successes, unsigned int trials, double probability) {
                    boost::math::binomial distribution(trials, probability);
                    return boost::math::pdf(distribution, successes);
                }
};
//*****************************************************************************
#endif
