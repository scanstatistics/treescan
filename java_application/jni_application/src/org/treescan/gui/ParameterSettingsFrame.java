package org.treescan.gui;

import java.awt.Component;
import java.awt.Container;
import java.beans.PropertyVetoException;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JRootPane;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.treescan.utils.FileAccess;
import org.treescan.importer.FileImporter;
import org.treescan.app.ParameterHistory;
import org.treescan.app.Parameters;
import org.treescan.app.UnknownEnumException;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.gui.utils.Utils;

/**
 * Parameter settings window.
 * @author  Hostovic
 */
public class ParameterSettingsFrame extends javax.swing.JInternalFrame implements InternalFrameListener {

    private Parameters _parameters = new Parameters();
    private Parameters _initialParameters = new Parameters();
    private boolean gbPromptOnExist = true;
    private final UndoManager undo = new UndoManager();
    private final JRootPane _rootPane;
    final static String STUDY_COMPLETE = "study_complete";
    final static String STUDY_GENERIC = "study_generic";


    /**
     * Creates new form ParameterSettingsFrame
     */
    public ParameterSettingsFrame(final JRootPane rootPane, final String sParameterFilename) {
        initComponents();
        setFrameIcon(new ImageIcon(getClass().getResource("/TreeScan.png")));
        _rootPane = rootPane;
        addInternalFrameListener(this);
        setUp(sParameterFilename);
    }

    /**
     * launches 'save as' dialog to permit user saving current settings to parameter file
     */
    public boolean SaveAs() {
        boolean bSaved = true;

        JFileChooser fc = new JFileChooser(TreeScanApplication.getInstance().lastBrowseDirectory);
        fc.setDialogTitle("Save Parameter Settings As");
        fc.addChoosableFileFilter(new InputFileFilter("prm", "Parameter Files (*.prm)"));
        int returnVal = fc.showSaveDialog(this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
            String filename = fc.getSelectedFile().getAbsolutePath();
            if (fc.getSelectedFile().getName().indexOf('.') == -1){
                filename = filename + ".prm";
            } 
            WriteSession(filename);
            setTitle(filename);
        } else {
            bSaved = false;
        }
        return bSaved;
    }

    /**
     * Writes the session information to disk
     */
    public boolean WriteSession(String sParameterFilename) {
        String sFilename = sParameterFilename;
        boolean bSaved = true;

        if (sFilename.equals("")) {
            sFilename = _parameters.getSourceFileName();
        }
        if (sFilename == null || sFilename.equals("")) {
            bSaved = SaveAs();
        } else {
            if (!FileAccess.ValidateFileAccess(sFilename, true)) {
                JOptionPane.showInternalMessageDialog(this, "Unable to save session parameters.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to write\nto this directory and file.");
            } else {
                saveParameterSettings(_parameters);
                _parameters.setSourceFileName(sFilename);
                _parameters.Write(sFilename);
                _initialParameters = (Parameters) _parameters.clone();
            }
        }
        return bSaved;
    }

    /**
     * If necessary, removes from from iconized state and brings to front.
     */
    public void focusWindow() {
        if (this.isIcon()) {
            try {
                this.setIcon(false);
            } catch (PropertyVetoException e) {
                return;
            }
        }
        toFront();
    }
    
    /** Determines whether window can be closed by comparing parameter settings contained
     * in window verse intial parameter settings. */
    public boolean QueryWindowCanClose() {
        boolean bReturn = true;

        saveParameterSettings(_parameters);
        if (!_parameters.equals(_initialParameters)) {
            focusWindow();
            switch (JOptionPane.showInternalConfirmDialog(this, "Parameter settings have changed. Do you want to save?", "Save?", JOptionPane.YES_NO_CANCEL_OPTION)) {
                case JOptionPane.YES_OPTION:
                    if (WriteSession("")) {
                        gbPromptOnExist = false;
                    } else {
                        bReturn = false;
                    }
                    break;
                case JOptionPane.CANCEL_OPTION:
                    bReturn = false;
                    break;
                default:
                    gbPromptOnExist = false;
            }
        }
        return bReturn;
    }

    /** Resets parameters that are not present in interface to default value.
     * Hidden features are to be used soley in command line version at this time. */
    private void defaultHiddenParameters() {
        //TODO
    }

    /**
     * Reads parameter settings from file and loads frames controls.
     */
    private void setUp(final String sParameterFileName) {
        if (sParameterFileName.length() > 0) {
            _parameters.Read(sParameterFileName);
        }
        //catch (ZdException &x) {
        //  x.SetLevel(ZdException::Notify);
        //  x.SetErrorMessage((const char*)"SaTScan is unable to read parameters from file \"%s\".\n", sParameterFileName);
        //  throw;
        //}
        defaultHiddenParameters();
        setupInterface(_parameters);

        //Save orginal parameter settings to compare against when window closes but
        //first save what the interface has produced for the settings read from file.
        saveParameterSettings(_parameters);
        _initialParameters = (Parameters) _parameters.clone();
    }

