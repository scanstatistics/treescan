package org.treescan.gui;

import java.awt.Desktop;
import java.beans.PropertyVetoException;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.ImageIcon;
import javax.swing.JEditorPane;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.Element;
import org.treescan.app.AppConstants;
import org.treescan.app.CalculationThread;
import org.treescan.utils.EmailClientLauncher;
import org.treescan.app.Parameters;
import org.treescan.gui.utils.JDocumentRenderer;
import org.treescan.app.OutputFileRegister;
import org.treescan.utils.BareBonesBrowserLaunch;
import org.treescan.utils.FileAccess;

/**
 * Analysis execution progress/cancellation and results window.
 * @author  Hostovic
 */
public class AnalysisRunInternalFrame extends javax.swing.JInternalFrame implements InternalFrameListener {

    private boolean gbCancel = false;
    private boolean gbCanClose = false;
    private boolean _has_warnings_errors = false;
    private boolean gbCanPrint;
    private final Parameters _parameters;
    private String gsProgramErrorCallPath = "";
    private final int MAXLINES = 999;

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AnalysisRunInternalFrame(final Parameters parameters) {
        initComponents();
        setFrameIcon(new ImageIcon(getClass().getResource("/TreeScan.png")));
        addInternalFrameListener(this);
        _parameters = (Parameters)parameters.clone();
        setTitle("Running " + (_parameters.getSourceFileName().equals("") ? "Session" : _parameters.getSourceFileName()));
        new CalculationThread(this, _parameters).start();
    }

    /**
     * Returns whether window can close.
     */
    public boolean GetCanClose() {
        return gbCanClose;
    }

    /**
     * Sets whether window can close.
     */
    public void setCanClose(boolean b) {
        gbCanClose = b;
    }

    /**
     * Enables the email button.
     */
    public void enableEmailButton() {
        _emailButton.setEnabled(true);
    }

    /**
     * Set property that indicates whether printing is enabled.
     */
    public void setPrintEnabled() {
        gbCanPrint = true;
    }

    /**
     * Returns whether user has cancelled analysis.
     */
    synchronized public boolean IsJobCanceled() {
        return gbCancel;
    }

    /**
     * This method returns a new String object, ensuring it ends in newline.
     * The intended use for this method is to append text to the JTextArea
     * controls from originating JNI method. The reason for creating a new
     * object is that the JNI method uses NewStringUTF(JNIENV*, const char*)
     * and many repeated calls exhaust Java heap. The solution appears to be
     * use of method DeleteLocalRef(JNIENV*,jobject) after JNI callbacl using
     * created string. What I'm not sure about here is the behavior between
     * delayed SwingUtilities.invokeLater() call and DeleteLocalRef(JNIENV*,jobject).
     * What does it mean to possibly first call DeleteLocalRef call then invokeLater()
     * on same String object?
     */
    private String getNewInvokeLaterString(final String s) {
        return new String(s + (s.endsWith("\n") ? "" : "\n"));
    }
    
