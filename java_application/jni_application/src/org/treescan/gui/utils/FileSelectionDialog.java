/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.treescan.gui.utils;

import java.awt.Component;
import java.awt.FileDialog;
import java.awt.Frame;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JFileChooser;
import javax.swing.JTextField;
import org.treescan.app.UnknownEnumException;
import org.treescan.gui.AbstractParameterSettingsFrame;
import org.treescan.gui.FileSourceWizard;
import org.treescan.gui.ParameterSettingsFrame;
import org.treescan.gui.TreeScanApplication;
import org.treescan.importer.InputSourceSettings;

/**
 *
 * @author hostovic
 */
public class FileSelectionDialog {
    private JFileChooser _file_chooser=null;
    private FileDialog _file_dialog=null;
    private File _lastBrowseDirectory;
    private Component _parent;

    public FileSelectionDialog(final Component parent, final String title, final List<InputFileFilter> filters, final File lastBrowseDirectory) {
        setup(parent, title, filters, lastBrowseDirectory);
    }

    public FileSelectionDialog(final Component parent, final InputSourceSettings.InputFileType fileType, final File lastBrowseDirectory) {
        String browse_title;
        List<InputFileFilter> filters = new ArrayList<>();

        browse_title = "Select " + getFileTypeAsString(fileType) +" File";
        switch (fileType) {
            case Tree :
                filters = FileSourceWizard.getInputFilters();
                filters.add(new InputFileFilter("tre", "Tree Files (*.tre)"));
                break;
            case Counts :
                filters = FileSourceWizard.getInputFilters();
                filters.add(new InputFileFilter("cas", "Count Files (*.cas)"));
                break;
            case Controls :
                filters = FileSourceWizard.getInputFilters();
                filters.add(new InputFileFilter("ctl", "Control Files (*.ctl)"));
                break;
            case Cut :
                filters = FileSourceWizard.getInputFilters();
                filters.add(new InputFileFilter("cut", "Cut Files (*.cut)"));
                break;
            case Power_Evaluations :
                filters = FileSourceWizard.getInputFilters();
                filters.add(new InputFileFilter("ha", "Alternative Hypothesis Files (*.ha)"));
                break;
           default: throw new UnknownEnumException(fileType);
        }
        setup(parent, browse_title, filters, lastBrowseDirectory);
    }

    public FileSelectionDialog(final Component parent, final String title, final File lastBrowseDirectory) {
        setup(parent, title, null, lastBrowseDirectory);
    }

    /* Returns file type as text string. */
    public static String getFileTypeAsString(InputSourceSettings.InputFileType fileType) {
        switch (fileType) {
            case Tree : return "Tree";
            case Counts: return "Count";
            case Controls: return "Control";
            case Cut: return "Cut";
            case Power_Evaluations: return "Alternative Hypothesis";
            default: throw new UnknownEnumException(fileType);
        }
    }

    public void setup(final Component parent, final String title, final List<InputFileFilter> filters, final File lastBrowseDirectory) {
        _parent = parent;
        _lastBrowseDirectory = lastBrowseDirectory;
        
        if (TreeScanApplication.getAwtBrowse()) {
            if (System.getProperty("os.name").toLowerCase().startsWith("mac"))
                System.setProperty("apple.awt.fileDialogForDirectories", "false");
            _file_dialog = new FileDialog((Frame)parent, title);
            _file_dialog.setDirectory(lastBrowseDirectory.getAbsolutePath());
            if (filters != null) {
                _file_dialog.setFilenameFilter((File dir, String name) -> {
                    for (InputFileFilter f: filters) {
                        if (name.endsWith("." + f.getFilter())) {
                            return true;
                        }
                    }
                    return false;
                });
            }
        } else {
            _file_chooser = new JFileChooser(lastBrowseDirectory);
            _file_chooser.setDialogTitle(title);
            if (filters != null) {
                for (InputFileFilter f: filters)
                    _file_chooser.addChoosableFileFilter(f);
            } else {
                _file_chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
            }
        }
    }

    /*
     * Browses for the input source file ...
     */
    public void browse_inputsource(JTextField inputSourceFilename, InputSourceSettings inputSourceSettings, AbstractParameterSettingsFrame settingsFrame, boolean enable_display_variables) {
        String filename = null;

        // If the input source filename is blank, display the file browse dialog.
        if (inputSourceFilename.getText().isEmpty()) {
           File file = browse_load(true);
           if (file != null) {
             filename = file.getAbsolutePath();
             TreeScanApplication.getInstance().lastBrowseDirectory = getDirectory();
           }
        } else {
            filename = inputSourceFilename.getText();
        }

        // If we have a filename at this point, display the file source wizard.
        if (filename != null) {
            FileSourceWizard wizard = new FileSourceWizard(TreeScanApplication.getInstance(),
                                                           filename,
                                                           settingsFrame.getParameterSettings().getSourceFileName(),
                                                           inputSourceSettings,
                                                           settingsFrame.getModelType(),
                                                           settingsFrame.getScanType(),
                                                           settingsFrame.getConditionalType(),
                                                           enable_display_variables);
            wizard.setVisible(true);
            if (wizard.getExecutedImport()) {
                inputSourceSettings.reset();
                inputSourceFilename.setText(wizard.getDestinationFilename());
            } else {
                if (wizard.getNeedsImportSourceSave())
                    inputSourceSettings.copy(wizard.getInputSourceSettings());
                inputSourceFilename.setText(wizard.getSourceFilename());
            }
            if (wizard.needsSettingsRefresh() && settingsFrame instanceof ParameterSettingsFrame) {
                ((ParameterSettingsFrame)settingsFrame).setControlsForAnalysisOptions(wizard.getScanType(), wizard.getConditionalType(), wizard.getModelType());
            }
        }
    }

    public File browse_load(boolean require_exits) {
        File file = null;
        if (_file_dialog != null) {
            _file_dialog.setVisible(true);
            _file_dialog.setMode(FileDialog.LOAD);
            if (_file_dialog.getFile() != null) {
                _lastBrowseDirectory = new File(_file_dialog.getDirectory());
                file = new File(_file_dialog.getDirectory() +  _file_dialog.getFile());
            }
        } else {
            int returnVal = _file_chooser.showOpenDialog(_parent);
            if (returnVal == JFileChooser.APPROVE_OPTION) {
                _lastBrowseDirectory = _file_chooser.getCurrentDirectory();
                file = new File(_file_chooser.getSelectedFile().getAbsolutePath());
            }
        }
        return file == null ? null : (require_exits && !file.exists() ? null : file);
    }

    public File browse_saveas() {
        File file = null;
        if (_file_dialog != null) {
            _file_dialog.setMode(FileDialog.SAVE);
            _file_dialog.setVisible(true);
            if (_file_dialog.getFile() != null) {
                _lastBrowseDirectory = new File(_file_dialog.getDirectory());
                file = new File(_file_dialog.getDirectory() +  _file_dialog.getFile());
            }
        } else {
            int returnVal = _file_chooser.showSaveDialog(_parent);
            if (returnVal == JFileChooser.APPROVE_OPTION) {
                _lastBrowseDirectory = _file_chooser.getCurrentDirectory();
                file = new File(_file_chooser.getSelectedFile().getAbsolutePath());
            }
        }
        return file;
    }

    public File getDirectory() {
        return _lastBrowseDirectory;
    }
}
