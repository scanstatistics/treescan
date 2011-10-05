/*
 * Utils.java
 *
 * Created on December 12, 2007, 9:14 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.gui.utils;

import javax.swing.JTextField;

/**
 * Utilities class for graphical controls.
 * @author Hostovic
 */
public class Utils {

    /**
     * Validates that key typed is a positive integer or back space; otherwise consumes key.
     */
    public static void validatePostiveNumericKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!Character.isDigit(e.getKeyChar()) && e.getKeyCode() != java.awt.event.KeyEvent.VK_BACK_SPACE) {
            e.consume();
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }

        /**
     * Validates that key typed is an integer or back space; otherwise consumes key.
     */
    public static void validateNumericKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!(Character.isDigit(e.getKeyChar()) || e.getKeyChar() == java.awt.event.KeyEvent.VK_BACK_SPACE || e.getKeyChar() == '-')) {
            e.consume();
            return;
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }

    /**
     * Validates that key typed is a positive integer, period or back space; otherwise consumes key.
     */
    public static void validatePostiveFloatKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!(Character.isDigit(e.getKeyChar()) || e.getKeyChar() == '\b' || e.getKeyChar() == '.')) {
            e.consume();
            return;
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }

    /**
     * Validates that key typed is a positive integer, period, minus or back space; otherwise consumes key.
     */
    public static void validateFloatKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!(Character.isDigit(e.getKeyChar()) || e.getKeyChar() == '\b' || e.getKeyChar() == '-' || e.getKeyChar() == '.')) {
            e.consume();
            return;
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }    

}
