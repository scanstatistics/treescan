/*
 * WaitCursor.java
 *
 * Created on September 18, 2007, 9:55 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.gui.utils;

import java.awt.Component;
import java.awt.Cursor;

/**
 * Sets components cursor to the wait cursor.
 * @author Hostovic
 */
public class WaitCursor {

    private Component _component;
    private Cursor _saveCursor;

    /** Creates a new instance of WaitCursor */
    public WaitCursor(Component component) {
        _component = component;
        set();
    }

    /** Creates a new instance of WaitCursor */
    public WaitCursor(Component component, boolean set) {
        _component = component;
        _saveCursor = _component.getCursor();
        if (set) {
            set();
        }
    }

    public void set() {
        Cursor saveCursor = _component.getCursor();
        _component.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
    }

    /**
     * Restores saved cursor to component.
     */
    public void restore() {
        _component.setCursor(_saveCursor);
    }
}
