import java.util.Vector;


public class nodestructure {
    public int     ID=-1;            // The node ID.
    public int     IntC=0, SimIntC=0; // Number of true and simulated cases internal to the node, respectively.
    public int     BrC=0, SimBrC=0;   // Number of true and simulated cases in the node and all decendants (children, grandchildren etc.)
    public double   IntN=0, BrN=0;      // Expexted number of cases internal to the node, and with all decendants respectively.
    public int     nChildren=0, nParents=0;  // Number of children and parents of that node, respectively.
    public Vector<Integer> Child; // List of node IDs of the children and parents
    public Vector<Integer> Parent; 
    public boolean    Anforlust=false;      // =1 if at least one node is an ancestor in more than one way, otherwise =0
//#if DUPLICATES
    public int     Duplicates=0;     // Number of duplicates that needs to be removed.
//#endif
    
    
    
    public nodestructure() {
    	super();    	
    	Child = new Vector<Integer>();
    	Parent = new Vector<Integer>();
    }
   
    @Override
    public Object clone() { 
      try {
    	  nodestructure newObject = (nodestructure)super.clone(); 
    	  newObject.Child = new Vector<Integer>(Child);
    	  newObject.Parent = new Vector<Integer>(Parent);

    	  return newObject; 
      } 
      catch (CloneNotSupportedException e) {
        throw new InternalError("But we are Cloneable!!!");
      }
    }
    
}
