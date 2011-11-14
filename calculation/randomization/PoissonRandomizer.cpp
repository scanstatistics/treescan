//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonRandomizer.h"

/* constructor */
PoissonRandomizer::PoissonRandomizer(bool conditional, int TotalC, double TotalN, long lInitialSeed)
                  : AbstractDenominatorDataRandomizer(lInitialSeed), _conditional(conditional), _TotalC(TotalC), _TotalN(TotalN) {}

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
int PoissonRandomizer::RandomizeData(unsigned int iSimulation,
                                      const ScanRunner::NodeStructureContainer_t& treeNodes,
                                      SimDataContainer_t& simData) {
  SetSeed(iSimulation);

    //-------------------- GENERATING THE RANDOM DATA ------------------------------
    int cases, CasesLeft, TotalSimC;
    double  ExpectedLeft;

    if (_conditional) {
        TotalSimC = _TotalC;
        CasesLeft = _TotalC;
        ExpectedLeft = _TotalN;
        for (size_t i=0; i < treeNodes.size(); i++) {
            cases = BinomialGenerator(CasesLeft, treeNodes.at(i)->getIntN() / ExpectedLeft);
            //if(cases>0 && Node[i].IntN<0.1) cout << "node=" << i <<  ", CasesLeft=" << CasesLeft << ", c=" << cases << ", exp=" << Node[i].IntN << ", ExpLeft=" << ExpectedLeft << endl;
            simData.at(i).first = cases; //treeNodes.at(i)->_SimIntC = cases;
            CasesLeft -= cases;
            ExpectedLeft -= treeNodes.at(i)->getIntN();
            simData.at(i).second = 0; //treeNodes.at(i)->_SimBrC = 0;  // Initilazing the branch cases with zero
        } // for i
    }  else { // if unconditional
        TotalSimC=0;
        ExpectedLeft = _TotalN;
        for(size_t i=0; i < treeNodes.size(); i++) {
            cases = PoissonGenerator(treeNodes.at(i)->getIntN());
            //if(cases>0 && Node[i].IntN<0.1) cout << "node=" << i <<  ",  c=" << cases << ", exp=" << Node[i].IntN << endl;
            simData.at(i).first = cases; //treeNodes.at(i)->_SimIntC=cases;
            TotalSimC += cases;
            ExpectedLeft -= treeNodes.at(i)->getIntN();
            simData.at(i).second = 0; //treeNodes.at(i)->_SimBrC = 0; // Initilazing the branch cases with zero
        }
    }

    //------------------------ UPDATING THE TREE -----------------------------------
    for (size_t i=0; i < treeNodes.size(); i++) {
        if (treeNodes.at(i)->getAnforlust()==false) addSimC(i, simData.at(i).first/*_Nodes.at(i)->_SimIntC*/, treeNodes, simData);
        else addSimCAnforlust(i, simData.at(i).first/*_Nodes.at(i)->_SimIntC*/, treeNodes, simData);
    }
    return TotalSimC;
}

/* Returns a Poisson distributed random variable. */
int PoissonRandomizer::PoissonGenerator(double lambda) {
    if (lambda==0) return 0;

    int     x=0;
    double  r=RandomUniform(), p=exp(-lambda), logfactorial=0;
    while (p < r) {
        x++;
        logfactorial = logfactorial + log(static_cast<double>(x));
        p = p + exp(-lambda + x * log(lambda) - logfactorial);
    }
    // cout << endl << "lambda=" << lambda << " r=" << r << " x=" << x << " rr=" << rr;
    return x;
}

/*
 Returns a uniform random number in the interval [0,1].
 Should be replaced by a better random number generator.
 */
double PoissonRandomizer::RandomUniform(bool classic) {
    //double rand_num = static_cast<double>(rand());
    //double return_value = (rand_num + 0.5) / static_cast<double>(RAND_MAX+1);
    //cout << "return_value " << return_value << endl;
    //return return_value;

    return classic ? double(rand()+0.5)/(RAND_MAX+1) : _randomNumberGenerator.GetRandomDouble();
}

/*
 ------ Returns a binomial(n,p) distributed random variable -------------------
 Note: SaTScan has a faster way of doing this.
*/
int PoissonRandomizer::BinomialGenerator(int n, double p, bool classic) {
    if (!classic) return gBinomialGenerator.GetBinomialDistributedVariable(n, static_cast<float>(p), _randomNumberGenerator);

    int     j;
    int     binomial;

    if(p==0) return 0;
    binomial=0;
    for (j=1;j<=n;j++) {
        if (RandomUniform() < p) binomial += 1;
    }
    return binomial;
}
