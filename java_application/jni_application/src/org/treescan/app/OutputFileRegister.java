/*
 * OutputFileRegister.java
 *
 * Created on December 10, 2007, 2:54 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.app;

import java.util.Vector;

/**
 *
 * @author Hostovic
 */
public class OutputFileRegister {

    private Vector<String> _outputFileNames = new Vector<String>();
    private static final OutputFileRegister _instance = new OutputFileRegister();

    /** Creates a new instance of OutputFileRegister */
    public OutputFileRegister() {
    }

    public static OutputFileRegister getInstance() {
        return _instance;
    }

    public boolean isRegistered(String fileName) {
        return _outputFileNames.contains(fileName);
    }

    public void register(String fileName) {
        if (!isRegistered(fileName)) {
            _outputFileNames.add(fileName);
        }
    }

    public void release(String fileName) {
        if (isRegistered(fileName)) {
            _outputFileNames.remove(fileName);
        }
    }
}
