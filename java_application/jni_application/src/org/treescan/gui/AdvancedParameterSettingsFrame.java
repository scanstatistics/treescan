package org.treescan.gui;

import java.awt.Component;
import java.awt.Container;
import java.io.File;
import javax.swing.DefaultListModel;
import javax.swing.ImageIcon;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.event.MouseInputAdapter;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.treescan.app.AdvFeaturesExpection;
import org.treescan.utils.FileAccess;
import org.treescan.app.Parameters;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.gui.utils.Utils;
import org.treescan.importer.FileImporter;

/*
 * ParameterSettingsFrame.java
 *
 * Created on December 5, 2007, 11:07 AM
 */
/**
 *
 * @author Hostovic
 */
public class AdvancedParameterSettingsFrame extends javax.swing.JInternalFrame {
    public enum FocusedTabSet { INPUT, ANALYSIS, OUTPUT };
    private JPanel _glass = null;
    private final JRootPane _rootPane;
    private final Component _rootPaneInitialGlass;
    private final UndoManager undo = new UndoManager();
    private final ParameterSettingsFrame _analysisSettingsWindow;
    private DefaultListModel _dataSetsListModel = new DefaultListModel();
    private FocusedTabSet _focusedTabSet = FocusedTabSet.INPUT;

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AdvancedParameterSettingsFrame(final JRootPane rootPane, final ParameterSettingsFrame analysisSettingsWindow, final Parameters parameters) {
        initComponents();

        setFrameIcon(new ImageIcon(getClass().getResource("/TreeScan.png")));
        _rootPane = rootPane;
        _analysisSettingsWindow = analysisSettingsWindow;
        _rootPaneInitialGlass = rootPane.getGlassPane();
        // create opaque glass pane
        _glass = new JPanel();
        _glass.setOpaque(false);
        // Attach mouse listeners
        MouseInputAdapter adapter = new MouseInputAdapter() {
        };
        _glass.addMouseListener(adapter);
        _glass.addMouseMotionListener(adapter);
        // Add modal internal frame to glass pane
        _glass.add(this);
        setupInterface(parameters);
    }

