/*
 * FileAccess.java
 *
 * Created on December 11, 2007, 3:57 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.treescan.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

/**
 *
 * @author Hostovic
 */
public class FileAccess {
    /**
     *
     */
    public static boolean ValidateFileAccess(String filename, boolean bWrite) {
        boolean bAccessible=false;
        
        try {
            if (bWrite) {
                @SuppressWarnings("unused") FileOutputStream file = new FileOutputStream(filename);
            } else {
                @SuppressWarnings("unused") FileInputStream file = new FileInputStream(filename);
            }
            bAccessible = true;
        } catch (FileNotFoundException e) {} catch (SecurityException e) {}
        
        return bAccessible;
    }   
    
    /* Get the extension of a file. */
    public static String getExtension(File f) {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');
        if (i > 0 && i < s.length() - 1) {
            ext = s.substring(i + 1).toLowerCase();
        }
        return ext;
    }    
    
    public static String changeExtension(String originalName, String newExtension) {
        int lastDot = originalName.lastIndexOf(".");
        if (lastDot != -1) {
            return originalName.substring(0, lastDot) + newExtension;
        } else {
            return originalName + newExtension;
        }
    }//end changeExtension
}
