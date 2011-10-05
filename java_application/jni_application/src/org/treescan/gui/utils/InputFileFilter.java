package org.treescan.gui.utils;

import java.io.File;
import javax.swing.filechooser.FileFilter;

public class InputFileFilter extends FileFilter {

    public final String filter;
    public final String description;

    public InputFileFilter(String filter, String description) {
        super();
        this.filter = filter;
        this.description = description;
    }

    public boolean accept(File f) {
        if (f.isDirectory()) {
            return true;
        }
        String extension = getExtension(f);
        if (extension != null) {
            if (extension.equals(filter)) {
                return true;
            } else {
                return false;
            }
        }
        return false;
    }

    public String getDescription() {
        return description;
    }
    /*
     * Get the extension of a file.
     */

    public static String getExtension(File f) {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');

        if (i > 0 && i < s.length() - 1) {
            ext = s.substring(i + 1).toLowerCase();
        }
        return ext;
    }
}
