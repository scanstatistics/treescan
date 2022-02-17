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
import java.nio.file.FileVisitResult;
import java.nio.file.FileVisitor;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Set;
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
    private boolean is64BitVM = true;

    /** Creates new form MainFrame */
    public InstallerFrame(String archiveFile, String launchApp, Vector<String> relaunchArgs) {
        initComponents();
        try {
            _archive = new ZipFile(archiveFile);
            _launchApp = new File(launchApp);
            _relaunchArgs = relaunchArgs;
            setIconImage(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/org/treescan/updaterapplication/TreeScan.gif")));
            setLocationRelativeTo(null);
            
          String bits = null;
          String vm_name = null;
          String os_arch = null;
          try { // first try to get VM type from 'sun.arch.data.model' property
            bits = System.getProperty("sun.arch.data.model");
          } catch (Throwable t) {System.out.println("'sun.arch.data.model' property not avaiable");}
          try { // second try to get VM type from 'java.vm.name' property
            vm_name = System.getProperty("java.vm.name");
          } catch (Throwable t) {System.out.println("'java.vm.name' property not avaiable");}
          try { // lastly try to get VM type from 'os.arch' property, this is the least best choice
            // since OS arch could be 64-bit but VM 32-bit
            os_arch = System.getProperty("os.arch");
          } catch (Throwable t) {System.out.println("'os.arch' property not avaiable");}
        
          if (bits != null) {
            is64BitVM = (bits.indexOf("64") >= 0);
            System.out.println("'sun.arch.data.model' property indicating VM data model is " + (is64BitVM ? "64" : "32") + "-bit");
          } else if (vm_name != null) {
            is64BitVM = vm_name.indexOf("64") >= 0; 
            System.out.println("'java.vm.name' property indicating VM data model is " + (is64BitVM ? "64" : "32") + "-bit");
          } else if (os_arch != null) {
            is64BitVM = os_arch.indexOf("64") >= 0; 
            System.out.println("'os.arch' property indicating VM data model is " + (is64BitVM ? "64" : "32") + "-bit");
          } else {
            is64BitVM = false; 
            System.out.println("Assuming VM data model is 32-bit.");
          }
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
            if (System.getProperty("os.name").toLowerCase().startsWith("mac")) {
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

    /* Invokes worker process to install updates. */
    public void installUpdate(boolean showEULA) throws Exception {
        try {
            setVisible(true);
            if (System.getProperty("os.name").toLowerCase().startsWith("mac")) {
                /* The application update process on the Mac is no longer valid. Teh user must uninstall then download new version. */
                JOptionPane.showMessageDialog(this, 
                    "TreeScan updater is no longer in service on the macOS.\n" +
                    "Please uninstall the current TreeScan version then install the last release from TreeScan website.", 
                    "TreeScan Update Cancelled", JOptionPane.INFORMATION_MESSAGE
                );
                relaunchApplication();
            } else {            
                if (showEULA) {
                    ULADialog ula = new ULADialog(this);
                    ula.setVisible(true);
                    if (ula._accepted == false) {
                        throw new updateCancelledException("User License Agreement not accepted.");
                    }
                }
                new Extracter().execute();
            }
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

        /*This function determines whether the ZipEntry should be extracted. */
        private boolean isExtracted(ZipEntry entry) {
            // We're bundling both x64 and x86 for Windows -- extract jre detected VM bitness, skip other.
            if (System.getProperty("os.name").toLowerCase().startsWith("windows") &&
                ((is64BitVM && entry.getName().contains("jre_x86")) || (!is64BitVM && entry.getName().contains("jre_x64"))))
                   return false;
            return true;
        }

        /* Determines write access of archive file. */
        private void checkAccess(File f) throws Exception {
            if (f.exists() && !f.isDirectory()) {
                //System.out.println("checkAccess file " + f.toString());
                boolean attemptAgain = true;
                while (attemptAgain) {
                    OutputStream outStream = null;
                    try {
                        outStream = new BufferedOutputStream(new FileOutputStream(f, true));
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

        /* Returns complete file path relative to launch application. */
        private File getCompleteFile(ZipEntry entry) {
            String entryName = entry.getName();
            // We're bundling both x64 and x86 for Windows -- renaming jre version for detected VM bitness.
            if (System.getProperty("os.name").toLowerCase().startsWith("windows")) {
               if (is64BitVM && entryName.contains("jre_x64")) {
                   entryName = entryName.replace("jre_x64", "jre");
               } else if (!is64BitVM && entryName.contains("jre_x86")) {
                   entryName = entryName.replace("jre_x86", "jre");
               }
            }
            return new File(_launchApp.getParent(), entryName);
        }

        /**
         * Attempts to extract archive member to hard drive.
         */
        private void extractMember(ZipEntry entry) {
            
            InputStream inStream = null;
            OutputStream outStream = null;
            try {
                File targetFile = getCompleteFile(entry);
                if (entry.isDirectory()) {
                    //System.out.println("Extracting directory " + targetFile.toString());
                    targetFile.mkdir();
                    targetFile.setWritable(true);
                } else {
                    //System.out.println("Extracting file " + targetFile.toString());
                    targetFile.setWritable(true);                

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
                    //System.out.println("Done with " + targetFile.toString());
                }
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
                // Determine which files are currently installed.
                Set<String> existing = new HashSet<>();                
                Files.walkFileTree(Paths.get(_launchApp.getParent()), new HashSet<>(), 2, new FileVisitor<Path>() {
                    @Override
                    public FileVisitResult preVisitDirectory(Path dir, BasicFileAttributes attrs) throws IOException {
                        existing.add(dir.toString());
                        return FileVisitResult.CONTINUE;
                    }

                    @Override
                    public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                        existing.add(file.toString());
                        return FileVisitResult.CONTINUE;
                    }

                    @Override
                    public FileVisitResult visitFileFailed(Path file, IOException exc) throws IOException {
                        return FileVisitResult.CONTINUE;
                    }

                    @Override
                    public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                        return FileVisitResult.CONTINUE;
                    }
                });                        
                
                _progressBar.setMaximum(_archive.size() * 2);
                _progressBar.setValue(0);
                //System.out.println("check file access ...");
                //check file access...
                Enumeration checkEntries = _archive.entries();
                while (checkEntries.hasMoreElements()) {
                    ZipEntry entry = (ZipEntry) checkEntries.nextElement();
                    if (isExtracted(entry) && !entry.isDirectory()) {
                        checkAccess(getCompleteFile(entry));
                    }
                    _progressBar.setValue(_progressBar.getValue() + 1);
                }
                //System.out.println("extract files...");
                //extract files...
                Enumeration extractEntries = _archive.entries();
                while (extractEntries.hasMoreElements()) {
                    ZipEntry entry = (ZipEntry) extractEntries.nextElement();
                    if (isExtracted(entry)) {
                        extractMember(entry);
                        existing.remove(getCompleteFile(entry).toString());
                    }
                    _progressBar.setValue(_progressBar.getValue() + 1);
                }
                
                System.out.println("Orphaned files .. ");
                for (String temp : existing) {
                    System.out.println(temp);
                }                
                System.out.println(" .. done.");
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
                //if (!System.getProperty("os.name").startsWith("Windows")) {
                //    //create symbolic links to shared objects -- still in progress
                //    _progressLabel.setText("Creating symbolic links ...");
                //    // ### hard coding gcc3.3.5 shared objects until a suitable design is determined ###
                //    Runtime.getRuntime().exec("ln -sf libsatscan.v8.0.x.gcc3.3.5_x86_64_32bit.so libsatscan32.so", null, _launchApp.getParentFile());
                //    Runtime.getRuntime().exec("ln -sf libsatscan.v8.0.x.gcc3.3.5_x86_64_64bit.so libsatscan64.so", null, _launchApp.getParentFile());
                //}
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
