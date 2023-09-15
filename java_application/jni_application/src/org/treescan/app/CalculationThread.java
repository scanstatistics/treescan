package org.treescan.app;

import java.io.File;
import org.treescan.gui.AnalysisRunInternalFrame;
import org.treescan.utils.FileAccess;

/**
 * Thread class for executing analysis through call to Java Native Interface.
 */
public class CalculationThread extends Thread {

    private final AnalysisRunInternalFrame AnalysisRun;
    final private Parameters _parameters;
    String gsProgramErrorCallPath = "";

    public native int RunAnalysis(Parameters jparameters);

    public CalculationThread(AnalysisRunInternalFrame AnalysisRun, final Parameters Parameters) {
        super(Parameters.getSourceFileName());
        this.AnalysisRun = AnalysisRun;
        this._parameters = Parameters;
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
            if (RunAnalysis(_parameters) == 0) {
                if (AnalysisRun.IsJobCanceled()) {
                    //analysis cancelled by user -- acknowledge that engine has terminated
                    AnalysisRun.setTitle("Job cancelled");
                    AnalysisRun.PrintProgressWindow("Job cancelled by user.");
                } else {
                    OutputFileRegister.getInstance().release(_parameters.getOutputFileName());
                    String resultsFilename = _parameters.getOutputFileName();
                    if (_parameters.isSequentialScanTreeOnly()) {
                        // The generated output files are created with '_look#', so we need to update output filename.
                        resultsFilename = FileAccess.changeExtension(
                            resultsFilename,
                            "_look" + _parameters.getLookNumber(resultsFilename) + FileAccess.getExtension(new File(resultsFilename))
                        );
                    }
                    AnalysisRun.LoadFromFile(resultsFilename);
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
