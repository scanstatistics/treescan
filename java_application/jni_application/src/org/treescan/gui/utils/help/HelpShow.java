/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.treescan.gui.utils.help;

import java.net.URL;
import java.util.Locale;
import javax.swing.JOptionPane;
import org.treescan.app.UnknownEnumException;
import org.treescan.gui.ExceptionDialog;
import org.treescan.gui.TreeScanApplication;
import org.treescan.utils.BareBonesBrowserLaunch;

/**
 *
 * @author hostovic
 */
public class HelpShow {
    public enum ShowType {JavaHelp, JavaHelpPopup, Website};
    /**
     * Shows Java help system, initially directing to section at 'helpID' if not null.
     * Conditionally shows help as popup window.
     * @param helpID
     */
     public static void showHelp(String helpID) {
        ShowType showType = helpID.contains("http://") ? HelpShow.ShowType.Website : HelpShow.ShowType.JavaHelp;
        try {
            if (showType == ShowType.Website)
                BareBonesBrowserLaunch.openURL(helpID);
            else
                throw new UnknownEnumException(showType);
        } catch (Throwable t) {
            new ExceptionDialog(TreeScanApplication.getInstance(), t).setVisible(true);
        }
    }
}

