#include "TreeScan.h"
#pragma hdrstop
#include "RandomNumberGenerator.h"
/**********************************************************************
 file: RandomNumberGenerator.cpp
 This file contains functions for random number generation.  It is
 based on "rng.pas", by Steve Park, as are the following comments.

 The generator upon which it is based is a "Lehmer random number
 generator" which returns a pseudo-random real number uniformly distributed
 between 0.0 and 1.0.  The period is (m - 1) where m = 2,147,483,647 and
 the smallest and largest possible values are (1 / m) and 1 - (1 / m)
 respectively.  For more details see -
   "Random Number Generators: Good Ones Are Hard To Find"
               Steve Park & Keith Miller
         Communications of the ACM, October, 1988
 **********************************************************************/

/** Lehmer generator which returns a pseudo-random real number uniformly
    distributed between 0 and 1.                                         */
double RandomNumberGenerator::GetRandomDouble() {
  long  t;

  t = glA * (glSeed % (glM / glA)) - (glM % glA) * (glSeed / (glM / glA));
  if (t > 0)
    glSeed = t;
  else
    glSeed = t + glM;
  return (double) glSeed /  (double) glM;
}

/** Lehmer generator which returns a pseudo-random real number uniformly
    distributed between 0 and 1.                                         */
float RandomNumberGenerator::GetRandomFloat() {
  long  t;

  t = glA * (glSeed % (glM / glA)) - (glM % glA) * (glSeed / (glM / glA));
  if (t > 0)
    glSeed = t;
  else
    glSeed = t + glM;
  return (float) glSeed / (float) glM;
}

/** Sets the random number generator seed.  Note: 0 < lSeed < glM */
void RandomNumberGenerator::SetSeed(long lSeed) {
  glSeed = ((0 < lSeed && lSeed < glM) ? lSeed : glDefaultSeed);
}

/** Sets initial value for the random number generator seed.  Note: 0 < lSeed < glM */
void RandomNumberGenerator::SetInitialSeed(long lSeed) {
  glSeed = glInitialSeed = ((0 < lSeed && lSeed < glM) ? lSeed : glDefaultSeed);
}

/** Tests for a correct implementation.
    Return value: 1 = Correct
                  0 = incorrect         */
int RandomNumberGenerator::Test() {
  long  l;

  SetSeed(1);
  for (l=1; l <= 10000; l++)
     GetRandomDouble();
  return (GetSeed() == glCheck);
}
