//------------------------------------------------------------------------------
//
//                           TreeScan C++ code
//
//                      Written by: Martin Kulldorff
//                  January, 2004; revised September 2007
//
//------------------------------------------------------------------------------
#pragma hdrstop
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace::std;

//---------------------- FUNCTION DEFINITIONS ----------------------------------
void AddCN(int id, int c, float n);
void AddSimC(int id, int c);
void AddSimCAnforlust(int id, int c);
int BinomialGenerator(int n, float p);
int PoissonGenerator(double lambda);
float PoissonLogLikelihood(int c, float n, int TotalC, float TotalN);
float UnconditionalPoissonLogLikelihood(int c, float n);
double RandomUniform(void);

//----------------- INPUT VARIABLES TO BE SET BY THE USER ----------------------
#define         DUPLICATES 0
//#define         INPUT_FILE "C:\\Documents and Settings\\MKulldor\\My Documents\\ADRdata\\All_Sites\\inpdx_core_dxtree_res_mod_PIO_180.txt"      // Input file name
//#define         OUTPUT_FILE "C:\\Documents and Settings\\MKulldor\\My Documents\\ADRdata\\All_Sites\\CONDITIONAL_inpdx_core_dxtree_res_mod_PIO_180_OUTPUT.txt"      // Output file name
#define         INPUT_FILE "C:\\prj\\treescan\\treescan\\data\\alldx_core_dxtree_res_mod_CLO_180.txt"      // Input file name
#define         OUTPUT_FILE "C:\\prj\\treescan\\treescan\\data\alldx_core_dxtree_res_mod_CLO_180__output.txt"      // Output file name
bool            Conditional=1; // If Conditional=0, an unconditional analysis is performed
//const int       nMCReplicas=99999;  // Number of Monte Carlo replicatons (simulations). Should be e.g. 999 or 9999.
const int       nMCReplicas=99999;  // Number of Monte Carlo replicatons (simulations). Should be e.g. 999 or 9999.
const int       nNodes=1088;        // Number of nodes
const int       MaxChildren=50;   // Max number of children per node (ADR: 11 or 50)
const int       MaxParents=1;     // Max number of parents per node
const int       nCuts=2000; // Number of most likely cuts that are saved.

//---------------------- GLOBAL VARIABLE DEFINITIONS ---------------------------
// Holds information about each node.
struct nodestructure {
        int     ID;            // The node ID.
        int     IntC, SimIntC; // Number of true and simulated cases internal to the node, respectively.
        int     BrC, SimBrC;   // Number of true and simulated cases in the node and all decendants (children, grandchildren etc.)
        float   IntN,BrN;      // Expexted number of cases internal to the node, and with all decendants respectively.
        int     nChildren, nParents;  // Number of children and parents of that node, respectively.
        int     Child[MaxChildren], Parent[MaxParents]; // List of node IDs of the children and parents
        bool    Anforlust;      // =1 if at least one node is an ancestor in more than one way, otherwise =0
#if DUPLICATES
        int     Duplicates;     // Number of duplicates that needs to be removed.
#endif
        } Node[nNodes];

int             TotalC;           // Total number of cases in the data set.
float           TotalN;           // Total measure in the data set.
int             Ancestor[nNodes]; // Zero/one variable indicating whether this node is an ancestor to the node in question.
// Holds information about the most likely cut.
struct cutstructure {
        int     ID;    // NodeID
        int     C;     // Number of cases.
        float   N;     // Expected number of cases.
        float   LogLikelihood;   // Loglikelihood value.
        } Cut[nCuts];


