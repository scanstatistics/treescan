//*****************************************************************************
#ifndef __RANDOMNUMBERGENERATOR_H
#define __RANDOMNUMBERGENERATOR_H
//*****************************************************************************

/**********************************************************************
 file: RandomNumberGenerator.h
 Header file for "RandomNumberGenerator.cpp"
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

class RandomNumberGenerator {
  private:
    long                glSeed;                         /** current randomization seed */
    long                glInitialSeed;                  /** initial seed at construction  */

    void                SetInitialSeed(long lSeed);

  public:
    RandomNumberGenerator(long lInitialSeed=glDefaultSeed) {SetInitialSeed(lInitialSeed);}
    ~RandomNumberGenerator() {}

    static const long   glDefaultSeed = 12345678;       /** default seed                  */
    static const long   glCheck       = 399268537;      /** value to check test against   */
    static const long   glM           = 2147483647;     /** fixed prime modulus, 2^31 - 1 */
    static const long   glA           = 48271;          /** multiplier                    */

    long                GetInitialSeed() const {return glInitialSeed;}
    long                GetMaxSeed() const {return glM;}
    double              GetRandomDouble();
    float               GetRandomFloat();
    long                GetSeed() const {return glSeed;}
    void                SetSeed(long lSeed);
    int                 Test();
};
#endif
