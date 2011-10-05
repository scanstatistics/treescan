/*
 *  Application.java
 *  allusionsApp
 *
 *  Created by Matthieu Cormier on Fri Jun 20 2003.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


package org.treescan.gui.utils;

//  This is a stub interface for non Mac OS X java 1.4 environments

import com.apple.eawt.Application;
import com.apple.eawt.ApplicationEvent;
import com.apple.eawt.ApplicationListener;
import java.awt.event.WindowEvent;
import org.treescan.gui.AboutDialog;
import org.treescan.gui.ExceptionDialog;
import org.treescan.gui.TreeScanApplication;

public class MacOSApplication extends Application {

    public MacOSApplication() {
        addApplicationListener(new MacOSApplicationAdapter());
    }
        
    class MacOSApplicationAdapter implements ApplicationListener {
        MacOSApplicationAdapter() {}

        public void handleAbout(ApplicationEvent arg0) {
            try {
                AboutDialog aboutDialog = new AboutDialog(TreeScanApplication.getInstance());
                aboutDialog.setVisible(true);
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.getInstance(), t).setVisible(true);
            }
            arg0.setHandled(true);
        }

        public void handleOpenApplication(ApplicationEvent arg0) {
            //throw new UnsupportedOperationException("Not supported yet.");
        }

        public void handleReOpenApplication(ApplicationEvent arg0) {
            throw new UnsupportedOperationException("Not supported yet.");
        }

        public void handleOpenFile(ApplicationEvent arg0) {
            //throw new UnsupportedOperationException("Not supported yet.");
        }

        public void handlePreferences(ApplicationEvent arg0) {
            //throw new UnsupportedOperationException("Not supported yet.");
        }

        public void handlePrintFile(ApplicationEvent arg0) {
            //throw new UnsupportedOperationException("Not supported yet.");
        }

        public void handleQuit(ApplicationEvent arg0) {
            TreeScanApplication.getInstance().windowClosing(new WindowEvent(TreeScanApplication.getInstance(), WindowEvent.WINDOW_CLOSING));
            arg0.setHandled(true);
        }
    }
}