//------------------------------------------------------------------------------
//#pragma argsused
int main(int argc, char* argv[])
{

int             i; // used to iterate over nodes
int             j; // used to iterate over children or parents within a node
int             k,m; // used for all other iterations
int             replica;
int             Rank[nCuts];
float           loglikelihood;
float           SimLogLikelihood;
float           LogLikelihoodRatio;
char            Temp;

time_t                gStartTime, gEndTime;
time(&gStartTime); //get start time

//----------------------- READING THE INPUT FILE -------------------------------
int             id;

if (argc != 2) {
   cerr << "usage: " << argv[0] << " [input filename]\n\n";
   return -1;
}

cout << "Reading the input file." << endl;
ifstream infile(argv[1]);
if(!infile) {
        cerr << "Unable to open input file.\n";
        cout << "Press any character to end." << endl; cin >> Temp;
        return -1;
        }
for(i=0;i<nNodes;i++) {
#if DUPLICATES
        infile >> Node[i].ID >>  Node[i].IntC >> Node[i].Duplicates >> Node[i].IntN >> Node[i].nParents;
#else
        infile >> Node[i].ID >>  Node[i].IntC >> Node[i].IntN >> Node[i].nParents;
#endif
//                                                                              cout << "id=" << id << ", c=" << Node[i].IntC << ", n=" << Node[i].IntN << ", nParents=" << Node[i].nParents << endl;
        if(Node[i].nParents > MaxParents) {
                cerr << "Error: To many parents for node" << i << endl;
                cout << "Press any character to end." << endl; cin >> Temp;
                return -1;
                }
        for(j=0;j<Node[i].nParents;j++) infile >> Node[i].Parent[j];
        if ( i != Node[i].ID ) {
                cerr << "Error: Incorrect node ID on line " << i+1 << " in the input file." << endl
                     << "These must start with zero and increase by one for each line." << endl;
                cout << "Press any character to end." << endl; cin >> Temp;
                return -1;
                }
      if (Node[i].IntC<0) {
              cerr << "Error: Negative number of cases in node" << i << endl;
              cout << "Press any character to end." << endl; cin >> Temp;
              return -1;
                }
#if DUPLICATES
      if (Node[i].Duplicates<0) {
              cerr << "Error: Negative number of duplicates in node" << i << endl;
              cout << "Press any character to end." << endl; cin >> Temp;
              return -1;
                }
#endif
        if (Node[i].IntN<0) {
                cerr << "Error: Negative population number in node" << i << endl;
                cout << "Press any character to end." << endl; cin >> Temp;
                return -1;
                }
        } // for


//--------------------------- SETTING UP THE TREE ------------------------------
float   adjustN;
int     parent;

cout << "Setting up the tree." << endl;
// Initialize variables
TotalC=0;TotalN=0;
for(i=0;i<nNodes;i++) {
        Node[i].BrC=0;
        Node[i].BrN=0;
        Node[i].nChildren=0;
        }

// Calculates the total number of cases and the total population at risk
for(i=0;i<nNodes;i++) {
        TotalC+=Node[i].IntC;
        TotalN+=Node[i].IntN;
        }
// Calculates the expected counts for each node and the total.
if(Conditional) {
        adjustN=TotalC/TotalN;
        for(i=0;i<nNodes;i++) Node[i].IntN*=adjustN;
        TotalN=TotalC;
        }

// For each node, calculates the observed and expected number of cases for that
// node together with all of its children, grandchildren, etc.
// Checks whether anforlust is true or false for each node.
// Also checks whether a node is an ancestor to itslef, which is not allowed.
for(i=0;i<nNodes;i++) {
        for(j=0;j<nNodes;j++) Ancestor[j]=0;
        AddCN(i,Node[i].IntC,Node[i].IntN);
        if(Ancestor[i]>1) {
                cerr << "Error: Node " << i << " has itself as an ancestor. \n";
                cout << "Press any character to end." << endl; cin >> Temp;
                return 0;
                } // if Ancestor[i]>1
        for(j=0;j<nNodes;j++) if(Ancestor[j]>1) Node[i].Anforlust=true;
        } // for i<nNodes

// For each node calculates the number of children and sets up the list of
// child IDs
for(i=0;i<nNodes;i++) {
        for(j=0;j<Node[i].nParents;j++) {
            parent=Node[i].Parent[j];
            if(Node[parent].nChildren==MaxChildren) {
                cerr << "Too many children in node " << parent;
                cout << ". Press any character to end." << endl; cin >> Temp;
                return -1;
                }
            Node[parent].Child[Node[parent].nChildren]=i;
            Node[parent].nChildren+=1;
        } // for j
} // for i < nNodes


// Checks that no node has negative expected cases or that a node with zero expected has observed cases.
for(i=0;i<nNodes;i++) {
// cout << "Node=" << i << ", BrC=" << Node[i].BrC << ", BrN=" << Node[i].BrN << endl;
        if ( Node[i].BrN < 0 ) {
                cerr << "Error: Node " << i << " has negative expected cases.\n";
                cout << "Press any character to end." << endl; cin >> Temp;
                return 0;
                }
        if ( Node[i].BrN == 0 && Node[i].BrC>0 ) {
                cerr << "Error: Node " << i << " has observed cases but zero expected.\n";
                cout << "Press any character to end." << endl; cin >> Temp;
                return 0;
                }
        } // for i
//---------------------------- SCANNING THE TREE -------------------------------
cout << "Scanning the tree." << endl;

for(k=0;k<nCuts;k++) Cut[k].LogLikelihood=0;

for(i=0;i<nNodes;i++) {
//                                                                              cout << endl << "c=" << Node[i].BrC << ", n=" << Node[i].BrN << ", C=" << TotalC << ", N=" << TotalN;
        if(Node[i].BrC>1) {
#if DUPLICATES
                loglikelihood=PoissonLogLikelihood(Node[i].BrC-Node[i].Duplicates,Node[i].BrN,TotalC,TotalN);
#else
                if(Conditional) loglikelihood=PoissonLogLikelihood(Node[i].BrC,Node[i].BrN,TotalC,TotalN);
                else loglikelihood=UnconditionalPoissonLogLikelihood(Node[i].BrC,Node[i].BrN);
#endif
//                                                                              cout << "i=" << i << ", loglikelihood=" << loglikelihood << endl;
                k=0;
                while(loglikelihood < Cut[k].LogLikelihood && k < nCuts) k++;
                if(k < nCuts) {
                        for(m=nCuts-1;m>k;m--) {
                                Cut[m].LogLikelihood = Cut[m-1].LogLikelihood;
                                Cut[m].ID = Cut[m-1].ID;
                                Cut[m].C = Cut[m-1].C;
                                Cut[m].N = Cut[m-1].N;
                                }
                        Cut[k].LogLikelihood = loglikelihood;
                        Cut[k].ID = i;
                        Cut[k].C = Node[i].BrC;
                        Cut[k].N = Node[i].BrN;
                        }
                }
        }
if(Conditional) LogLikelihoodRatio = Cut[0].LogLikelihood-TotalC*log(TotalC/TotalN);
else LogLikelihoodRatio = Cut[0].LogLikelihood;
cout << "The log likelihood ratio of the most likely cut is " << LogLikelihoodRatio << endl;


//---------------- DOING THE MONTE CARLO SIMULATIONS ---------------------------

cout << "Doing the " << nMCReplicas << " Monte Carlo simulations:" <<  endl;
for(k=0;k<nCuts;k++) Rank[k]=1;
for(replica=0;replica<nMCReplicas;replica++) {

//-------------------- GENERATING THE RANDOM DATA ------------------------------
int     cases, CasesLeft, TotalSimC;
float   ExpectedLeft;

if(Conditional) {
        TotalSimC=TotalC;
        CasesLeft=TotalC;
        ExpectedLeft=TotalN;
        for(i=0;i<nNodes;i++) {
                cases = BinomialGenerator(CasesLeft, Node[i].IntN / ExpectedLeft);
//                                                                              if(cases>0 && Node[i].IntN<0.1) cout << "node=" << i <<  ", CasesLeft=" << CasesLeft << ", c=" << cases << ", exp=" << Node[i].IntN << ", ExpLeft=" << ExpectedLeft << endl;
                Node[i].SimIntC=cases;
                CasesLeft-=cases;
                ExpectedLeft-=Node[i].IntN;
                Node[i].SimBrC=0; // Initilazing the branch cases with zero
               } // for i
        } // if conditional
else {                 // if unconditional
        TotalSimC=0;
        ExpectedLeft=TotalN;
        for(i=0;i<nNodes;i++) {
                cases = PoissonGenerator(Node[i].IntN);
//                                                                      if(cases>0 && Node[i].IntN<0.1) cout << "node=" << i <<  ",  c=" << cases << ", exp=" << Node[i].IntN << endl;
                Node[i].SimIntC=cases;
                TotalSimC+=cases;
                ExpectedLeft-=Node[i].IntN;
                Node[i].SimBrC=0; // Initilazing the branch cases with zero
                }
      }

//------------------------ UPDATING THE TREE -----------------------------------
        for(i=0;i<nNodes;i++)
                if(Node[i].Anforlust==false) AddSimC(i,Node[i].SimIntC);
                        else AddSimCAnforlust(i,Node[i].SimIntC);


//--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
        loglikelihood=0;
        SimLogLikelihood=0;
        if (Conditional) for(i=1;i<nNodes;i++) {
                if(Node[i].SimBrC>1)
                        loglikelihood=PoissonLogLikelihood(Node[i].SimBrC,Node[i].BrN,TotalC,TotalN);
                if (loglikelihood > SimLogLikelihood )
                        SimLogLikelihood=loglikelihood;
//cout << endl << "i=" << i << " c=" << Node[i].SimBrC <<" n=" << Node[i].BrN << " LL=" << loglikelihood << " SLL=" << SimLogLikelihood ;
                } // for i<nNodes
        else for(i=1;i<nNodes;i++) {
                if(Node[i].SimBrC>1)
                        loglikelihood=UnconditionalPoissonLogLikelihood(Node[i].SimBrC,Node[i].BrN);
                if (loglikelihood > SimLogLikelihood )
                        SimLogLikelihood=loglikelihood;
                } // for i<nNodes

        cout << "The result of Monte Carlo replica #" << replica+1 << " is: ";
        if(Conditional) cout << SimLogLikelihood-TotalSimC*log(TotalSimC/TotalN) << endl;
        else cout << SimLogLikelihood << endl;
        for(k=0;k<nCuts;k++)
                if (SimLogLikelihood > Cut[k].LogLikelihood ) Rank[k]++;

        } // for i<nMCReplicas


//-------------------------- REPORT RESULTS ------------------------------------
std::string outputfile(argv[1]);
outputfile += "__output.txt";
cout << "Creating the output file: " << outputfile.c_str() << endl;
ofstream outfile(outputfile);
if(!outfile) {
        cerr << "Unable to open output file: " << outputfile.c_str() << endl;
        cout << "Press any character to end." << endl; cin >> Temp;
        return -1;
        }

cout    << endl;
cout    << "RESULTS" << endl;
outfile << "RESULTS" << endl;
if(Conditional==1) cout    << "Conditional Analysis,";
        else cout    << "Unconditional Analysis,";
if(Conditional==1) outfile    << "Conditional Analysis,";
        else outfile    << "Unconditional Analysis,";
cout    << " Total Cases:" << TotalC;
outfile << " Total Cases:" << TotalC;
cout    << " Total Measure:" << TotalN << endl;
outfile << " Total Measure:" << TotalN << endl;

outfile << endl;
outfile << "Cut# NodeID #Obs ";
#if DUPLICATES
outfile << "#CasesWithoutDuplicates ";
#endif
outfile << "#Exp O/E ";
#if DUPLICATES
outfile << "O/EWithoutDuplicates ";
#endif
outfile << "LLR pvalue" << endl;
if(Cut[0].C==0) cout << "No clusters were found." << endl;
if(Cut[0].C==0) outfile << "No clusters were found." << endl;

k=0;
outfile.setf(ios::fixed);
outfile.precision(5);
while(k<nCuts && Cut[k].C>0 && Rank[k]<nMCReplicas+1) {
        cout    << endl;
        cout    << "Most Likely Cut #" << k+1 << ":" << endl;
        outfile << k+1;
        cout    << "Node ID =" << Cut[k].ID << endl;
        outfile << " " << Cut[k].ID;
        cout    << "Number of Cases =" << Cut[k].C << endl;
        outfile << " " << Cut[k].C;
        #if DUPLICATES
        cout    << "Number of Cases (duplicates removed) =" << Cut[k].C-Node[Cut[k].ID].Duplicates << endl;
        outfile << " " << Cut[k].C-Node[Cut[k].ID].Duplicates;
        #endif
        cout    << "Expected =" << Cut[k].N << endl;
        outfile << " " << Cut[k].N;
        cout    << "O/E =" << Cut[k].C/Cut[k].N << endl;
        outfile << " " << Cut[k].C/Cut[k].N;
        #if DUPLICATES
        cout    << "O/E (duplicates removed) =" << (Cut[k].C-Node[Cut[k].ID].Duplicates)/Cut[k].N << endl;
        outfile << " " << (Cut[k].C-Node[Cut[k].ID].Duplicates)/Cut[k].N << endl;
        #endif
        if(Conditional) cout    << "Log Likelihood Ratio =" << Cut[k].LogLikelihood-TotalC*log(TotalC/TotalN) << endl;
        else cout    << "Log Likelihood Ratio =" << Cut[k].LogLikelihood << endl;
        if(Conditional) outfile << " " << Cut[k].LogLikelihood-TotalC*log(TotalC/TotalN);
        else outfile << " " << Cut[k].LogLikelihood;
        cout    << "P-value=" << (float)Rank[k] /(nMCReplicas+1) << endl;
        outfile << " " << (float)Rank[k] /(nMCReplicas+1) << endl;
        k++;
        }

outfile << endl << endl;
outfile << "Information About Each Node" << endl;
outfile << "ID Obs Exp O/E" << endl;
// outfile.width(10);
for(i=0;i<nNodes;i++)
        if(Node[i].BrN>0)
                 outfile << "0 " << Node[i].ID << " " << Node[i].BrC << " " << Node[i].BrN << " " << Node[i].BrC/Node[i].BrN << " 0 0 " << endl;



//------------------------------ END PROGRAM -----------------------------------
///// cout << "Done. Press any character to end." << endl; cin >> Temp;
time(&gEndTime); //get start time
printf("\nProgram run on: %s\n", ctime(&gStartTime));
printf("\nProgram finished on: %s\n", ctime(&gEndTime));
return 0;

} /* main() */


