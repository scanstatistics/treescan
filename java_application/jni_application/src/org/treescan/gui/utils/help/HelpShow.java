/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.treescan.gui.utils.help;

import java.net.URL;
import java.util.Locale;
import javax.help.HelpBroker;
import javax.help.HelpSet;
import javax.help.Popup;
import javax.help.SwingHelpUtilities;
import javax.swing.JOptionPane;
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
        final String helpsetName = "SaTScan_Help";
        final String defaultID = "introduction_htm";
        ShowType showType = helpID.contains("http://") ? HelpShow.ShowType.Website : HelpShow.ShowType.JavaHelp;

        try {
            if (showType == ShowType.Website) {
                BareBonesBrowserLaunch.openURL(helpID);
            } else {
                SwingHelpUtilities.setContentViewerUI("org.satscan.gui.utils.ExternalLinkContentViewerUI");
                ClassLoader cl = TreeScanApplication.class.getClassLoader();
                URL url = HelpSet.findHelpSet(cl, helpsetName, "", Locale.getDefault());
                if (url == null) {
                    url = HelpSet.findHelpSet(cl, helpsetName, ".hs", Locale.getDefault());
                    if (url == null) {
                        JOptionPane.showMessageDialog(null, "The help system could not be located.", " Help", JOptionPane.WARNING_MESSAGE);
                        return;
                    }
                }
                HelpSet mainHS = new HelpSet(cl, url);
                HelpBroker mainHB = mainHS.createHelpBroker();
                if (showType == ShowType.JavaHelpPopup && helpID != null) {
                    Popup popup = (Popup)Popup.getPresentation(mainHS,null);
                    popup.setInvoker (TreeScanApplication.getInstance());
                    popup.setCurrentID(helpID);
                    popup.setDisplayed(true);
                } else {
                    mainHB.setCurrentID(helpID != null ? helpID : defaultID);
                    mainHB.setDisplayed(true);
                }
            }
        } catch (Throwable t) {
            new ExceptionDialog(TreeScanApplication.getInstance(), t).setVisible(true);
        }
    }
}

