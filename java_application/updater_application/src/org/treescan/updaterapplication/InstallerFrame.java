/*
 * MainFrame.java
 *
 * Created on December 21, 2007, 9:55 PM
 */
package org.treescan.updaterapplication;

import java.awt.Toolkit;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import javax.swing.JOptionPane;
import javax.swing.SwingWorker;

/**
 * Installer window.
 * @author  Scott
 */
public class InstallerFrame extends javax.swing.JFrame {

    private final ZipFile _archive;
    private final File _launchApp;
    private final Vector<String> _relaunchArgs;

    /** Creates new form MainFrame */
    public InstallerFrame(String archiveFile, String launchApp, Vector<String> relaunchArgs) {
        initComponents();
        try {
            _archive = new ZipFile(archiveFile);
            _launchApp = new File(launchApp);
            _relaunchArgs = relaunchArgs;
            setIconImage(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/org/treescan/updaterapplication/TreeScan.gif")));
            setLocationRelativeTo(null);
        } catch (IOException e) {//
            throw new archiveException(e.getMessage());
        }
    }

    private void relaunchApplication() {
        try {
            // Get java path from System
            StringBuilder java_path = new StringBuilder();
            java_path.append(System.getProperty("java.home")).append(System.getProperty("file.separator")).append("bin").append(System.getProperty("file.separator")).append("java");                            
            // Build command to relaunch TreeScan
            Vector<String> execute = new Vector<String>();
            if (System.getProperty("os.name").startsWith("Mac")) {
                execute.add("open");
                //change from likely name 'TreeScan.jar' to 'TreeScan.app'
                execute.add(_launchApp.getName().replace(".jar", ".app"));
            } else {
                execute.add(java_path.toString());
                execute.add("-jar");
                execute.add("-Djava.library.path=.");
                execute.add(_launchApp.getName());
                for (int i=0; i < _relaunchArgs.size(); ++i) {
                    execute.add(_relaunchArgs.elementAt(i));
                }
            }
            Runtime.getRuntime().exec(execute.toArray(new String[]{}), null, _launchApp.getParentFile());            
        } catch (IOException ex) {
            JOptionPane.showMessageDialog(this, ex.getMessage() + "\n" + _launchApp.getAbsolutePath()
                    + "\n" + _launchApp.getName() + "\n" + _launchApp.getParentFile()
                     , "TreeScan Update Cancelled", JOptionPane.INFORMATION_MESSAGE);
            Logger.getLogger(InstallerFrame.class.getName()).log(Level.SEVERE, null, ex);
        } finally {
            InstallerFrame.this.setVisible(false);
            System.exit(0);
        }
    }

    /**
     * Invokes worker process to install updates.
     */
    public void installUpdate(boolean showEULA) throws Exception {
        try {
            setVisible(true);
            if (showEULA) {
                ULADialog ula = new ULADialog(this);
                ula.setVisible(true);
                if (ula._accepted == false) {
                    throw new updateCancelledException("User License Agreement not accepted.");
                }
            }
            new Extracter().execute();
        } catch (updateCancelledException e) {
            JOptionPane.showMessageDialog(this, "TreeScan update was cancelled. Please try to update again at a later time.", "TreeScan Update Cancelled", JOptionPane.INFORMATION_MESSAGE);
            deleteArchive();
            setVisible(false);
        } catch (Throwable t) {
            JOptionPane.showMessageDialog(this, t.getMessage(), "Note", JOptionPane.ERROR_MESSAGE);
            setVisible(false);
            deleteArchive();
        }
    }

    /**
     * Attempts to delete archive file from hard drive.
     */
    private void deleteArchive() {
        try {
            _archive.close();
            new File(_archive.getName()).delete();
        } catch (IOException ex) {
        }
    }

    class Extracter extends SwingWorker<Void, Void> {

        private static final int _bufferSize = 4096;

        /**
         * This function determines whether the ZipEntry should be extracted
         * given the OS it is being installed on. The current implementation
         * is very limited, we probably should make this more robust in the future.
         * For now, this hack will work ...
         */
        private boolean isExtracted(ZipEntry entry) {
            if (entry.getName().endsWith(".jar") ||
                entry.getName().endsWith(".prm") ||
                entry.getName().endsWith(".cas") ||
                entry.getName().endsWith(".ctl") ||
                entry.getName().endsWith(".pop") ||
                entry.getName().endsWith(".geo") ||
                entry.getName().endsWith(".grd") ||
                entry.getName().endsWith(".pdf")) {
                return true;
            } else if (System.getProperty("os.name").toLowerCase().startsWith("windows")) {
                return entry.getName().endsWith(".exe") || 
                       entry.getName().endsWith(".dll") ? true : false;
            } else if (System.getProperty("os.name").toLowerCase().startsWith("linux")) {
                return entry.getName().endsWith("_x86_64_32bit") || 
                       entry.getName().endsWith("_x86_64_64bit") || 
                       entry.getName().endsWith("_x86_64_32bit.so") || 
                       entry.getName().endsWith("_x86_64_64bit.so") ? true : false;
            } else if (System.getProperty("os.name").toLowerCase().startsWith("mac") || System.getProperty("os.name").toLowerCase().startsWith("darwin")) {
                return true; // Mac update should never be bundled with other platform updates ... so extract all files.
            } else if (System.getProperty("os.name").toLowerCase().startsWith("sunos") || System.getProperty("os.name").toLowerCase().startsWith("solaris")) {
                return entry.getName().endsWith("_sparc_32bit") || 
                       entry.getName().endsWith("_sparc_64bit") || 
                       entry.getName().endsWith("_sparc_32bit.so") || 
                       entry.getName().endsWith("_sparc_64bit.so") ? true : false;
            } else {//Unknown platform - default to true
                return true;
            }
        }

        /** 
         * Determines write access of archive file.
         */
        private void checkAccess(File f) throws Exception {
            if (f.exists() && !f.isDirectory()) {
                boolean attemptAgain = true;
                while (attemptAgain) {
                    OutputStream outStream = null;
                    try {
                        outStream = new BufferedOutputStream(new FileOutputStream(f));
                    } catch (Exception e) {
                        String message = "The updater have determined that file '" + f.getAbsolutePath() +
                                "'\nis open or active and can not be correctly updated.\n\n" +
                                "Please shutdown any application(s) accessing this file and select Ok.\n\n" +
                                "Note that you might not have privelages to perform this update.\n"; 
                        if (System.getProperty("os.name").toLowerCase().startsWith("windows")) {
                            message += "In that case, you will need to Cancel this update, restart TreeScan with the 'Run as adminstrator'\n" +
                                       "Windows feature and select the 'Check for New Version' option again.";
                        } else {
                            message += "In that case, you will need to Cancel this update, restart TreeScan as an adminstrator\n" +
                                       "and select the 'Check for New Version' option again.";
                        }
                        int option = JOptionPane.showConfirmDialog(InstallerFrame.this, message, "TreeScan Update", JOptionPane.OK_CANCEL_OPTION);
                        if (option != JOptionPane.YES_OPTION) {
                            throw new fileAccessException(f.getAbsolutePath());
                        }
                    } finally {
                        attemptAgain = outStream == null;
                        if (outStream != null) {
                            outStream.flush();
                            outStream.close();
                        }
                    }
                }
            }
        }

        /**
         * Returns complete file path relative to launch application.
         */
        private File getCompleteFile(ZipEntry entry) {
            return new File(_launchApp.getParent(), entry.getName());
        }

        /**
         * Attempts to extract archive member to hard drive.
         */
        private void extractMember(ZipEntry entry) {
            InputStream inStream = null;
            OutputStream outStream = null;
            try {
                File targetFile = getCompleteFile(entry);
                if (targetFile.isDirectory()) { //skip directories
                    return;
                }
                targetFile.getParentFile().mkdirs();
                inStream = new BufferedInputStream(_archive.getInputStream(entry));
                outStream = new BufferedOutputStream(new FileOutputStream(targetFile));

                byte[] b = new byte[_bufferSize];
                int readCount;
                while ((readCount = inStream.read(b, 0, b.length)) > 0) {
                    outStream.write(b, 0, readCount);
                }
                outStream.close();
                inStream.close();
            } catch (IOException e) {
                throw new archiveException(e.getMessage());
            } finally {
                try {
                    if (inStream != null) {
                        inStream.close();
                    }
                    if (outStream != null) {
                        outStream.close();
                    }
                } catch (IOException e) {
                    throw new archiveException(e.getMessage());
                }
            }
        }

        @Override
        protected Void doInBackground() throws Exception {
            try {
                _progressBar.setMaximum(_archive.size() * 2);
                _progressBar.setValue(0);
                //check file access...
                Enumeration checkEntries = _archive.entries();
                while (checkEntries.hasMoreElements()) {
                    ZipEntry entry = (ZipEntry) checkEntries.nextElement();
                    if (isExtracted(entry)) {
                        checkAccess(getCompleteFile(entry));
                    }
                    _progressBar.setValue(_progressBar.getValue() + 1);
                }
                //extract files...
                Enumeration extractEntries = _archive.entries();
                while (extractEntries.hasMoreElements()) {
                    ZipEntry entry = (ZipEntry) extractEntries.nextElement();
                    if (isExtracted(entry)) {
                        extractMember(entry);
                    }
                    _progressBar.setValue(_progressBar.getValue() + 1);
                }
            } catch (archiveException e) {
                JOptionPane.showMessageDialog(InstallerFrame.this, String.format("TreeScan update was aborted due to an error while reading updates.\nPlease email TreeScan with the following information:\n\n%s.", e.getMessage()), "TreeScan Update Aborted", JOptionPane.ERROR_MESSAGE);
                deleteArchive();
            } catch (fileAccessException e) {
                JOptionPane.showMessageDialog(InstallerFrame.this, "TreeScan update was cancelled. Please try to update again at a later time.", "TreeScan Update Cancelled", JOptionPane.INFORMATION_MESSAGE);
                deleteArchive();
            } catch (Throwable t) {
                JOptionPane.showMessageDialog(InstallerFrame.this, t.getMessage(), "Note", JOptionPane.ERROR_MESSAGE);
                deleteArchive();
            }
            return null;
        }

        @Override
        public void done() {
            try {
                deleteArchive();
                _progressLabel.setText("Update completed ...");
            //} catch (IOException ex) {
            //    Logger.getLogger(InstallerFrame.class.getName()).log(Level.SEVERE, null, ex);
            //}    
            } finally { // time to restart application
                relaunchApplication();
            }
        }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel1 = new javax.swing.JPanel();
        _progressLabel = new javax.swing.JLabel();
        _progressBar = new javax.swing.JProgressBar();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setLocationByPlatform(true);
        setResizable(false);

        _progressLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _progressLabel.setText("Installing Update ...");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_progressBar, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 271, Short.MAX_VALUE)
                    .addComponent(_progressLabel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 271, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGap(0, 0, 0)
                .addComponent(_progressLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_progressBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
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
    private javax.swing.JProgressBar _progressBar;
    private javax.swing.JLabel _progressLabel;
    private javax.swing.JPanel jPanel1;
    // End of variables declaration//GEN-END:variables
}
