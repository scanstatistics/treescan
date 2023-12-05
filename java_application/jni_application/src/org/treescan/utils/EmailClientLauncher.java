/*
 * EmailClientLauncher.java
 *
 * Created on December 10, 2007, 4:02 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.utils;

import java.awt.Desktop;
import java.net.URI;
import org.treescan.app.AppConstants;

/**
 *
 * @author Hostovic
 */
public class EmailClientLauncher {

    private static int MAX_BODY_LEN = 1500;

    /** Creates a new instance of EmailClientLauncher */
    public EmailClientLauncher() {
    }

    public String getSystemInfo() {
        // append environment variables
        StringBuilder info = new StringBuilder("System Information:\n");
        try { // JNI library might not be available.
            info.append("TreeScan Application Version").append(" : ").append(AppConstants.getGraphicalVersion()).append("\n");
            info.append("TreeScan Engine Version").append(" : ").append(AppConstants.getVersion()).append("\n");
            info.append("TreeScan Version Id").append(" : ").append(AppConstants.getVersionId()).append("\n");
        } catch (Throwable e) {}
        info.append("os.name").append(" : ").append(System.getProperty("os.name")).append("\n");
        info.append("java.vendor").append(" : ").append(System.getProperty("java.vm.vendor")).append("\n");
        info.append("java.runtime.version").append(" : ").append(System.getProperty("java.runtime.version")).append("\n");
        info.append("os.arch").append(" : ").append(System.getProperty("os.arch")).append("\n");
        return info.append("\n").toString();
    }
    
    /**
     * Launches default email application
     */
    public boolean launchDefaultClientEmail(String mailTo, String subject, String body) {        
        try {
            if (!(Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.MAIL)))
                return false;

            URI uriMailTo = null;            
            String _text =  "Please provide any additional information regarding the problem you are experiencing:\n\n" + getSystemInfo() + body;
            // There appears to be a limitation either in the Java API or with Outlook (assuming most users use Outlook)
            // with passing body text; empirically noted that limit is in the region of > 1500 characters ...
            if (_text != null && System.getProperties().getProperty("os.name").toLowerCase().startsWith("windows")) {
                _text = _text.substring(0, Math.min(MAX_BODY_LEN, _text.length()));
            }
            StringBuilder uriText = new StringBuilder();
            uriText.append(mailTo);
            if (subject != null && subject.length() > 0) {
                uriText.append("?SUBJECT=" + subject);
            }
            if (_text != null && _text.length() > 0) {
                uriText.append("&BODY=" + _text);
            }
            uriMailTo = new URI("mailto", uriText.toString(), null);
            Desktop.getDesktop().mail(uriMailTo);
        } catch (Exception e) {
            // Try launching without body ...
            return launchDefaultClientEmail(mailTo, subject);
        }
        return true;
    }

    /**
     * Launches default email application.
     */
    public boolean launchDefaultClientEmail(String mailTo, String subject) {
        try {
            if (!(Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.MAIL)))
                return false;

            URI uriMailTo = null;
            StringBuilder uriText = new StringBuilder();
            uriText.append(mailTo);
            if (subject != null && subject.length() > 0) {
                uriText.append("?SUBJECT=" + subject + "&BODY=" + "Please describe the problem you are experiencing:\n\n" + getSystemInfo());
            }
            uriMailTo = new URI("mailto", uriText.toString(), null);
            Desktop.getDesktop().mail(uriMailTo);
        } catch (Exception e) {
            return false;
        }
        return true;
    }
}
