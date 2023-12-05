package org.treescan.gui;

import java.awt.Component;
import java.awt.Container;
import java.beans.PropertyVetoException;
import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import javax.swing.JRootPane;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import javax.swing.undo.UndoManager;
import org.treescan.app.ParameterHistory;
import org.treescan.app.Parameters;
import org.treescan.app.UnknownEnumException;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.importer.CSVImportDataSource;
import org.treescan.importer.ImportDataSource;
import org.treescan.importer.InputSourceSettings;
import org.treescan.importer.XLSImportDataSource;
import org.treescan.utils.FileAccess;

/**
 *
 * @author hostovic
 */
public class AbstractParameterSettingsFrame extends javax.swing.JInternalFrame implements InternalFrameListener {
    protected Parameters _parameters = null;
    protected Parameters _initialParameters = new Parameters();
    protected boolean gbPromptOnExist = true;
    protected final UndoManager undo = new UndoManager();
    protected final JRootPane _rootPane;
    protected Map _input_source_map = new HashMap();

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AbstractParameterSettingsFrame(final JRootPane rootPane, Parameters parameters) {
        initFrameComponents();
        setFrameIcon(new ImageIcon(getClass().getResource("/TreeScan.png")));
        _rootPane = rootPane;
        addInternalFrameListener(this);
        _parameters = parameters;

        setupInterface(_parameters);
        // Save orginal parameter settings to compare against when window closes but
        // first save what the interface has produced for the settings read from file.
        saveParameterSettings(_parameters);
        _initialParameters = (Parameters) _parameters.clone();
    }

    /** If necessary, removes from iconized state and brings to front. */
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

    public void showExecOptionsDialog(java.awt.Frame parent) {
        new ExecutionOptionsDialog(parent, _parameters).setVisible(true);
    }

    public boolean CheckSettings() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    protected void setupInterface(final Parameters parameters) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public Parameters.ScanType getScanType() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public Parameters.ConditionalType getConditionalType() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public Parameters.ModelType getModelType() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    /**
     * Determines whether window can be closed by comparing parameter settings
     * contained in window versus initial parameter settings.
     */
    public boolean queryWindowCanClose() {
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

    protected void saveParameterSettings(Parameters parameters) {
        throw new UnsupportedOperationException("Not supported yet.");
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
     * Launches 'save as' dialog to permit user saving current settings to
     * parameter file
     */
    public boolean SaveAs() {
        boolean bSaved = true;
        List<InputFileFilter> filters = new ArrayList<InputFileFilter>();
        filters.add(new InputFileFilter("prm", "Settings Files (*.prm)"));
        FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Parameters File", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
        File file = select.browse_saveas();
        if (file != null) {
            org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
            String filename = file.getAbsolutePath();
            if (new File(filename).getName().lastIndexOf('.') == -1){
                filename = filename + ".prm";
            }
            WriteSession(filename);
            setTitle(filename);
        } else {
            bSaved = false;
        }
        return bSaved;
    }

    public final Parameters getParameterSettings() {
        saveParameterSettings(_parameters);
        return _parameters;
    }

    /** Return the ImportDataSource object -- based upon the source file type. */
    public int getNumImportSourceColumns(InputSourceSettings iss, String filename) {
        try {
            ImportDataSource source=null;
            switch (iss.getSourceDataFileType()) {
                case Excel97_2003 :
                case Excel : source = new XLSImportDataSource(new File(filename), false); break;
                case CSV :
                default : source = new CSVImportDataSource(new File(filename), iss.getFirstRowHeader(), '\n', iss.getDelimiter().charAt(0), iss.getGroup().charAt(0), iss.getSkiplines());
            }
            int num_columns = source.getColumnNames().length;
            source.close();
            return num_columns;
        } catch (Exception e) {}
        return 0;
    }

    /* Validates the source data file against restrictions on source and InputSourceSettings settings. */
    public String validateInputSourceDataFile(String filepath, String mapKey, String verbosename) {
        // First exclude file types that are not readable - namely, Excel97_2003;
        String extension = FileAccess.getExtension(new File(filepath));
        extension = extension == null ? "" : extension.toLowerCase();
        if (extension.equals(".xls") || extension.equals(".xlsx")) {
            return "Excel files (.xls and  xlsx extensions) can only be read directly by TreeScan.\nYou must import this " + verbosename + " file.";
        }
        boolean iss_exists = _input_source_map.containsKey(mapKey);
        if (iss_exists) {
            InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(mapKey);
            // Verify that the input source settings's source data file type matches extension.
            boolean correct_filetype=true;
            InputSourceSettings.SourceDataFileType extensionType=FileSourceWizard.getSourceFileType(filepath);
            switch (inputSourceSettings.getSourceDataFileType()) {
                case CSV : correct_filetype = !(extension.equals(".xls") || extension.equals(".xlsx")); break;
                case Excel97_2003 :
                case Excel :  correct_filetype = extension.equals(".xls") || extension.equals(".xlsx"); break;
                default:    throw new UnknownEnumException(inputSourceSettings.getSourceDataFileType());
            }
            if (!correct_filetype) {
                return "The import feature must be performed again on the " + verbosename + " file.\nThe current import settings indicate a " + inputSourceSettings.getSourceDataFileType().toString() + " file but the specified file is a " + extensionType.toString() + " file.";
            }
            // Verify that the mappings align with the data source available options.
            // Safely get the number of columns in datasource, if mapping references column index greater than # columns, then display error.
            if (inputSourceSettings.isSet()) {
                int num_cols = getNumImportSourceColumns(inputSourceSettings, filepath);
                int max = 0;
                for (String stdIdx : inputSourceSettings.getFieldMaps()) {
                    if (!stdIdx.isEmpty()) {
                        max = Math.max(Integer.parseInt(stdIdx), max);
                    }
                }
                if (max > num_cols) {
                    return "The import feature must be performed again on the " + verbosename + " file.\nThe current import settings conflict with the file structure.";
                }
            }
        }
        return null;
    }


    protected void initFrameComponents() {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void internalFrameOpened(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void internalFrameClosing(InternalFrameEvent e) {
        if ((gbPromptOnExist ? queryWindowCanClose() : true) == true) {
            ParameterHistory.getInstance().AddParameterToHistory(_parameters.getSourceFileName());
            dispose();
        }
    }

    @Override
    public void internalFrameClosed(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void internalFrameIconified(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void internalFrameDeiconified(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void internalFrameActivated(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet.");
    }

    @Override
    public void internalFrameDeactivated(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet.");
    }

    /** Exception class that notes the Component that caused the exceptional situation. */
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

        /**
         * Recursively searches Container objects contained in 'rootComponent'
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

        public void setControlFocus() {
            focusComponent.requestFocus();
        }
    }

}