//-------------------------- FUNCTIONS -----------------------------------------

// Adds cases and measure through the tree from each node through the tree to
// all its parents, and so on, to all its ancestors as well.
// If a node is a decendant to an ancestor in more than one way, the cases and
// measure is only added once to that ancestor.
void AddCN(int id, int c, float n) {
    int     j,k,parent;

    Ancestor[id]=1;
    Node[id].BrC+=c;
    Node[id].BrN+=n;
    for(j=0;j<Node[id].nParents;j++) {
        parent=Node[id].Parent[j];
        if(Ancestor[parent]==0) AddCN(parent,c,n);
                else Ancestor[parent]++;
        } // for j
    } // AddCN


// Adds simulated cases up the tree from branhes to all its parents, and so on,
// for a node without anforlust.
void AddSimC(int id, int c) {
        int     j;
        Node[id].SimBrC+=c;
        for(j=0;j<Node[id].nParents;j++) AddSimC(Node[id].Parent[j],c);
        } // AddSimC

// Adds simulated cases up the tree from branhes to all its parents, and so on,
// for a node with anforlust.
// Note: This code can be made more efficient by storing in memory the ancestral
// nodes that should be updated with additional simlated cases from the node
// with internal cases. To do sometime in the future.
void AddSimCAnforlust(int id, int c) {
        int     j;
        Node[id].SimBrC+=c;
        for(j=0;j<Node[id].nParents;j++) AddSimCAnforlust(Node[id].Parent[j],c);
        } // AddSimC


