/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.treescan.gui.utils;

import ca.guydavis.swing.desktop.JWindowsMenu;
import javax.swing.JDesktopPane;

/**
 * Extends JWindowsMenu to customize menus.
 * @author Hostovic
 */
public class WindowsMenu extends JWindowsMenu {
    public WindowsMenu(JDesktopPane desktop) {
        super("Windows", desktop);
        removeAll();
    }
}
