/*
 * ModalInternalFrame.java
 *
 * Created on December 10, 2007, 10:23 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.gui;

import java.awt.Component;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.event.MouseInputAdapter;

/**
 *
 * @author Hostovic
 */
public class ModalInternalFrame extends javax.swing.JInternalFrame {

    protected JPanel _glassPanel = null;
    protected final JRootPane _rootPane;
    protected final Component _rootPaneInitialGlass;

    public ModalInternalFrame(final JRootPane rootPane) {
        super();
        _rootPane = rootPane;
        _rootPaneInitialGlass = _rootPane.getGlassPane();
        // create opaque glass pane
        _glassPanel = new JPanel();
        _glassPanel.setOpaque(false);
        // Attach mouse listeners
        MouseInputAdapter adapter = new MouseInputAdapter() {
        };
        _glassPanel.addMouseListener(adapter);
        _glassPanel.addMouseMotionListener(adapter);
        // Add modal internal frame to glass pane
        _glassPanel.setLayout(null);
        _glassPanel.add(this);
    }

    /**
     * Positions window in the center of root pane.
     */
    public void positionRootPaneCenter() {
        // position the window in the center of the rootpane
        setLocation(_rootPane.getWidth() / 2 - getWidth() / 2, _rootPane.getHeight() / 2 - getHeight() / 2);
    }

    /**
     *
     */
    @Override
    public void setVisible(boolean value) {
        super.setVisible(value);
        if (value) {
            startModal();
        } else {
            stopModal();
        }
    }

    /**
     *
     */
    private synchronized void startModal() {
        if (_glassPanel != null) {
            _rootPane.setGlassPane(_glassPanel);
            _glassPanel.setVisible(true); // Change glass pane to our panel
        }
    }

    /**
     *
     */
    private synchronized void stopModal() {
        if (_glassPanel != null) {
            _glassPanel.setVisible(false);
            //reset root pane glass to original
            _rootPane.setGlassPane(_rootPaneInitialGlass);
        }
    }
}
