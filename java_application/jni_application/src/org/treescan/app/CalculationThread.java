package org.treescan.app;

import org.treescan.gui.AnalysisRunInternalFrame;

/**
 * Thread class for executing analysis through call to Java Native Interface.
 */
public class CalculationThread extends Thread {

    private final AnalysisRunInternalFrame AnalysisRun;
    final private Parameters Parameters;
    String gsProgramErrorCallPath = "";

    public native int RunAnalysis(Parameters jparameters);

    public CalculationThread(AnalysisRunInternalFrame AnalysisRun, final Parameters Parameters) {
        super(Parameters.getSourceFileName());
        this.AnalysisRun = AnalysisRun;
        this.Parameters = Parameters;
    }

    synchronized private boolean IsCancelled() {
        return AnalysisRun.IsJobCanceled();
    }

    synchronized private void PrintStandard(java.lang.String line) {
        AnalysisRun.PrintProgressWindow(line);
    }

    synchronized public void PrintError(String line) {
        AnalysisRun.PrintIssuesWindndow(line);
    }

    synchronized public void PrintWarning(String line) {
        AnalysisRun.PrintIssuesWindndow(line);
    }

    synchronized public void PrintNotice(String line) {
        AnalysisRun.PrintIssuesWindndow(line);
    }

    synchronized public void setCallpath(String sCallpath) {
        gsProgramErrorCallPath = sCallpath;
    }

    /**
     * Starts thread execution -- makes call to native code through JNI function. 
     */
    @Override
    public void run() {
        try {
            if (RunAnalysis(Parameters) == 0) {
                if (AnalysisRun.IsJobCanceled()) {
                    //analysis cancelled by user -- acknowledge that engine has terminated
                    AnalysisRun.setTitle("Job cancelled");
                    AnalysisRun.PrintProgressWindow("Job cancelled by user.");
                } else {
                    AnalysisRun.LoadFromFile(Parameters.getOutputFileName());
                }
            } else {
                AnalysisRun.enableEmailButton();
                AnalysisRun.setProgramErrorCallpathExplicit(gsProgramErrorCallPath);
                AnalysisRun.CancelJob();
            }

            AnalysisRun.setCanClose(true);
            AnalysisRun.setCloseButton();
            AnalysisRun.setPrintEnabled();
            if (!AnalysisRun.getWarningsErrorsEncountered()) {
                AnalysisRun.PrintIssuesWindndow("No Warnings or Errors.");
            }
        } catch (Throwable t) {
            PrintError(t.getMessage());
            AnalysisRun.setProgramErrorCallpath(t.getStackTrace());
            AnalysisRun.enableEmailButton();
            AnalysisRun.CancelJob();
        }

    }
}
