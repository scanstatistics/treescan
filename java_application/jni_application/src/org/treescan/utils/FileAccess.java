/*
 * FileAccess.java
 *
 * Created on December 11, 2007, 3:57 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.treescan.utils;

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
}
