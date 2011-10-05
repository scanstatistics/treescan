/*
 * UnknownEnumException.java
 *
 * Created on December 14, 2007, 10:40 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.app;

/**
 *
 * @author Hostovic
 */
public class UnknownEnumException extends RuntimeException {

    /**
     * Creates a new instance of UnknownEnumException
     */
    public UnknownEnumException(Enum e) {
        super("Unknown enumeration '" + e.toString() + "' for '" + e.getClass().getName() + "' ");
    }
}
