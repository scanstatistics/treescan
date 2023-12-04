#include "TreeScan.h"
#pragma hdrstop
#include "RandomDistribution.h"
/**********************************************************************
 file: randdist.c
 This file contains the following random variable distributions:
      Function            Range     Mean         Variance
      Bernoulli(p)        0..1      p            p*(1-p)
      Binomial(n, p)      0..n      n*p          n*p*(1-p)
      Equilikely(a, b)    [a,b]     (a+b)/2      ((b-a+1)*(b-a+1)-1)/12
 **********************************************************************/

// #define PI 3.141592654
#define PI 3.1415926535897932384626433832795028841972  /*KR-7/11/97*/

/**********************************************************************
 Returns 1 with probability p,
         0 with probability 1-p.
 **********************************************************************/
long Bernoulli(float p, RandomNumberGenerator & rng)
{
   if (rng.GetRandomDouble() < (1 - p))
      return 0;
   return 1;
}

/**********************************************************************
 Gamma function as used by the Binomial random generator function
 **********************************************************************/

// double /*floatKR-7/11/97*/ gammln(xx)
// double /*floatKR-7/11/97*/ xx;
double gammln(double xx)
{
   double x,tmp,ser;
   static double cof[6]={76.18009173,-86.50532033,24.01409822,
      -1.231739516,0.120858003e-2,-0.536382e-5};
   int j;

   x=xx-1.0;
   tmp=x+5.5;
   tmp -= (x+0.5)*log(tmp);
   ser=1.0;
   for (j=0;j<=5;j++) {
      x += 1.0;
      ser += cof[j]/x;
   }
   return -tmp+log(2.50662827465*ser);
}


/** Returns integers uniformly distributed from a to b, inclusive */
long Equilikely(long a, long b, RandomNumberGenerator & rng)
{
   return a + (long) floor((b - a + 1) * rng.GetRandomDouble());
}

/** Returns double uniformly distributed from a to b, inclusive */
double Equilikely(double a, double b, RandomNumberGenerator & rng)
{
   //return a + floor((b - a) * rng.GetRandomDouble());
   return a + (b - a) * rng.GetRandomDouble();
}

/** Returns binomial(n, p) distributed variable. */
long BinomialGenerator::GetBinomialDistributedVariable(long n, float pp, RandomNumberGenerator & rng) {
   long         j, rtn;
   double       am,em,g,angle,p,bnl,sq,t,y;

   // NOTE: these variables were defined as static at one time - that was
   //       determined to be unneccesary as results of continuous calls,
   //       whether static or not, produced identical results.
   double      pold = -1;
   double      pc=0;
   double      plog=0;
   double      pclog=0;
   double      en;
   double      oldg;
   int         nold = -1;

   p=(pp <= 0.5 ? pp : 1.0-pp);
   am=n*p;
   if (n < 25) {
      bnl=0.0;
      for (j=1;j<=n;j++)
         if (rng.GetRandomDouble() < p) bnl += 1.0;
   } else if (am < 1.0) {
      g=exp(-am);
      t=1.0;
      for (j=0;j<=n;j++) {
         t *= rng.GetRandomDouble();
         if (t < g) break;
      }
      bnl=(j <= n ? j : n);
   } else {
      if (n != nold) {
         en=n;
         oldg=gammln(en+1.0);
         //nold=n;
      } if (p != pold) {
         pc=1.0-p;
         plog=log(p);
         pclog=log(pc);
         //pold=p;
      }
      sq=sqrt(2.0*am*pc);
      do {
         do {
            angle=PI*rng.GetRandomDouble();
            y=tan(angle);
            em=sq*y+am;
         } while (em < 0.0 || em >= (en+1.0));
         em=floor(em);
         t=1.2*sq*(1.0+y*y)*exp(oldg-gammln(em+1.0)
            -gammln(en-em+1.0)+em*plog+(en-em)*pclog);
      } while (rng.GetRandomDouble() > t);
      bnl=em;
   }
   if (p != pp) bnl=n-bnl;
   rtn = (long)bnl;
   return rtn;
}