    /**
     * Validates 'Input Files' tab
     */
    private void ValidateInputFiles() {
        //validate the case file
        if (_countFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a case file.", (Component) _countFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_countFileTextField.getText(), false)) {
            throw new SettingsException("The count file could not be opened for reading.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to read\nfrom this directory and file.",  (Component) _countFileTextField);
        }
        //validate the tree file
        if (_treelFileTextField.getText().length() == 0) {
            throw new SettingsException("For the Bernoulli model, please specify a control file.", (Component) _treelFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_treelFileTextField.getText(), false)) {
            throw new SettingsException("The tree file could not be opened for reading.\n" + "Please confirm that the path and/or file name are\n" + "valid and that you have permissions to read from\nthis directory and file.", (Component) _treelFileTextField);
        }
    }

    /**
     * Verifies all parameters on the 'Output Files' tab. Returns whether tab is valid.
     */
    private void CheckOutputParams() {
        if (_outputFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a results file.", (Component) _outputFileTextField);
        }
        if (_reportResultsAsHTML.isSelected() && !(_outputFileTextField.getText().endsWith(".htm") || _outputFileTextField.getText().endsWith(".html"))) {
            throw new SettingsException("Outputting results as HTML requires a file extension of '.html'.", (Component) _outputFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_outputFileTextField.getText(), true)) {
            throw new SettingsException("Results file could not be opened for writing.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to write\nto this directory and file.",
                    (Component) _outputFileTextField);
        }
    }

    /**
     *
     */
    public boolean ValidateParams() {
        try {
            ValidateInputFiles();
            CheckOutputParams();
        } catch (SettingsException e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            e.setControlFocus();
            return false;
        }
        return true;
    }
    /** setup interface from parameter settings */
    private void setupInterface(final Parameters parameters) {
        title = parameters.getSourceFileName();
        if (title == null || title.length() == 0) {
            title = "New Session";
        }
        _treelFileTextField.setText(parameters.getTreeFileName());
        _treelFileTextField.setCaretPosition(0);
        _countFileTextField.setText(parameters.getCountFileName());
        _countFileTextField.setCaretPosition(0);
        _duplicatesInCaseFile.setSelected(parameters.isDuplicates());
        _outputFileTextField.setText(parameters.getOutputFileName());
        _outputFileTextField.setCaretPosition(0);
        _reportResultsAsHTML.setSelected(parameters.getResultsFormat() == Parameters.ResultsFormat.HTML);
        _montCarloReplicationsTextField.setText(Integer.toString(parameters.getNumReplicationsRequested()));
        _conditionalAnalysis.setSelected(parameters.isConditional());
    }
    /** sets CParameters class with settings in form */
    private void saveParameterSettings(Parameters parameters) {
        setTitle(parameters.getSourceFileName());
        parameters.setTreeFileName(_treelFileTextField.getText());
        parameters.setCountFileName(_countFileTextField.getText());
        parameters.setDuplicates(_duplicatesInCaseFile.isSelected());
        parameters.setOutputFileName(_outputFileTextField.getText());
        parameters.setResultsFormat(_reportResultsAsHTML.isSelected() ? Parameters.ResultsFormat.HTML.ordinal() : Parameters.ResultsFormat.TEXT.ordinal());
        parameters.setNumReplications(Integer.parseInt(_montCarloReplicationsTextField.getText()));
        parameters.setConditional(_conditionalAnalysis.isSelected());
    }
    public final Parameters getParameterSettings() {
        saveParameterSettings(_parameters);
        return _parameters;
    }
    public void showExecOptionsDialog(java.awt.Frame parent) {
        new ExecutionOptionsDialog(parent, _parameters).setVisible(true);
    }
    /** Modally shows import dialog. */
    public void LaunchImporter(String sFileName, FileImporter.InputFileType eFileType) {
        ImportWizardDialog wizard = new ImportWizardDialog(TreeScanApplication.getInstance(), sFileName, eFileType);
        wizard.setVisible(true);
        if (!wizard.getCancelled()) {
            switch (eFileType) {  // set parameters
                case Tree:
                    _treelFileTextField.setText(wizard.getDestinationFilename());
                    break;
                case Counts:
                    _countFileTextField.setText(wizard.getDestinationFilename());
                    break;
                default:
                    throw new UnknownEnumException(eFileType);
            }
        }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _inputTab = new javax.swing.JPanel();
        _resultsFileLabel = new javax.swing.JLabel();
        _outputFileTextField = new javax.swing.JTextField();
        _resultsFileBrowseButton = new javax.swing.JButton();
        _caseFileLabel = new javax.swing.JLabel();
        _countFileTextField = new javax.swing.JTextField();
        _countFileBrowseButton = new javax.swing.JButton();
        _countFileImportButton = new javax.swing.JButton();
        _treelFileTextField = new javax.swing.JTextField();
        _controlFileLabel = new javax.swing.JLabel();
        _treeFileBrowseButton = new javax.swing.JButton();
        _treeFileImportButton = new javax.swing.JButton();
        _duplicatesInCaseFile = new javax.swing.JCheckBox();
        jPanel1 = new javax.swing.JPanel();
        _labelMonteCarloReplications = new javax.swing.JLabel();
        _montCarloReplicationsTextField = new javax.swing.JTextField();
        _conditionalAnalysis = new javax.swing.JCheckBox();
        _reportResultsAsHTML = new javax.swing.JCheckBox();

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);

