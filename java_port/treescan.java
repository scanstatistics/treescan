import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Date;
import java.util.Scanner;
import java.util.Vector;
import java.util.regex.MatchResult;


public class treescan {
	public Vector<nodestructure> Nodes;
	public Vector<cutstructure> Cut;
	public Vector<Integer> Rank;
	public Vector<Integer> Ancestor;	
	private int TotalC=0;
	private double TotalN=0;
	private boolean Conditional=true;
	private boolean DUPLICATES=false;
	private int nCuts=2000;
	private int nMCReplicas=99999;

	public treescan() {
		super();
		Nodes = new Vector<nodestructure>();
		Cut = new Vector<cutstructure>();
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		treescan scan = new treescan();
		try {
			Date start = new Date(); 
			scan.run(args[0]);
			Date stop = new Date();
			System.out.println("Program run on: " + start.toString());
			System.out.println("Program finished on: " + stop.toString());
			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	private void run(String file) throws IOException {
		System.out.println("Reading the input file.");
		readInputFile(file);
		System.out.println("Setting up the tree.");
		// Initialize variables
		for(int i=0; i < Nodes.size(); i++) {
			Nodes.elementAt(i).BrC = 0;
			Nodes.elementAt(i).BrN = 0;
			Nodes.elementAt(i).nChildren = 0;
        }
		// Calculates the total number of cases and the total population at risk
		for(int i=0; i < Nodes.size(); i++) {
			TotalC += Nodes.elementAt(i).IntC;
		    TotalN += Nodes.elementAt(i).IntN;
		}
		// Calculates the expected counts for each node and the total.
		double adjustN;
		if(Conditional) {
			adjustN = TotalC/TotalN;
			for(int i=0; i < Nodes.size(); i++) 
				Nodes.elementAt(i).IntN *= adjustN;
		    TotalN=TotalC;
		}
		// For each node, calculates the observed and expected number of cases for that
		// node together with all of its children, grandchildren, etc.
		// Checks whether anforlust is true or false for each node.
		// Also checks whether a node is an ancestor to itslef, which is not allowed.
		Ancestor = new Vector<Integer>(Nodes.size());
		for (int j=0; j < Nodes.size(); j++) Ancestor.add(new Integer(0));
		for (int i=0; i < Nodes.size(); i++) {
	        for (int j=0; j < Nodes.size(); j++) Ancestor.set(j, new Integer(0));
		    AddCN(i, Nodes.elementAt(i).IntC, Nodes.elementAt(i).IntN);
		    if(Ancestor.elementAt(i).intValue() > 1) {
		    	System.out.println("Error: Node " + i + " has itself as an ancestor.");
	            return;
            } // if Ancestor[i]>1
		    for(int j=0; j < Nodes.size(); j++) if(Ancestor.elementAt(i).intValue() > 1) Nodes.elementAt(i).Anforlust=true;
        } // for i<nNodes
		// For each node calculates the number of children and sets up the list of
		// child IDs
		for (int i=0; i < Nodes.size(); i++) {
			for(int j=0; j < Nodes.elementAt(i).nParents; j++) {
				Integer parent = Nodes.elementAt(i).Parent.elementAt(j);		            
		        Nodes.elementAt(parent.intValue()).Child.add(new Integer(i));
		        Nodes.elementAt(parent.intValue()).nChildren += 1;
		    } // for j
		} // for i < nNodes
		// Checks that no node has negative expected cases or that a node with zero expected has observed cases.
		for (int i=0; i < Nodes.size(); i++) {
			// cout << "Node=" << i << ", BrC=" << Node[i].BrC << ", BrN=" << Node[i].BrN << endl;
		    if ( Nodes.elementAt(i).BrN < 0 ) {
		    	System.out.println("Error: Node " + i + " has negative expected cases.");
                return;
            }
		    if ( Nodes.elementAt(i).BrN == 0 && Nodes.elementAt(i).BrC > 0 ) {
		    	System.out.println("Error: Node " + i + " has observed cases but zero expected.");
                return;
            }
		} // for i
		
		//---------------------------- SCANNING THE TREE -------------------------------
		System.out.println("Scanning the tree.");
		double loglikelihood=0;
		double LogLikelihoodRatio=0;
		for(int k=0; k < nCuts; k++) Cut.add(new cutstructure());

		for (int i=0; i < Nodes.size(); i++) {
			//System.out.println("c=" + Nodes.elementAt(i).BrC + ", n=" + Nodes.elementAt(i).BrN + ", C=" + TotalC + ", N=" + TotalN);
		    if (Nodes.elementAt(i).BrC > 1) {
		    	if (DUPLICATES) {
		    		loglikelihood = PoissonLogLikelihood(Nodes.elementAt(i).BrC - Nodes.elementAt(i).Duplicates, Nodes.elementAt(i).BrN, TotalC, TotalN);
		    	} else {
		    		if(Conditional) loglikelihood = PoissonLogLikelihood(Nodes.elementAt(i).BrC, Nodes.elementAt(i).BrN, TotalC, TotalN);
		    		else loglikelihood = UnconditionalPoissonLogLikelihood(Nodes.elementAt(i).BrC, Nodes.elementAt(i).BrN);
		    	}
		    	//System.out.println("i=" + i + ", loglikelihood=" + loglikelihood);
		    	int k=0;
		        while(loglikelihood < Cut.elementAt(k).LogLikelihood && k < Cut.size()) k++;
               	if (k < Cut.size()) {
               		for(int m=Cut.size() - 1; m > k; m--) {
               			Cut.elementAt(m).LogLikelihood = Cut.elementAt(m - 1).LogLikelihood;
               			Cut.elementAt(m).ID = Cut.elementAt(m - 1).ID;
		                Cut.elementAt(m).C = Cut.elementAt(m - 1).C;
		                Cut.elementAt(m).N = Cut.elementAt(m - 1).N;
		            }
               		Cut.elementAt(k).LogLikelihood = loglikelihood;
               		Cut.elementAt(k).ID = i;
               		Cut.elementAt(k).C = Nodes.elementAt(i).BrC;
               		Cut.elementAt(k).N = Nodes.elementAt(i).BrN;
		        }
		    }
		}
		if(Conditional) LogLikelihoodRatio = Cut.elementAt(0).LogLikelihood - TotalC * Math.log(TotalC/TotalN);
		else LogLikelihoodRatio = Cut.elementAt(0).LogLikelihood;
		System.out.println("The log likelihood ratio of the most likely cut is " + LogLikelihoodRatio);

		//---------------- DOING THE MONTE CARLO SIMULATIONS ---------------------------

		System.out.println("Doing the " + nMCReplicas + " Monte Carlo simulations:");
		Rank = new Vector<Integer>(Cut.size());
		for(int k=0; k < Cut.size() ;k++) Rank.add(new Integer(1));
		for(int replica=0; replica < nMCReplicas; replica++) {
			//-------------------- GENERATING THE RANDOM DATA ------------------------------
			int      cases, CasesLeft, TotalSimC;
			double   ExpectedLeft;

			if (Conditional) {
				TotalSimC=TotalC;
		        CasesLeft=TotalC;
		        ExpectedLeft=TotalN;
		        for (int i=0; i < Nodes.size(); i++) {
		        	cases = BinomialGenerator(CasesLeft, Nodes.elementAt(i).IntN / ExpectedLeft);
		        	//if(cases>0 && Nodes.elementAt(i).IntN<0.1) System.out.println("node=" + i +  ", CasesLeft=" + CasesLeft + ", c=" + cases + ", exp=" + Nodes.elementAt(i).IntN + ", ExpLeft=" + ExpectedLeft);
		        	Nodes.elementAt(i).SimIntC = cases;
	                CasesLeft -= cases;
	                ExpectedLeft -= Nodes.elementAt(i).IntN;
	                Nodes.elementAt(i).SimBrC=0; // Initilazing the branch cases with zero
		        } // for i
		    } // if conditional
			else {                 // if unconditional
				TotalSimC=0;
		        ExpectedLeft=TotalN;
		        for (int i=0; i < Nodes.size(); i++) {
		                cases = PoissonGenerator(Nodes.elementAt(i).IntN);
		                //if(cases>0 && Nodes.elementAt(i).IntN<0.1) System.out.println("node=" + i +  ",  c=" + cases + ", exp=" + Nodes.elementAt(i).IntN);
		                Nodes.elementAt(i).SimIntC = cases;
		                TotalSimC += cases;
		                ExpectedLeft -= Nodes.elementAt(i).IntN;
		                Nodes.elementAt(i).SimBrC=0; // Initilazing the branch cases with zero
		        }
		    }

			//------------------------ UPDATING THE TREE -----------------------------------
			for (int i=0; i < Nodes.size(); i++) {
				if(Nodes.elementAt(i).Anforlust==false) AddSimC(i, Nodes.elementAt(i).SimIntC);
				else AddSimCAnforlust(i,Nodes.elementAt(i).SimIntC);
			}

			//--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
			loglikelihood=0;
			double SimLogLikelihood=0;
			if (Conditional) {
				for(int i=1; i < Nodes.size(); i++) {
					if(Nodes.elementAt(i).SimBrC > 1)
						loglikelihood=PoissonLogLikelihood(Nodes.elementAt(i).SimBrC, Nodes.elementAt(i).BrN,TotalC,TotalN);
		            if (loglikelihood > SimLogLikelihood )
		            	SimLogLikelihood=loglikelihood;
		            //System.out.println("i=" + i + " c=" + Nodes.elementAt(i).SimBrC + " n=" + Nodes.elementAt(i).BrN + " LL=" + loglikelihood + " SLL=" + SimLogLikelihood);
				} // for i<nNodes
			} else { 
				for(int i=1; i < Nodes.size();i++) {
					if(Nodes.elementAt(i).SimBrC > 1)
						loglikelihood = UnconditionalPoissonLogLikelihood(Nodes.elementAt(i).SimBrC, Nodes.elementAt(i).BrN);
					if (loglikelihood > SimLogLikelihood )
						SimLogLikelihood=loglikelihood;
				} // for i<nNodes
			}
			System.out.print("The result of Monte Carlo replica #" + (replica+1) + " is: ");
			if(Conditional) System.out.println(SimLogLikelihood - TotalSimC * Math.log(TotalSimC/TotalN));
			else System.out.println(SimLogLikelihood);
			for(int k=0; k < Cut.size();k++)
				if (SimLogLikelihood > Cut.elementAt(k).LogLikelihood ) Rank.set(k, Rank.elementAt(k) + 1);
		} // for i<nMCReplicas	
		
		//-------------------------- REPORT RESULTS ------------------------------------
		System.out.println("Creating the output file.");
	    FileWriter fstream = new FileWriter(file.toString() + ".out.txt");
        BufferedWriter out = new BufferedWriter(fstream);
        System.out.println("RESULTS");
        out.write("RESULTS" + System.getProperty( "line.separator" ));
		if(Conditional) System.out.println("Conditional Analysis,");
		else System.out.println("Unconditional Analysis,");
		if(Conditional) out.write("Conditional Analysis,");
		else out.write("Unconditional Analysis,");
		System.out.println(" Total Cases:" + TotalC);
		out.write(" Total Cases:" + TotalC);
		System.out.println(" Total Measure:" + TotalN);
		out.write(" Total Measure:" + TotalN + System.getProperty( "line.separator" ));

		out.write(System.getProperty( "line.separator" ));
		out.write("Cut# NodeID #Obs ");
		if (DUPLICATES)
			out.write("#CasesWithoutDuplicates ");
		out.write("#Exp O/E ");
		if (DUPLICATES)
			out.write("O/EWithoutDuplicates ");
		out.write("LLR pvalue" + System.getProperty( "line.separator" ));
		if(Cut.elementAt(0).C == 0) System.out.println("No clusters were found.");
		if(Cut.elementAt(0).C == 0) out.write("No clusters were found." + System.getProperty( "line.separator" ));

		int k=0;
		//outfile.setf(ios::fixed);
		//outfile.precision(5);
		while(k < Cut.size() && Cut.elementAt(k).C > 0 && Rank.elementAt(k) < nMCReplicas + 1) {
			System.out.println("");
			System.out.println("Most Likely Cut #" + k+1 + ":");
			out.write(k+1);
			System.out.println("Node ID =" + Cut.elementAt(k).ID);
	        out.write(" " + Cut.elementAt(k).ID);
	        System.out.println("Number of Cases =" + Cut.elementAt(k).C);
	        out.write(" " + Cut.elementAt(k).C);
	        if (DUPLICATES) {
	        	System.out.println("Number of Cases (duplicates removed) =" + (Cut.elementAt(k).C - Nodes.elementAt(Cut.elementAt(k).ID).Duplicates));
	        	out.write(" " + (Cut.elementAt(k).C - Nodes.elementAt(Cut.elementAt(k).ID).Duplicates));
	        }
	        System.out.println("Expected =" + Cut.elementAt(k).N);
	        out.write(" " + Cut.elementAt(k).N);
	        System.out.println("O/E =" + Cut.elementAt(k).C/Cut.elementAt(k).N);
	        out.write(" " + Cut.elementAt(k).C/Cut.elementAt(k).N);
	        if (DUPLICATES) {
	        	System.out.println("O/E (duplicates removed) =" + ((Cut.elementAt(k).C - Nodes.elementAt(Cut.elementAt(k).ID).Duplicates)/Cut.elementAt(k).N));
	        	out.write(" " + ((Cut.elementAt(k).C - Nodes.elementAt(Cut.elementAt(k).ID).Duplicates)/Cut.elementAt(k).N) + System.getProperty( "line.separator" ));
	        }
		    if(Conditional) System.out.println("Log Likelihood Ratio =" + (Cut.elementAt(k).LogLikelihood - TotalC * Math.log(TotalC/TotalN)));
		    else System.out.println("Log Likelihood Ratio =" + Cut.elementAt(k).LogLikelihood);
		    if(Conditional) out.write(" " + (Cut.elementAt(k).LogLikelihood - TotalC * Math.log(TotalC/TotalN)));
		    else out.write(" " + Cut.elementAt(k).LogLikelihood);
		    System.out.println("P-value=" + (((double)Rank.elementAt(k))/((double)(nMCReplicas+1))));
		    out.write(" " + (Rank.elementAt(k)/(nMCReplicas+1)) + System.getProperty( "line.separator" ));
		    k++;
		}

		out.write(System.getProperty( "line.separator" ) + System.getProperty( "line.separator" ));
		out.write("Information About Each Node" + System.getProperty( "line.separator" ));
		out.write("ID Obs Exp O/E" + System.getProperty( "line.separator" ));
		// outfile.width(10);
		for(int i=0; i < Nodes.size();i++) {
			if (Nodes.elementAt(i).BrN > 0)
				out.write("0 " + Nodes.elementAt(i).ID + " " + Nodes.elementAt(i).BrC + " " + Nodes.elementAt(i).BrN + " " + Nodes.elementAt(i).BrC/Nodes.elementAt(i).BrN + " 0 0 " + System.getProperty( "line.separator" ));
		}
		out.close();
		fstream.close();
		//------------------------------ END PROGRAM -----------------------------------				
	}
		
	
	//-------------------------- FUNCTIONS -----------------------------------------

	// Adds cases and measure through the tree from each node through the tree to
	// all its parents, and so on, to all its ancestors as well.
	// If a node is a decendant to an ancestor in more than one way, the cases and
	// measure is only added once to that ancestor.
	private void AddCN(int id, int c, double n) {
	    Ancestor.set(id, new Integer(1));
	    Nodes.elementAt(id).BrC += c;
	    Nodes.elementAt(id).BrN += n;
	    for(int j=0; j < Nodes.elementAt(id).nParents; j++) {
	    	int parent = Nodes.elementAt(id).Parent.elementAt(j).intValue();
	        if(Ancestor.elementAt(parent).intValue() == 0) 
	        	AddCN(parent,c,n);
	        else 
	        	Ancestor.set(parent, new Integer(Ancestor.elementAt(parent).intValue() + 1));
	    } // for j
	} // AddCN	
	
	// Adds simulated cases up the tree from branhes to all its parents, and so on,
	// for a node without anforlust.
	private void AddSimC(int id, int c) {
		Nodes.elementAt(id).SimBrC += c;
        for(int j=0; j <Nodes.elementAt(id).nParents; j++) 
        	AddSimC(Nodes.elementAt(id).Parent.elementAt(j),c);
	} // AddSimC

	// Adds simulated cases up the tree from branhes to all its parents, and so on,
	// for a node with anforlust.
	// Note: This code can be made more efficient by storing in memory the ancestral
	// nodes that should be updated with additional simlated cases from the node
	// with internal cases. To do sometime in the future.
	private void AddSimCAnforlust(int id, int c) {
		Nodes.elementAt(id).SimBrC+=c;
        for(int j=0;j<Nodes.elementAt(id).nParents;j++) 
        	AddSimCAnforlust(Nodes.elementAt(id).Parent.elementAt(j),c);
	} // AddSimC
	
	//---------------- Calculates the conditional Poisson log likelihood -----------------------
	private double PoissonLogLikelihood(int c, double n, int TotalC, double TotalN) {
		//System.out.println("1: " + c + " " + n + " " + TotalC + " " + TotalN);
	    if(c - n < 0.0001) return 0;
        if (c == TotalC) return c * Math.log(c/n);
        return c * Math.log(c/n) + (TotalC-c) * Math.log((TotalC-c)/(TotalN-n));
    } //  PoissonLogLikelihood

	//---------------- Calculates the unconditional Poisson log likelihood -----------------------
	private double UnconditionalPoissonLogLikelihood(int c, double n) {
		if(c - n < 0.0001) return 0;
	    return (n-c) + c * Math.log(c/n);
    } //  UnconditionalPoissonLogLikelihood	
	
	//------ Returns a binomial(n,p) distributed random variable -------------------
	// Note: SaTScan has a faster way of doing this.
	int BinomialGenerator(int n, double p) {
        int     binomial=0;
        double   r=0,rr=0;

        if(p==0) return 0;
        for (int j=1; j <=n; j++) {
        	if (RandomUniform() < p) binomial += 1;
	    }
	    //System.out.println("n=" + n + " p=" + p + " r=" + r + " rr=" + rr + " binomial=" + binomial);
	    return binomial;
	} // BinomialGenerator

	//------ Returns a Poisson distributed random variable -------------------
	int PoissonGenerator(double lambda) {
		Integer x = new Integer(0);
	    double  r,rr=0,p,logfactorial;

        if(lambda==0) return 0;
        r=RandomUniform();
        logfactorial=0;
        p=Math.exp(-lambda);
        while (p<r) {
        	x++;
	        logfactorial = logfactorial + Math.log(x.doubleValue());
	        p = p + Math.exp(-lambda + x.doubleValue() * Math.log(lambda) - logfactorial);
	    }
        //System.out.println("lambda=" + lambda + " r=" + r + " x=" + x + " rr=" + rr);
        return x;
    } // PoissonGenerator

	//--------------- Returns a uniform random number in the interval [0,1] --------
	// Should be replaced by a better random number generator.
	double RandomUniform() {
		double return_value = Math.random();
		//System.out.println("return_value " + return_value);
		return return_value;
		//return (Math.random() + 0.5)/(RAND_MAX+1); // This needs a "+0.05" and "+1" or RandomUniform is zero and one too often.
	} // RandomUniform	
	
	private void readInputFile(String file) {		
        Scanner lineScanner = null;
        try {
        	lineScanner = new Scanner(new BufferedReader(new FileReader(file)));
            while (lineScanner.hasNextLine()) {
            	Scanner s = new Scanner(lineScanner.nextLine());
                s.findInLine("\\s*(\\d+)\\s+(\\d+)\\s+(\\d+\\.?\\d*)\\s+(\\d+)\\s+(\\d*)\\s*");
                MatchResult result = s.match();
            	nodestructure n = new nodestructure();
            	n.ID = Integer.parseInt(result.group(1));
            	//System.out.println("n.ID " + n.ID);
            	n.IntC = Integer.parseInt(result.group(2));
            	n.IntN = Double.parseDouble(result.group(3));
              	n.nParents = Integer.parseInt(result.group(4));
              	
              	// This part is not correct if there are more than one parent -- perhaps regex is not correct animal here!
              	n.Parent = new Vector<Integer>(n.nParents);
              	for(int j=0; j < n.nParents; j++) {
              		n.Parent.add(Integer.parseInt(result.group(5)));
              	}
            	//System.out.println("result.groupCount() " + result.groupCount());
            	
            	Nodes.add(n);
                //System.out.println("next");
                //System.out.println(n.ID);
            }
            //System.out.println("done");
        } catch (FileNotFoundException e) {
        	e.printStackTrace();
        } catch (IllegalStateException e) {
        	e.printStackTrace();        	
        } finally {
            if (lineScanner != null) {
            	lineScanner.close();
            }
        }	
	}
	
}
