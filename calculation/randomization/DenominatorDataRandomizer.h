//******************************************************************************
#ifndef __DenominatorDataRandomizer_H
#define __DenominatorDataRandomizer_H
//******************************************************************************
#include "Randomization.h"

/** Abstraction for denominator data randomizer. */
class AbstractDenominatorDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   gBinomialGenerator;

  public:
    AbstractDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractRandomizer(lInitialSeed) {}
    virtual ~AbstractDenominatorDataRandomizer() {}
};
//******************************************************************************
#endif