    /**
     * recursively searches Container objects contained in 'rootComponent' for
     * for 'searchComponent'.
     */
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
        } catch (ClassNotFoundException e) {
        }
        return false;
    }

    /**
     * Sets tab set visible and attempts to set focus on 'focusComponent'.
     */
    public void setVisible(FocusedTabSet focusedTabSet, Component focusComponent) {
        //set tab set visible
        setVisible(true, focusedTabSet);
        //find focus component and request focus
        for (int i = 0; i < jTabbedPane1.getTabCount(); ++i) {
            if (isContainedComponent(jTabbedPane1.getComponentAt(i), focusComponent)) {
                jTabbedPane1.setSelectedIndex(i);
                focusComponent.requestFocus();
                return;
            }
        }
    }

    /**
     *
     */
    public void setVisible(boolean value, FocusedTabSet focusedTabSet) {
        if (value == false) {
            _closeButton.requestFocus();
        } //cause any text controls to loose focus
        super.setVisible(value);
        if (value) {
            startModal(focusedTabSet);
            enableSetDefaultsButton();
        } else {
            stopModal();
            _analysisSettingsWindow.enableAdvancedButtons();
        }
    }

    /**
     * enables 'Set Defaults' button
     */
    private void enableSetDefaultsButton() {
        // update enable/disable of Set Defaults button
        switch (_focusedTabSet) {
            case INPUT:
                _setDefaultButton.setEnabled(!getDefaultsSetForInputOptions());
                break;
            case ANALYSIS:
                _setDefaultButton.setEnabled(!getDefaultsSetForAnalysisOptions());
                break;
            case OUTPUT:
                _setDefaultButton.setEnabled(!getDefaultsSetForOutputOptions());
                break;
        }
    } 
    
    /**
     * Checks to determine if only default values are set in the dialog Returns
     * true if only default values are set Returns false if user specified a
     * value other than a default
     */
    public boolean getDefaultsSetForInputOptions() {
        boolean bReturn = true;
        bReturn &= (_cutFileTextField.getText().equals(""));
        return bReturn;
    }    
    
    public boolean getDefaultsSetForAnalysisOptions() {
        boolean bReturn = true;
        bReturn &= (Integer.parseInt(_montCarloReplicationsTextField.getText()) == 99999);
        return bReturn;
    }    
    
    public boolean getDefaultsSetForOutputOptions() {
        boolean bReturn = true;
        return bReturn;
    }    
    
    private synchronized void startModal(FocusedTabSet focusedTabSet) {
        if (_glass != null) {
            _rootPane.setGlassPane(_glass);
            _glass.setVisible(true); // Change glass pane to our panel
        }
        setFocusedTabSet(focusedTabSet);
    }

    private void setFocusedTabSet(FocusedTabSet focusedTabSet) {
        jTabbedPane1.removeAll();
        if (_focusedTabSet != null) {
            _focusedTabSet = focusedTabSet;
        }
        switch (_focusedTabSet) {
            case ANALYSIS:
                setTitle("Advanced Analysis Options");
                jTabbedPane1.addTab("Advanced Analysis", null, _advancedanalysisTab, null);
                break;
            case INPUT:
            default:
                setTitle("Advanced Input Options");
                jTabbedPane1.addTab("Advanced Input", null, _advancedinputTab, null);
        }
    }

    private synchronized void stopModal() {
        if (_glass != null) {
            _glass.setVisible(false);
            //reset root pane glass to original
            _rootPane.setGlassPane(_rootPaneInitialGlass);
        }
    }

    /**
     * sets CParameters class with settings in form
     */
    public void saveParameterSettings(Parameters parameters) {
        parameters.setCutsFileName(_cutFileTextField.getText());
        parameters.setNumReplications(Integer.parseInt(_montCarloReplicationsTextField.getText()));
    }

    /**
     * Resets advanced settings to default values
     */
    private void setDefaultsClick() {
        switch (_focusedTabSet) {
            case INPUT:
                setDefaultsForInputTab();
                break;
            case ANALYSIS:
                setDefaultsForAnalysisTabs();
                break;
            case OUTPUT:
                setDefaultsForOutputTab();
                break;
        }
        enableSetDefaultsButton();
    }

    /**
     * Sets default values for Input related tab and respective controls
     */
    private void setDefaultsForInputTab() {
        _cutFileTextField.setText("");
    }
    
    /**
     * Sets default values for Analysis related tabs and their respective
     * controls pulled these default values from the CParameter class
     */
    private void setDefaultsForAnalysisTabs() {
        _montCarloReplicationsTextField.setText("99999");
    }
    
    private void setupInterface(final Parameters parameters) {
        _cutFileTextField.setText(parameters.getCutsFileName());
        _cutFileTextField.setCaretPosition(0);      
        _montCarloReplicationsTextField.setText(Integer.toString(parameters.getNumReplicationsRequested()));        
    }

    /**
     * Sets default values for Output related tab and respective controls pulled
     * these default values from the CParameter class
     */
    private void setDefaultsForOutputTab() {
    }    
    
    /**
     * validates all the settings in this dialog
     */
    public void validateParameters() {
        validateInputSettings();
        validateInferenceSettings();
    }
    
    private void validateInputSettings() {
        //validate the cuts file
        if (_cutFileTextField.getText().length() > 0 && !FileAccess.ValidateFileAccess(_cutFileTextField.getText(), false)) {
            throw new AdvFeaturesExpection("The cuts file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", FocusedTabSet.INPUT, (Component) _cutFileTextField);
        }
    }   
    
    private void validateInferenceSettings() {
        int dNumReplications;
        if (_montCarloReplicationsTextField.getText().trim().length() == 0) {
            throw new AdvFeaturesExpection("Please specify a number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.", FocusedTabSet.ANALYSIS, (Component)_montCarloReplicationsTextField);
        }
        dNumReplications = Integer.parseInt(_montCarloReplicationsTextField.getText().trim());
        if (!((dNumReplications == 0 || dNumReplications == 9 || dNumReplications == 19 || (dNumReplications + 1) % 1000 == 0))) {
            throw new AdvFeaturesExpection("Invalid number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.", FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
        }
    }   

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jTabbedPane1 = new javax.swing.JTabbedPane();
        _advancedinputTab = new javax.swing.JPanel();
        _cutFileLabel = new javax.swing.JLabel();
        _cutFileTextField = new javax.swing.JTextField();
        _cutFileBrowseButton = new javax.swing.JButton();
        _cutFileImportButton = new javax.swing.JButton();
        _advancedanalysisTab = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        _labelMonteCarloReplications = new javax.swing.JLabel();
        _montCarloReplicationsTextField = new javax.swing.JTextField();
        _closeButton = new javax.swing.JButton();
        _setDefaultButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.HIDE_ON_CLOSE);

        _cutFileLabel.setText("Cut File:"); // NOI18N

        _cutFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
        _cutFileBrowseButton.setToolTipText("Browse for cut file ..."); // NOI18N
        _cutFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("txt","Text Files (*.txt)"), new InputFileFilter("cut","Cut Files (*.cut)")};
                FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Cut File", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_load(true);
                if (file != null) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    _cutFileTextField.setText(file.getAbsolutePath());
                }
            }
        });

        _cutFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _cutFileImportButton.setToolTipText("Import cut file ..."); // NOI18N
        _cutFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("dbf","dBase Files (*.dbf)"),
                        new InputFileFilter("csv","Delimited Files (*.csv)"),
                        new InputFileFilter("xls","Excel Files (*.xls)"),
                        new InputFileFilter("txt","Text Files (*.txt)"),
                        new InputFileFilter("cut","Cut Files (*.cut)")};

                    FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Cuts File Import Source", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                    File file = select.browse_load(true);
                    if (file != null) {
                        org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                        _analysisSettingsWindow.LaunchImporter(file.getAbsolutePath(), FileImporter.InputFileType.Cuts);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(org.treescan.gui.TreeScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        javax.swing.GroupLayout _advancedinputTabLayout = new javax.swing.GroupLayout(_advancedinputTab);
        _advancedinputTab.setLayout(_advancedinputTabLayout);
        _advancedinputTabLayout.setHorizontalGroup(
            _advancedinputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advancedinputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_advancedinputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_advancedinputTabLayout.createSequentialGroup()
                        .addComponent(_cutFileTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_cutFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_cutFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap())
                    .addGroup(_advancedinputTabLayout.createSequentialGroup()
                        .addComponent(_cutFileLabel)
                        .addGap(0, 552, Short.MAX_VALUE))))
        );
        _advancedinputTabLayout.setVerticalGroup(
            _advancedinputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advancedinputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_cutFileLabel)
                .addGap(9, 9, 9)
                .addGroup(_advancedinputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_cutFileBrowseButton)
                    .addComponent(_cutFileImportButton)
                    .addComponent(_cutFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(142, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Advanced Input Options", _advancedinputTab);

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Monte Carlo Replications"));

        _labelMonteCarloReplications.setText("Number of replications (0, 9, 999, or value ending in 999):"); // NOI18N

        _montCarloReplicationsTextField.setText("99999"); // NOI18N
        _montCarloReplicationsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_montCarloReplicationsTextField.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _montCarloReplicationsTextField.setText("99999");
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
                .addContainerGap(219, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_labelMonteCarloReplications)
                    .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _advancedanalysisTabLayout = new javax.swing.GroupLayout(_advancedanalysisTab);
        _advancedanalysisTab.setLayout(_advancedanalysisTabLayout);
        _advancedanalysisTabLayout.setHorizontalGroup(
            _advancedanalysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advancedanalysisTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _advancedanalysisTabLayout.setVerticalGroup(
            _advancedanalysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advancedanalysisTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(133, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Advanced Analysis Options", _advancedanalysisTab);

        _closeButton.setText("Close"); // NOI18N
        _closeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisible(false, null);
            }
        });

        _setDefaultButton.setText("Set Defaults"); // NOI18N
        _setDefaultButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setDefaultsClick();
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(_setDefaultButton, javax.swing.GroupLayout.PREFERRED_SIZE, 120, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_closeButton, javax.swing.GroupLayout.PREFERRED_SIZE, 105, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
            .addComponent(jTabbedPane1)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 229, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_setDefaultButton)
                    .addComponent(_closeButton))
                .addContainerGap())
        );

        jTabbedPane1.getAccessibleContext().setAccessibleName("Cuts");

        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _advancedanalysisTab;
    private javax.swing.JPanel _advancedinputTab;
    private javax.swing.JButton _closeButton;
    private javax.swing.JButton _cutFileBrowseButton;
    private javax.swing.JButton _cutFileImportButton;
    private javax.swing.JLabel _cutFileLabel;
    public javax.swing.JTextField _cutFileTextField;
    private javax.swing.JLabel _labelMonteCarloReplications;
    private javax.swing.JTextField _montCarloReplicationsTextField;
    private javax.swing.JButton _setDefaultButton;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    // End of variables declaration//GEN-END:variables
}
