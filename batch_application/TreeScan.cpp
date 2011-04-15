//------------------------------------------------------------------------------
//
//                           TreeScan C++ code
//
//                      Written by: Martin Kulldorff
//                  January, 2004; revised September 2007
//
//------------------------------------------------------------------------------
#include "ScanRunner.h"
#include "PrintScreen.h"

//----------------- INPUT VARIABLES TO BE SET BY THE USER ----------------------
#define         DUPLICATES 0
bool            Conditional=0;      // If Conditional=0, an unconditional analysis is performed
const int       nMCReplicas=99999;  // Number of Monte Carlo replicatons (simulations). Should be e.g. 999 or 9999.
const int       nCuts=2000;         // Number of most likely cuts that are saved.

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " [input filename]\n\n";
        return -1;
    }

    PrintScreen console(false);
    ScanRunner runner(Conditional, DUPLICATES, nCuts, nMCReplicas, console);
    runner.run(argv[1]);
    return 0;
}
