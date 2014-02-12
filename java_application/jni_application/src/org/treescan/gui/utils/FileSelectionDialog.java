/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.treescan.gui.utils;

import java.awt.Component;
import java.awt.FileDialog;
import java.awt.Frame;
import java.io.File;
import java.io.FilenameFilter;
import javax.swing.JFileChooser;

/**
 *
 * @author hostovic
 */
public class FileSelectionDialog {
    private JFileChooser _file_chooser=null;
    private FileDialog _file_dialog=null;
    private File _lastBrowseDirectory;
    private Component _parent;
    
    public FileSelectionDialog(final Component parent, final String title, final InputFileFilter[] filters, final File lastBrowseDirectory) {
        _lastBrowseDirectory = lastBrowseDirectory;
        if (System.getProperty("os.name").toLowerCase().startsWith("mac")) {
            System.setProperty("apple.awt.fileDialogForDirectories", "false");
            _file_dialog = new FileDialog((Frame)parent, title);
            _file_dialog.setDirectory(lastBrowseDirectory.getAbsolutePath());
            _file_dialog.setFilenameFilter(new FilenameFilter(){
                    @Override
                    public boolean accept(File dir, String name) {
                        for (int f=0; f < filters.length; f++) {                        
                            if (name.endsWith("." + filters[f].getFilter())) {
                                return true;
                            }
                        }
                        return false;
                    }
            });                            
        } else {
            _file_chooser = new JFileChooser(lastBrowseDirectory);
            _file_chooser.setDialogTitle(title);
            for (int f=0; f < filters.length; f++) {
                _file_chooser.addChoosableFileFilter(filters[f]);
            }
        }
    }
    
    public FileSelectionDialog(final Component parent, final String title, final File lastBrowseDirectory) {
        _lastBrowseDirectory = lastBrowseDirectory;
        if (System.getProperty("os.name").toLowerCase().startsWith("mac")) {
            System.setProperty("apple.awt.fileDialogForDirectories", "true");
            _file_dialog = new FileDialog((Frame)parent, title);
            _file_dialog.setDirectory(lastBrowseDirectory.getAbsolutePath());
        } else {
            _file_chooser = new JFileChooser(lastBrowseDirectory);
            _file_chooser.setDialogTitle(title);
            _file_chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
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
            int returnVal = _file_chooser.showOpenDialog(_parent);
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
