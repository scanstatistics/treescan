/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.treescan.updaterapplication;

import java.util.StringTokenizer;
import java.util.Vector;
import javax.swing.JOptionPane;
import javax.swing.UIManager;

/**
 *
 * @author Scott
 */
public class Main {

    private static String VER_ID_OPTION_STRING = "-ver_id";
    private static String RELAUNCH_ARGS_OPTION = "relaunch_args=";
    private static String RELAUNCH_TOKEN = "&";

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            if (args.length < 2) {
                throw new RuntimeException("Invalid argument count.\nUsage: 'archive filename' 'launch application' [-ver_id[=val]] [relaunch_args=<arg&arg...>]");
            }
            //check and show End User License Agreement if not "unrequested":
            boolean hasVersionOption = false;
            for (int i=2; i < args.length && !hasVersionOption; ++i) {
                hasVersionOption = args[i].startsWith(VER_ID_OPTION_STRING);
            }            
            boolean hasArgsOption = false;
            Vector<String> relaunchArgs = new Vector<String>();
            for (int i=2; i < args.length && !hasArgsOption; ++i) {
                if (args[i].startsWith(RELAUNCH_ARGS_OPTION)) {
                    hasArgsOption = true;
                    StringTokenizer tokener = new StringTokenizer(args[i].substring(args[i].indexOf("=") + 1), RELAUNCH_TOKEN);
                    while (tokener.hasMoreTokens())
                        relaunchArgs.add(tokener.nextToken());
                }
            }
            //run the update
            new InstallerFrame(args[0], args[1], relaunchArgs).installUpdate(!hasVersionOption);
        } catch (Throwable t) {
            JOptionPane.showMessageDialog(null, String.format("TreeScan update was aborted due to an error while reading updates.\nPlease email TreeScan with the following information:\n\n%s.", t.getMessage()), "TreeScan Update Aborted", JOptionPane.ERROR_MESSAGE);
            System.exit(0);
        }
    }
}
