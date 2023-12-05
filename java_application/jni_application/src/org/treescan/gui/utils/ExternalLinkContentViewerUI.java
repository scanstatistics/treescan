/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.treescan.gui.utils;

import java.awt.Desktop;
import java.net.URL;
import javax.help.JHelpContentViewer;
import javax.help.plaf.basic.BasicContentViewerUI;
import javax.swing.JComponent;
import javax.swing.event.HyperlinkEvent;

/**
  * A UI subclass that will open external links (website or mail links) in an external browser
*/
public class ExternalLinkContentViewerUI extends BasicContentViewerUI{
    public ExternalLinkContentViewerUI(JHelpContentViewer x){
        super(x);
    }
            
    public static javax.swing.plaf.ComponentUI createUI(JComponent x){
	return new ExternalLinkContentViewerUI((JHelpContentViewer)x);
    }

    @Override
    public void hyperlinkUpdate(HyperlinkEvent he){
        if (he.getEventType()==HyperlinkEvent.EventType.ACTIVATED){
            try {
                URL u = he.getURL();
		if (u.getProtocol().equalsIgnoreCase("mailto")||u.getProtocol().equalsIgnoreCase("http")||u.getProtocol().equalsIgnoreCase("ftp")){
                    Desktop.getDesktop().browse(u.toURI());
                    return;
		}
            } catch(Throwable t){}
        }
	super.hyperlinkUpdate(he);
    }
}