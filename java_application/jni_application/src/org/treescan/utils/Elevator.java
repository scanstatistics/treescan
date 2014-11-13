/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.treescan.utils;

import org.treescan.utils.Shell32X.SHELLEXECUTEINFO;

import com.sun.jna.WString;
import com.sun.jna.platform.win32.Kernel32;
import com.sun.jna.platform.win32.Kernel32Util;
import javax.swing.JOptionPane;

public class Elevator {
    public static void main(String... args) {
        executeAsAdministrator("c:\\windows\\system32\\notepad.exe", "", null);
    }

    public static void executeAsAdministrator(String command, String args, String directory) {
        Shell32X.SHELLEXECUTEINFO execInfo = new Shell32X.SHELLEXECUTEINFO();
        execInfo.lpFile = new WString(command);
        if (args != null)
            execInfo.lpParameters = new WString(args);
        execInfo.nShow = Shell32X.SW_HIDE;
        execInfo.fMask = Shell32X.SEE_MASK_NOCLOSEPROCESS;
        execInfo.lpVerb = new WString("runas");
        if (directory != null)
            execInfo.lpDirectory = new WString(directory);
        boolean result = Shell32X.INSTANCE.ShellExecuteEx(execInfo);
        if (!result) {
            int lastError = Kernel32.INSTANCE.GetLastError();
            String errorMessage = Kernel32Util.formatMessageFromLastErrorCode(lastError);
            throw new RuntimeException("Error performing elevation: " + lastError + ": " + errorMessage + " (apperror=" + execInfo.hInstApp + ")");
        }
    }
}