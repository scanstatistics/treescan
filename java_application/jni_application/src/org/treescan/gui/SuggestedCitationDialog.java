/*
 * SuggestedCitationDialog.java
 *
 * Created on December 11, 2007, 10:07 AM
 */
package org.treescan.gui;

import java.net.URL;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import org.treescan.utils.BareBonesBrowserLaunch;

/**
 *
 * @author  Hostovic
 */
public class SuggestedCitationDialog extends javax.swing.JDialog {

    /** Creates new form SuggestedCitationDialog */
    public SuggestedCitationDialog(java.awt.Frame parent, URL urlLink) throws Exception {
        super(parent, true);
        initComponents();
        jEditorPane1.setPage(urlLink);
        jEditorPane1.addHyperlinkListener(new Hyperactive());
        setLocationRelativeTo(parent);
    }

    class Hyperactive implements HyperlinkListener {

        public void hyperlinkUpdate(HyperlinkEvent e) {
            if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
                BareBonesBrowserLaunch.openURL(e.getURL().toString());
            } else if (e.getEventType() == HyperlinkEvent.EventType.ENTERED) {
                jEditorPane1.setToolTipText(e.getURL().toString());
            } else {
                jEditorPane1.setToolTipText("");
            }
        }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jScrollPane1 = new javax.swing.JScrollPane();
        jEditorPane1 = new javax.swing.JEditorPane();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Suggested Citation");
        jEditorPane1.setEditable(false);
        jEditorPane1.setContentType("text/html");
        jScrollPane1.setViewportView(jEditorPane1);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 514, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 350, Short.MAX_VALUE)
                .addContainerGap())
        );
        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JEditorPane jEditorPane1;
    private javax.swing.JScrollPane jScrollPane1;
    // End of variables declaration//GEN-END:variables
}
