//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonRandomizer.h"

/** Constructor */
PoissonRandomizer::PoissonRandomizer(bool conditional, const ScanRunner& scanner, long lInitialSeed)
                  : AbstractDenominatorDataRandomizer(scanner, lInitialSeed), 
	               _conditional(conditional), _total_C(scanner.getTotalC()), _total_N(scanner.getTotalN()) {
	sequentialSetup(_scanner);
}

/** Creates randomized data under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
int PoissonRandomizer::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    // reset seed of random number generator
    setSeed(iSimulation);
    // reset simData
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    //-------------------- GENERATING THE RANDOM DATA ------------------------------
    int cases, CasesLeft, TotalSimC;
    double  ExpectedLeft;

    if (_conditional) {
        TotalSimC = _parameters.isSequentialScanPoisson() ? _scanner.getTotalsFromLook().first : _total_C;
        CasesLeft = TotalSimC;
        ExpectedLeft = _parameters.isSequentialScanPoisson() ? _scanner.getTotalsFromLook().second : _total_N;
        for (size_t i=0; i < treeNodes.size(); i++) {
            treeSimNodes[i].refBrC() = 0; // initializing the branch cases with zero
            if (!treeNodes.randomized(i)) continue; // skip if not randomized
            cases = BinomialGenerator(CasesLeft, treeNodes.getIntN(i) / ExpectedLeft);
            treeSimNodes[i].refIntC() = cases; // treeNodes.at(i)->_SimIntC = cases;
            CasesLeft -= cases;
            ExpectedLeft -= treeNodes.getIntN(i);
        } // for i
    }  else { // if unconditional
        TotalSimC=0;
        for(size_t i=0; i < treeNodes.size(); i++) {
            treeSimNodes[i].refBrC() = 0; // initializing the branch cases with zero
            if (!treeNodes.randomized(i)) continue; // skip if not randomized
            cases = PoissonGenerator(treeNodes.getIntN(i));
            treeSimNodes[i].refIntC() = cases; // treeNodes.at(i)->_SimIntC=cases;
            TotalSimC += cases;
        }
    }
    return TotalSimC;
}

/** Returns a Poisson distributed random variable. */
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

/** Returns a uniform random number in the interval [0,1].
    Should be replaced by a better random number generator. */
double PoissonRandomizer::RandomUniform() {
    return _random_number_generator.GetRandomDouble();
}

/** Returns a binomial(n,p) distributed random variable
    Note: TreeScan has a faster way of doing this.
*/
int PoissonRandomizer::BinomialGenerator(int n, double p, bool classic) {
    if (!classic) return _binomial_generator.GetBinomialDistributedVariable(n, static_cast<float>(p), _random_number_generator);

    int     j;
    int     binomial;

    if(p==0) return 0;
    binomial=0;
    for (j=1;j<=n;j++) {
        if (RandomUniform() < p) binomial += 1;
    }
    return binomial;
}