    /**
     * Prints progress string to output textarea.
     */
    synchronized public void PrintProgressWindow(final String ProgressString) {
        final String progress = getNewInvokeLaterString(ProgressString);
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                _progressTextArea.append(progress);
            }
        }); 
        
        //Limit number of lines to specified maximum. The JTextArea control is a real
        //memory hog once the number of lines gets large. With default java heap, memory
        //exhausts around the 143,000 line appended. When printing simulations to window, 
        //it's had to believe anyone would be interested in more than MAXLINES lines.
        Element root = _progressTextArea.getDocument().getDefaultRootElement();
        if (root.getElementCount() > MAXLINES) {
           Element firstLine = root.getElement(0);
           try {
             _progressTextArea.getDocument().remove(0, firstLine.getEndOffset());
           } catch(BadLocationException e) {
             gbCanClose = true; 
           }                
        }
    }

    /**
     * Prints warning/error string to output textarea.
     */
    synchronized public void PrintIssuesWindndow(final String ProgressString) {
        _has_warnings_errors = true;
        final String progress = getNewInvokeLaterString(ProgressString);
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                _warningsErrorsTextArea.append(progress);
            }
        });
    }

    /** Loads analysis results from file into memo control */
    synchronized public void LoadFromFile(final String sFileName) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                try {
                    _progressTextArea.setText("");
                    _progressTextArea.read(new FileReader(sFileName), null);
                } catch (IOException e) {
                    setTitle("Job Completed");
                    _progressTextArea.append("\nTreeScan completed successfully but was unable to read results from file.\n" +
                                             "The results have been written to: \n" + _parameters.getOutputFileName() + "\n\n");
                }
                if (_parameters.isGeneratingHtmlResults()) {
                    PrintProgressWindow("\nOpening results in web browser ...");
                    File path = new File(FileAccess.changeExtension(sFileName, ".html"));
                    if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.OPEN)) {
                        try {
                            Desktop.getDesktop().open(path);
                        } catch (IOException ex) {
                            PrintIssuesWindndow("Failed to open file: " + path.toString() + "\n");
                        }
                    } else {
                        String resultsFile = "file://localhost/" + path.getAbsolutePath();
                        BareBonesBrowserLaunch.openURL(resultsFile.replace('\\', '/'));
                    }
                } else {/* nop */}
                OutputFileRegister.getInstance().release(_parameters.getOutputFileName());
            }
        });
    }

    /**
     * Sends output window text to printer.
     */
    public void printWindow() {
        try {
            if (gbCanPrint) {
                String sPrintText = _progressTextArea.getText() + "\n\n\nWARNINGS / ERRORS\n" + _warningsErrorsTextArea.getText();
                JDocumentRenderer documentRenderer = new JDocumentRenderer();
                documentRenderer.print(new JEditorPane("text/plain", sPrintText));
            }
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Unable to print.", "Operation Failed", JOptionPane.INFORMATION_MESSAGE);
        }
    }

    /**
     * Launches default email application and creates message detailing error.
     */
    private void launchDefaultClientEmail() {
        StringBuilder messageBody = new StringBuilder();

        messageBody.append("--Job Progress Information--\n");
        messageBody.append(_progressTextArea.getText());
        messageBody.append("\n\n\n--Warnings/Errors Information--\n");
        messageBody.append(_warningsErrorsTextArea.getText());
        if (gsProgramErrorCallPath.length() > 0) {
            messageBody.append("\n\n--Call Path Information--\n\n");
            messageBody.append(gsProgramErrorCallPath);
            messageBody.append("\n\n");
        }
        messageBody.append("\n--End Of Error Message--");

        EmailClientLauncher launcher = new EmailClientLauncher();
        if (!launcher.launchDefaultClientEmail(AppConstants.getTechnicalSupportEmail(), "Automated Error Message", messageBody.toString())) {
            JOptionPane.showMessageDialog(this, "Unable to launch default email application.", "Operation Failed", JOptionPane.INFORMATION_MESSAGE);
        }
    }

    /**
     * triggers TForm::Close() -- if bForce is true, the thread is forced to terminate
     */
    public void forceClose() {
        //ForceThreadTermination();
        OutputFileRegister.getInstance().release(_parameters.getOutputFileName());
        setCanClose(true);
        dispose();
    }

    /**
     *
     */
    public void setCloseButton() {
        _cancelButton.setText("Close");
    }

    /**
     *
     */
    public void CancelJob() {
        if (getWarningsErrorsEncountered() == false) {
            _progressTextArea.append("Job cancelled. Please review 'Warnings/Errors' window below.");
        } else {
            _progressTextArea.append("Job cancelled.");
        }
        setTitle("Job cancelled");
        setCloseButton();
        gbCancel = true;
        OutputFileRegister.getInstance().release(_parameters.getOutputFileName());
        setCanClose(true);
    }

    /**
     *
     */
    public void setProgramErrorCallpathExplicit(String path) {
        gsProgramErrorCallPath = path;
    }

    /**
     *
     */
    public void setProgramErrorCallpath(StackTraceElement[] stackTrace) {
        StringBuilder trace = new StringBuilder();
        for (int i = 0; i < stackTrace.length; ++i) {
            trace.append(stackTrace[i].getMethodName() + " of " + stackTrace[i].getClassName() + "\n");
        }
        gsProgramErrorCallPath = trace.toString();
    }

    /**
     * Returns whether any warnings or errors were posted.
     */
    public boolean getWarningsErrorsEncountered() {
        try {
            return _has_warnings_errors;
        } catch (Throwable t) {return false;}
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jSplitPane1 = new javax.swing.JSplitPane();
        jScrollPane1 = new javax.swing.JScrollPane();
        _progressTextArea = new javax.swing.JTextArea();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane2 = new javax.swing.JScrollPane();
        _warningsErrorsTextArea = new javax.swing.JTextArea();
        _cancelButton = new javax.swing.JButton();
        _emailButton = new javax.swing.JButton();

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);
        setMaximizable(true);
        setResizable(true);

        jSplitPane1.setBorder(null);
        jSplitPane1.setDividerLocation(251);
        jSplitPane1.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);
        jSplitPane1.setResizeWeight(1.0);

        _progressTextArea.setColumns(20);
        _progressTextArea.setEditable(false);
        _progressTextArea.setFont(new java.awt.Font("Monospaced", 0, 11));
        _progressTextArea.setRows(5);
        jScrollPane1.setViewportView(_progressTextArea);

        jSplitPane1.setTopComponent(jScrollPane1);

        jLabel1.setText("Warnings/Errors:");

        _warningsErrorsTextArea.setEditable(false);
        jScrollPane2.setViewportView(_warningsErrorsTextArea);

        _cancelButton.setText("Cancel");
        _cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                if (_cancelButton.getText().equals("Close")) {
                    try {
                        AnalysisRunInternalFrame.this.setClosed(true);
                    } catch (PropertyVetoException e1) {
                        // TODO Auto-generated catch block
                        e1.printStackTrace();
                    }
                } else {
                    PrintProgressWindow("Cancelling job, please wait...");
                    gbCancel = true;
                }
                // gRegistry.Release(gsOutputFileName);
            }
        });

        _emailButton.setText("Email");
        _emailButton.setEnabled(false);
        _emailButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                launchDefaultClientEmail();
            }
        });

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                .addGap(429, 429, 429)
                .addComponent(_emailButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_cancelButton)
                .addContainerGap())
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addComponent(jLabel1)
                .addContainerGap())
            .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 567, Short.MAX_VALUE)
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 112, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_cancelButton)
                    .addComponent(_emailButton)))
        );

        jSplitPane1.setRightComponent(jPanel1);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jSplitPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 559, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jSplitPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 417, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    public void internalFrameOpened(InternalFrameEvent e) {
    }

    public void internalFrameClosing(InternalFrameEvent e) {
        if (GetCanClose()) {
            OutputFileRegister.getInstance().release(_parameters.getOutputFileName());
            dispose();
        }
    }

    public void internalFrameClosed(InternalFrameEvent e) {
    }

    public void internalFrameIconified(InternalFrameEvent e) {
    }

    public void internalFrameDeiconified(InternalFrameEvent e) {
    }

    public void internalFrameActivated(InternalFrameEvent e) {
    }

    public void internalFrameDeactivated(InternalFrameEvent e) {
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton _cancelButton;
    private javax.swing.JButton _emailButton;
    private javax.swing.JTextArea _progressTextArea;
    private javax.swing.JTextArea _warningsErrorsTextArea;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JSplitPane jSplitPane1;
    // End of variables declaration//GEN-END:variables
}