//---------------- Calculates the conditional Poisson log likelihood -----------------------
float PoissonLogLikelihood(int c, float n, int TotalC, float TotalN) {
// cout << "1: " << c << " " << n << " " << TotalC << " " << TotalN ;
        if(c-n<0.0001) return 0;
        if (c==TotalC) return c*log(c/n);
        return c*log(c/n)+(TotalC-c)*log((TotalC-c)/(TotalN-n));
        } //  PoissonLogLikelihood

//---------------- Calculates the unconditional Poisson log likelihood -----------------------
float UnconditionalPoissonLogLikelihood(int c, float n) {
        if(c-n<0.0001) return 0;
        return (n-c)+c*log(c/n);
        } //  UnconditionalPoissonLogLikelihood



//------ Returns a binomial(n,p) distributed random variable -------------------
// Note: SaTScan has a faster way of doing this.
int BinomialGenerator(int n, float p) {
        int     j;
        int     binomial;
        double   r,rr;

        if(p==0) return 0;
        binomial=0;
        for (j=1;j<=n;j++) {
                if (RandomUniform() < p) binomial += 1;
                }
// cout << endl << "n=" << n << " p=" << p << " r=" << r << " rr=" << rr << " binomial=" << binomial;
        return binomial;
        } // BinomialGenerator

//------ Returns a Poisson distributed random variable -------------------
int PoissonGenerator(double lambda) {
        int     x;
        double  r,rr,p,logfactorial;

        if(lambda==0) return 0;
        x=0;
        r=RandomUniform();
        logfactorial=0;
        p=exp(-lambda);
        while (p<r) {
                x++;
                logfactorial=logfactorial+log(static_cast<double>(x));
                p=p+exp(-lambda + x*log(lambda) - logfactorial);
                }
// cout << endl << "lambda=" << lambda << " r=" << r << " x=" << x << " rr=" << rr;
        return x;
        } // PoissonGenerator


//--------------- Returns a uniform random number in the interval [0,1] --------
// Should be replaced by a better random number generator.
double RandomUniform(void) {
        //double rand_num = static_cast<double>(rand());
        //double return_value = (rand_num + 0.5) / static_cast<double>(RAND_MAX+1);
        //cout << "return_value " << return_value << endl; 
        //return return_value;
        return double(rand()+0.5)/(RAND_MAX+1);                                       // This needs a "+0.05" and "+1" or RandomUniform is zero and one too often.
        } // RandomUniform


