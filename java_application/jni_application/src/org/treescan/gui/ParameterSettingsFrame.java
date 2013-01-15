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
        fc.setDialogTitle("Save Settings As");
        fc.addChoosableFileFilter(new InputFileFilter("tml", "Settings Files (*.tml)"));
        int returnVal = fc.showSaveDialog(this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
            String filename = fc.getSelectedFile().getAbsolutePath();
            if (fc.getSelectedFile().getName().indexOf('.') == -1){
                filename = filename + ".tml";
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
     * checks 'Input Files' tab
     */
    private void CheckInputSettings() {
        //validate the tree file
        if (_treelFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a tree file.", (Component) _treelFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_treelFileTextField.getText(), false)) {
            throw new SettingsException("The tree file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _treelFileTextField);
        }

        //validate the cuts file
        if (_cutFileTextField.getText().length() > 0 && !FileAccess.ValidateFileAccess(_cutFileTextField.getText(), false)) {
            throw new SettingsException("The cuts file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.",  (Component) _cutFileTextField);
        }

        //validate the case file
        if (_countFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a case file.", (Component) _countFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_countFileTextField.getText(), false)) {
            throw new SettingsException("The count file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.",  (Component) _countFileTextField);
        }
    }

    /**
     * checks 'Analysis' tab
     */
    private void CheckAnalysisSettings() {
        if (_BernoulliButton.isSelected()) {
            int eventProbNumerator = Integer.parseInt(_eventProbabiltyNumerator.getText().trim());
            int eventProbDenominator = Integer.parseInt(_eventProbabiltyDenominator.getText().trim());
            if (eventProbNumerator == 0 || eventProbDenominator == 0 || eventProbNumerator >= eventProbDenominator) {
                throw new SettingsException("Please specify an event probabilty that is between zero and one.", (Component) _eventProbabiltyNumerator);
            }
        }
        int dNumReplications;
        if (_montCarloReplicationsTextField.getText().trim().length() == 0) {
            throw new SettingsException("Please specify a number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.",(Component)_montCarloReplicationsTextField);
        }
        dNumReplications = Integer.parseInt(_montCarloReplicationsTextField.getText().trim());
        if (!((dNumReplications == 0 || dNumReplications == 9 || dNumReplications == 19 || (dNumReplications + 1) % 1000 == 0))) {
            throw new SettingsException("Invalid number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.",(Component) _montCarloReplicationsTextField);
        }
    }

    /**
     * checks 'Output' tab
     */
    private void CheckOutputSettings() {
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

    public boolean CheckSettings() {
        try {
            CheckInputSettings();
            CheckAnalysisSettings();
            CheckOutputSettings();
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
        _cutFileTextField.setText(parameters.getCutsFileName());
        _cutFileTextField.setCaretPosition(0);
        _countFileTextField.setText(parameters.getCountFileName());
        _countFileTextField.setCaretPosition(0);
        _outputFileTextField.setText(parameters.getOutputFileName());
        _outputFileTextField.setCaretPosition(0);
        _reportResultsAsHTML.setSelected(parameters.getResultsFormat() == Parameters.ResultsFormat.HTML);
        _montCarloReplicationsTextField.setText(Integer.toString(parameters.getNumReplicationsRequested()));
        _PoissonButton.setSelected(parameters.getModelType() == Parameters.ModelType.POISSON);
        _BernoulliButton.setSelected(parameters.getModelType() == Parameters.ModelType.BERNOULLI);
        _unconditionalButton.setSelected(!parameters.isConditional());
        _conditionalButton.setSelected(parameters.isConditional());
        _eventProbabiltyNumerator.setText(Integer.toString(parameters.getProbabilityRatioNumerator()));
        _eventProbabiltyDenominator.setText(Integer.toString(parameters.getProbabilityRatioDenominator()));
    }
    /** sets CParameters class with settings in form */
    private void saveParameterSettings(Parameters parameters) {
        setTitle(parameters.getSourceFileName());
        parameters.setTreeFileName(_treelFileTextField.getText());
        parameters.setCutsFileName(_cutFileTextField.getText());
        parameters.setCountFileName(_countFileTextField.getText());
        parameters.setOutputFileName(_outputFileTextField.getText());
        parameters.setResultsFormat(_reportResultsAsHTML.isSelected() ? Parameters.ResultsFormat.HTML.ordinal() : Parameters.ResultsFormat.TEXT.ordinal());
        parameters.setNumReplications(Integer.parseInt(_montCarloReplicationsTextField.getText()));
        if (_PoissonButton.isSelected()) {
          parameters.setModelType(Parameters.ModelType.POISSON.ordinal());
        } else if (_BernoulliButton.isSelected()) {
          parameters.setModelType(Parameters.ModelType.BERNOULLI.ordinal());
        }
        parameters.setConditional(_conditionalButton.isSelected() ? true : false);
        parameters.setProbabilityRatioNumerator(Integer.parseInt(_eventProbabiltyNumerator.getText()));
        parameters.setProbabilityRatioDenominator(Integer.parseInt(_eventProbabiltyDenominator.getText()));
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
                case Cuts:
                    _cutFileTextField.setText(wizard.getDestinationFilename());
                    break;
                case Counts:
                    _countFileTextField.setText(wizard.getDestinationFilename());
                    break;
                default:
                    throw new UnknownEnumException(eFileType);
            }
        }
    }

    private void enableSettingsForStatisticModelCombination() {
        boolean enabled = _BernoulliButton.isSelected() && _unconditionalButton.isSelected();
        _eventProbabilityLabel.setEnabled(enabled);
        _eventProbabilityLabel2.setEnabled(enabled);
        _eventProbabiltyNumerator.setEnabled(enabled);
        _eventProbabiltyDenominator.setEnabled(enabled);
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        modelButtonGroup = new javax.swing.ButtonGroup();
        scanStatisticButtonGroup = new javax.swing.ButtonGroup();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        _inputTab = new javax.swing.JPanel();
        _cutFileLabel = new javax.swing.JLabel();
        _cutFileTextField = new javax.swing.JTextField();
        _cutFileBrowseButton = new javax.swing.JButton();
        _cutFileImportButton = new javax.swing.JButton();
        _treelFileTextField = new javax.swing.JTextField();
        _controlFileLabel = new javax.swing.JLabel();
        _treeFileBrowseButton = new javax.swing.JButton();
        _treeFileImportButton = new javax.swing.JButton();
        _countFileLabel = new javax.swing.JLabel();
        _countFileTextField = new javax.swing.JTextField();
        _countFileBrowseButton = new javax.swing.JButton();
        _countFileImportButton = new javax.swing.JButton();
        _analysisTab = new javax.swing.JPanel();
        _probabilityModelPanel = new javax.swing.JPanel();
        _PoissonButton = new javax.swing.JRadioButton();
        _BernoulliButton = new javax.swing.JRadioButton();
        _eventProbabiltyDenominator = new javax.swing.JTextField();
        _eventProbabiltyNumerator = new javax.swing.JTextField();
        _eventProbabilityLabel = new javax.swing.JLabel();
        _eventProbabilityLabel2 = new javax.swing.JLabel();
        _scanStatisticPanel = new javax.swing.JPanel();
        _conditionalButton = new javax.swing.JRadioButton();
        _unconditionalButton = new javax.swing.JRadioButton();
        jPanel1 = new javax.swing.JPanel();
        _labelMonteCarloReplications = new javax.swing.JLabel();
        _montCarloReplicationsTextField = new javax.swing.JTextField();
        _outputTab = new javax.swing.JPanel();
        _reportResultsAsHTML = new javax.swing.JCheckBox();
        _outputFileTextField = new javax.swing.JTextField();
        _resultsFileLabel = new javax.swing.JLabel();
        _resultsFileBrowseButton = new javax.swing.JButton();

        modelButtonGroup.add(_PoissonButton);
        modelButtonGroup.add(_BernoulliButton);

        scanStatisticButtonGroup.add(_conditionalButton);
        scanStatisticButtonGroup.add(_unconditionalButton);

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);

        _cutFileLabel.setText("Cut File (optional):"); // NOI18N

        _cutFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
        _cutFileBrowseButton.setToolTipText("Browse for cut file ..."); // NOI18N
        _cutFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Cut File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("cut","Cut Files (*.cut)"));
                int returnVal = fc.showOpenDialog(ParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _cutFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _cutFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _cutFileImportButton.setToolTipText("Import cut file ..."); // NOI18N
        _cutFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    JFileChooser fc = new JFileChooser(org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                    fc.setDialogTitle("Select Cuts File Import Source");
                    fc.addChoosableFileFilter(new InputFileFilter("dbf","dBase Files (*.dbf)"));
                    fc.addChoosableFileFilter(new InputFileFilter("csv","Delimited Files (*.csv)"));
                    fc.addChoosableFileFilter(new InputFileFilter("xls","Excel Files (*.xls)"));
                    fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                    fc.addChoosableFileFilter(new InputFileFilter("cut","Cut Files (*.cut)"));
                    int returnVal = fc.showOpenDialog(ParameterSettingsFrame.this);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                        LaunchImporter(fc.getSelectedFile().getAbsolutePath(), FileImporter.InputFileType.Cuts);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(org.treescan.gui.TreeScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        _controlFileLabel.setText("Tree File:"); // NOI18N

        _treeFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
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

        _treeFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
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

        _countFileLabel.setText("Count File:"); // NOI18N

        _countFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
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

        _countFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
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

        javax.swing.GroupLayout _inputTabLayout = new javax.swing.GroupLayout(_inputTab);
        _inputTab.setLayout(_inputTabLayout);
        _inputTabLayout.setHorizontalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_controlFileLabel)
                    .addComponent(_cutFileLabel)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                        .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(_cutFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 422, Short.MAX_VALUE)
                            .addComponent(_treelFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 422, Short.MAX_VALUE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                                .addComponent(_treeFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_treeFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                                .addComponent(_cutFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_cutFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addComponent(_countFileLabel)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                        .addComponent(_countFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 422, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_countFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_countFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        _inputTabLayout.setVerticalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_controlFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addComponent(_treelFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_cutFileLabel))
                    .addComponent(_treeFileImportButton)
                    .addComponent(_treeFileBrowseButton))
                .addGap(9, 9, 9)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_cutFileBrowseButton)
                    .addComponent(_cutFileImportButton)
                    .addComponent(_cutFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_countFileLabel)
                .addGap(9, 9, 9)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_countFileBrowseButton)
                    .addComponent(_countFileImportButton)
                    .addComponent(_countFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(56, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Input", _inputTab);

        _probabilityModelPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Probability Model"));

        _PoissonButton.setSelected(true);
        _PoissonButton.setText("Poisson");
        _PoissonButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });
        _PoissonButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                _PoissonButtonActionPerformed(evt);
            }
        });

        _BernoulliButton.setText("Bernoulli");
        _BernoulliButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        _eventProbabiltyDenominator.setText("2");
        _eventProbabiltyDenominator.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_eventProbabiltyDenominator.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _eventProbabiltyDenominator.setText("2");
            }
        });
        _eventProbabiltyDenominator.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_eventProbabiltyDenominator, e, 6);
            }
        });
        _eventProbabiltyDenominator.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _eventProbabiltyNumerator.setText("1");
        _eventProbabiltyNumerator.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_eventProbabiltyNumerator.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _eventProbabiltyNumerator.setText("1");
            }
        });
        _eventProbabiltyNumerator.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_eventProbabiltyNumerator, e, 6);
            }
        });
        _eventProbabiltyNumerator.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _eventProbabilityLabel.setText("Event Probability:");

        _eventProbabilityLabel2.setText("/");

        javax.swing.GroupLayout _probabilityModelPanelLayout = new javax.swing.GroupLayout(_probabilityModelPanel);
        _probabilityModelPanel.setLayout(_probabilityModelPanelLayout);
        _probabilityModelPanelLayout.setHorizontalGroup(
            _probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                        .addComponent(_PoissonButton)
                        .addContainerGap(264, Short.MAX_VALUE))
                    .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                        .addComponent(_BernoulliButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 45, Short.MAX_VALUE)
                        .addComponent(_eventProbabilityLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabiltyNumerator, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabilityLabel2)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabiltyDenominator, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(34, 34, 34))))
        );
        _probabilityModelPanelLayout.setVerticalGroup(
            _probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_PoissonButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 18, Short.MAX_VALUE)
                .addGroup(_probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_BernoulliButton)
                    .addComponent(_eventProbabilityLabel)
                    .addComponent(_eventProbabiltyNumerator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_eventProbabilityLabel2)
                    .addComponent(_eventProbabiltyDenominator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );

        _scanStatisticPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Type of Scan Statistic"));

        _conditionalButton.setSelected(true);
        _conditionalButton.setText("Conditional");
        _conditionalButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        _unconditionalButton.setText("Unconditional");
        _unconditionalButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        javax.swing.GroupLayout _scanStatisticPanelLayout = new javax.swing.GroupLayout(_scanStatisticPanel);
        _scanStatisticPanel.setLayout(_scanStatisticPanelLayout);
        _scanStatisticPanelLayout.setHorizontalGroup(
            _scanStatisticPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_scanStatisticPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_scanStatisticPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_conditionalButton)
                    .addComponent(_unconditionalButton))
                .addContainerGap(28, Short.MAX_VALUE))
        );
        _scanStatisticPanelLayout.setVerticalGroup(
            _scanStatisticPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _scanStatisticPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_unconditionalButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 20, Short.MAX_VALUE)
                .addComponent(_conditionalButton)
                .addContainerGap())
        );

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Monte Carlo Replications"));

        _labelMonteCarloReplications.setText("Number of replications (0, 9, 999, or value ending in 999):"); // NOI18N

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

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_labelMonteCarloReplications)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(121, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_labelMonteCarloReplications)
                    .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _analysisTabLayout = new javax.swing.GroupLayout(_analysisTab);
        _analysisTab.setLayout(_analysisTabLayout);
        _analysisTabLayout.setHorizontalGroup(
            _analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_analysisTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_analysisTabLayout.createSequentialGroup()
                        .addComponent(_scanStatisticPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_probabilityModelPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .addContainerGap())
        );
        _analysisTabLayout.setVerticalGroup(
            _analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_analysisTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_scanStatisticPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_probabilityModelPanel, 0, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addGap(10, 10, 10)
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(31, 31, 31))
        );

        jTabbedPane1.addTab("Analysis", _analysisTab);

        _reportResultsAsHTML.setText("Report Results as HTML");

        _resultsFileLabel.setText("Results File:"); // NOI18N

        _resultsFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
        _resultsFileBrowseButton.setToolTipText("Browse for results file ...");
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

        javax.swing.GroupLayout _outputTabLayout = new javax.swing.GroupLayout(_outputTab);
        _outputTab.setLayout(_outputTabLayout);
        _outputTabLayout.setHorizontalGroup(
            _outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_outputTabLayout.createSequentialGroup()
                        .addGroup(_outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_outputTabLayout.createSequentialGroup()
                                .addGap(10, 10, 10)
                                .addComponent(_reportResultsAsHTML))
                            .addComponent(_outputFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 453, Short.MAX_VALUE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_resultsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_resultsFileLabel))
                .addContainerGap())
        );
        _outputTabLayout.setVerticalGroup(
            _outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_resultsFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_resultsFileBrowseButton)
                    .addGroup(_outputTabLayout.createSequentialGroup()
                        .addComponent(_outputFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_reportResultsAsHTML)))
                .addContainerGap(139, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Output", _outputTab);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 509, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 243, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void _PoissonButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event__PoissonButtonActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event__PoissonButtonActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JRadioButton _BernoulliButton;
    private javax.swing.JRadioButton _PoissonButton;
    private javax.swing.JPanel _analysisTab;
    private javax.swing.JRadioButton _conditionalButton;
    private javax.swing.JLabel _controlFileLabel;
    private javax.swing.JButton _countFileBrowseButton;
    private javax.swing.JButton _countFileImportButton;
    private javax.swing.JLabel _countFileLabel;
    private javax.swing.JTextField _countFileTextField;
    private javax.swing.JButton _cutFileBrowseButton;
    private javax.swing.JButton _cutFileImportButton;
    private javax.swing.JLabel _cutFileLabel;
    private javax.swing.JTextField _cutFileTextField;
    private javax.swing.JLabel _eventProbabilityLabel;
    private javax.swing.JLabel _eventProbabilityLabel2;
    private javax.swing.JTextField _eventProbabiltyDenominator;
    private javax.swing.JTextField _eventProbabiltyNumerator;
    private javax.swing.JPanel _inputTab;
    private javax.swing.JLabel _labelMonteCarloReplications;
    private javax.swing.JTextField _montCarloReplicationsTextField;
    private javax.swing.JTextField _outputFileTextField;
    private javax.swing.JPanel _outputTab;
    private javax.swing.JPanel _probabilityModelPanel;
    private javax.swing.JCheckBox _reportResultsAsHTML;
    private javax.swing.JButton _resultsFileBrowseButton;
    private javax.swing.JLabel _resultsFileLabel;
    private javax.swing.JPanel _scanStatisticPanel;
    private javax.swing.JButton _treeFileBrowseButton;
    private javax.swing.JButton _treeFileImportButton;
    private javax.swing.JTextField _treelFileTextField;
    private javax.swing.JRadioButton _unconditionalButton;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.ButtonGroup modelButtonGroup;
    private javax.swing.ButtonGroup scanStatisticButtonGroup;
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
