/*
 * StartDialog.java
 *
 * Created on December 11, 2007, 12:51 PM
 */

package org.treescan.gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import org.treescan.app.ParameterHistory;

/**
 *
 * @author  Hostovic
 */
public class StartDialog extends javax.swing.JDialog {
    enum StartType {NEW, SAVED, LAST, CANCEL};
    private StartType _openType = StartType.CANCEL;
    
    /**
     * Creates new form StartDialog
     */
    public StartDialog(java.awt.Frame parent) {
        super(parent, true);
        initComponents();
        _open_last_session.setEnabled(ParameterHistory.getInstance().getHistoryList().size() > 0);
        _open_last_session.setToolTipText(ParameterHistory.getInstance().getHistoryList().get(0).toString());
        setLocationRelativeTo(parent);
    }
    
    public StartType GetOpenType() {
        return _openType;
    }    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jButton1 = new javax.swing.JButton();
        jPanel1 = new javax.swing.JPanel();
        _cancel_dialog = new javax.swing.JButton();
        _cancel_dialog.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _openType = StartType.CANCEL;
                setVisible(false);
            }
        } );
        _open_last_session = new javax.swing.JButton();
        _open_last_session.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _openType = StartType.LAST;
                setVisible(false);
            }
        } );
        _open_saved_session = new javax.swing.JButton();
        _open_saved_session.addActionListener( new ActionListener() {     public void actionPerformed( ActionEvent e ) {         _openType = StartType.SAVED;         setVisible(false);     } } );
        _new_session = new javax.swing.JButton();
        _new_session.setMnemonic(KeyEvent.VK_B);
        _new_session.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _openType = StartType.NEW;
                setVisible(false);
            }
        } );

        jButton1.setText("jButton1");

        setTitle("Start Window");
        setModal(true);

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Welcome to TreeScan", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 12))); // NOI18N

        _cancel_dialog.setText("Cancel");
        _cancel_dialog.addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                _openType = StartType.CANCEL;
                setVisible(false);
            }
        } );

        _open_last_session.setText("Open Last Session");

        _open_saved_session.setText("Open Saved Session");

        _new_session.setText("Create New Session");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_open_last_session, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_cancel_dialog, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_open_saved_session, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_new_session, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_new_session)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_open_saved_session)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_open_last_session)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_cancel_dialog)
                .addContainerGap())
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
   
   
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton _cancel_dialog;
    private javax.swing.JButton _new_session;
    private javax.swing.JButton _open_last_session;
    private javax.swing.JButton _open_saved_session;
    private javax.swing.JButton jButton1;
    private javax.swing.JPanel jPanel1;
    // End of variables declaration//GEN-END:variables
    
}