        _resultsFileLabel.setText("Results File:"); // NOI18N

        _resultsFileBrowseButton.setText("..."); // NOI18N
        _resultsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Results File");
                fc.addChoosableFileFilter(new InputFileFilter("html","Results Files (*.html)"));
                fc.addChoosableFileFilter(new InputFileFilter("txt","Results Files (*.txt)"));
                if (fc.showSaveDialog(ParameterSettingsFrame.this) == JFileChooser.APPROVE_OPTION) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _outputFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _caseFileLabel.setText("Count File:"); // NOI18N

        _countFileBrowseButton.setText("..."); // NOI18N
        _countFileBrowseButton.setToolTipText("Browse for count file ..."); // NOI18N
        _countFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Count File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("cts","Count Files (*.cts)"));
                int returnVal = fc.showOpenDialog(ParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _countFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _countFileImportButton.setText("..."); // NOI18N
        _countFileImportButton.setToolTipText("Import count file ..."); // NOI18N
        _countFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    JFileChooser fc = new JFileChooser(org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                    fc.setDialogTitle("Select Counts File Import Source");
                    fc.addChoosableFileFilter(new InputFileFilter("dbf","dBase Files (*.dbf)"));
                    fc.addChoosableFileFilter(new InputFileFilter("csv","Delimited Files (*.csv)"));
                    fc.addChoosableFileFilter(new InputFileFilter("xls","Excel Files (*.xls)"));
                    fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                    fc.addChoosableFileFilter(new InputFileFilter("cts","Count Files (*.cts)"));
                    int returnVal = fc.showOpenDialog(ParameterSettingsFrame.this);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                        LaunchImporter(fc.getSelectedFile().getAbsolutePath(), FileImporter.InputFileType.Counts);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(org.treescan.gui.TreeScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        _controlFileLabel.setText("Tree File:"); // NOI18N

        _treeFileBrowseButton.setText("..."); // NOI18N
        _treeFileBrowseButton.setToolTipText("Browse for tree file ..."); // NOI18N
        _treeFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Tree File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("tre","Tree Files (*.tre)"));
                int returnVal = fc.showOpenDialog(ParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _treelFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _treeFileImportButton.setText("..."); // NOI18N
        _treeFileImportButton.setToolTipText("Import tree file ..."); // NOI18N
        _treeFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    JFileChooser fc = new JFileChooser(org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                    fc.setDialogTitle("Select Tree File Import Source");
                    fc.addChoosableFileFilter(new InputFileFilter("dbf","dBase Files (*.dbf)"));
                    fc.addChoosableFileFilter(new InputFileFilter("csv","Delimited Files (*.csv)"));
                    fc.addChoosableFileFilter(new InputFileFilter("xls","Excel Files (*.xls)"));
                    fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                    fc.addChoosableFileFilter(new InputFileFilter("tre","Tree Files (*.tre)"));
                    int returnVal = fc.showOpenDialog(ParameterSettingsFrame.this);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                        LaunchImporter(fc.getSelectedFile().getAbsolutePath(), FileImporter.InputFileType.Tree);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(org.treescan.gui.TreeScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        _duplicatesInCaseFile.setText("data contains duplicates field");

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder("Advanced Settings"));

        _labelMonteCarloReplications.setText("Number of Monte Carlo replications (0, 9, 999, or value ending in 999):"); // NOI18N

        _montCarloReplicationsTextField.setText("999"); // NOI18N
        _montCarloReplicationsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_montCarloReplicationsTextField.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _montCarloReplicationsTextField.setText("999");
            }
        });
        _montCarloReplicationsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_montCarloReplicationsTextField, e, 10);
            }
        });
        _montCarloReplicationsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _conditionalAnalysis.setText("Perform conditional analysis");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_conditionalAnalysis)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(_labelMonteCarloReplications)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 88, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(121, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_labelMonteCarloReplications)
                    .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_conditionalAnalysis)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _reportResultsAsHTML.setText("report results as HTML");

        javax.swing.GroupLayout _inputTabLayout = new javax.swing.GroupLayout(_inputTab);
        _inputTab.setLayout(_inputTabLayout);
        _inputTabLayout.setHorizontalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addGap(10, 10, 10)
                        .addComponent(_reportResultsAsHTML))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                        .addComponent(_treelFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 513, Short.MAX_VALUE)
                        .addGap(6, 6, 6)
                        .addComponent(_treeFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_treeFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_controlFileLabel)
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addComponent(_countFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 513, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_countFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_countFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addGap(10, 10, 10)
                        .addComponent(_duplicatesInCaseFile, javax.swing.GroupLayout.DEFAULT_SIZE, 565, Short.MAX_VALUE))
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addComponent(_caseFileLabel)
                        .addGap(521, 521, 521))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                        .addComponent(_outputFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 534, Short.MAX_VALUE)
                        .addGap(16, 16, 16)
                        .addComponent(_resultsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_resultsFileLabel)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _inputTabLayout.setVerticalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_controlFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_treelFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_treeFileImportButton)
                    .addComponent(_treeFileBrowseButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_caseFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_countFileBrowseButton)
                            .addComponent(_countFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_duplicatesInCaseFile, javax.swing.GroupLayout.PREFERRED_SIZE, 14, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addComponent(_countFileImportButton)
                        .addGap(16, 16, 16)))
                .addGap(18, 18, 18)
                .addComponent(_resultsFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_outputFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_resultsFileBrowseButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportResultsAsHTML)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 22, Short.MAX_VALUE)
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_inputTab, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_inputTab, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel _caseFileLabel;
    private javax.swing.JCheckBox _conditionalAnalysis;
    private javax.swing.JLabel _controlFileLabel;
    private javax.swing.JButton _countFileBrowseButton;
    private javax.swing.JButton _countFileImportButton;
    private javax.swing.JTextField _countFileTextField;
    private javax.swing.JCheckBox _duplicatesInCaseFile;
    private javax.swing.JPanel _inputTab;
    private javax.swing.JLabel _labelMonteCarloReplications;
    private javax.swing.JTextField _montCarloReplicationsTextField;
    private javax.swing.JTextField _outputFileTextField;
    private javax.swing.JCheckBox _reportResultsAsHTML;
    private javax.swing.JButton _resultsFileBrowseButton;
    private javax.swing.JLabel _resultsFileLabel;
    private javax.swing.JButton _treeFileBrowseButton;
    private javax.swing.JButton _treeFileImportButton;
    private javax.swing.JTextField _treelFileTextField;
    private javax.swing.JPanel jPanel1;
    // End of variables declaration//GEN-END:variables
    @Override
    public void internalFrameOpened(InternalFrameEvent e) {}
    @Override
    public void internalFrameClosing(InternalFrameEvent e) {
        if ((gbPromptOnExist ? (QueryWindowCanClose() ? true : false) : true) == true) {
            ParameterHistory.getInstance().AddParameterToHistory(_parameters.getSourceFileName());
            dispose();
        }
    }
    @Override
    public void internalFrameClosed(InternalFrameEvent e) {}
    public void internalFrameIconified(InternalFrameEvent e) {}
    public void internalFrameDeiconified(InternalFrameEvent e) {}
    public void internalFrameActivated(InternalFrameEvent e) {}
    public void internalFrameDeactivated(InternalFrameEvent e) {}
    /* Exception class that notes the Component that caused the exceptional situation. */
    public class SettingsException extends RuntimeException {
        private static final long serialVersionUID = 1L;
        public final Component focusComponent;
        public SettingsException(Component focusComponent) {
            super();
            this.focusComponent = focusComponent;
        }
        public SettingsException(String arg0, Component focusComponent) {
            super(arg0);
            this.focusComponent = focusComponent;
        }
        public SettingsException(String arg0, Throwable arg1, Component focusComponent) {
            super(arg0, arg1);
            this.focusComponent = focusComponent;
        }
        public SettingsException(Throwable arg0, Component focusComponent) {
            super(arg0);
            this.focusComponent = focusComponent;
        }
        /** recursively searches Container objects contained in 'rootComponent' for for 'searchComponent'. */
        boolean isContainedComponent(Component rootComponent, Component searchComponent) {
            if (rootComponent == searchComponent) {
                return true;
            }
            try {
                if (Class.forName("java.awt.Container").isInstance(rootComponent)) {
                    Container rootContainer = (Container) rootComponent;
                    for (int j = 0; j < rootContainer.getComponentCount(); ++j) {
                        if (isContainedComponent(rootContainer.getComponent(j), searchComponent)) {
                            return true;
                        }
                    }
                }
            } catch (ClassNotFoundException e) {}
            return false;
        }
        public void setControlFocus() {
            focusComponent.requestFocus();
        }
    }
}
