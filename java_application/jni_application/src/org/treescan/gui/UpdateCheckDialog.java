/*
 * UpdateCheckDialog.java
 *
 * Created on December 21, 2007, 2:14 PM
 */
package org.treescan.gui;

import java.awt.CardLayout;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import javax.swing.SwingWorker;
import java.util.Base64;
import org.treescan.gui.utils.WaitCursor;
import org.treescan.app.AppConstants;
import org.treescan.utils.BareBonesBrowserLaunch;
import org.treescan.gui.utils.JHyperLink;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.io.FileNotFoundException;
import java.net.URI;
import java.nio.file.StandardCopyOption;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.JOptionPane;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.SystemUtils;

/** @author  Hostovic */
public class UpdateCheckDialog extends javax.swing.JDialog {
    private final TreeScanApplication _applicationFrame;

    private boolean _updateExists = false;
    private int _error_code = 0;
    private FileInfo _update_application_info = null;
    private FileInfo _update_archive_info = null;
    private FileInfo _download_file_info = null;
    private String _new_update_version = "";
    private boolean _download_completed = false;
    private boolean _restart_required = false;

    private static final String CARD_CHECK = "check";
    private static final String CARD_UPDATE = "update";
    private static final String CARD_NOUPDATE = "noUpdate";
    private static final String CARD_DOWNLOAD = "download";
    private static final String CARD_FAILED = "failed";
    private static final String CARD_UPDATEINFOONLY = "updateinfoonly";

    public static boolean _runUpdateOnTerminate = false;
    public static File _updater_filename = null;
    public static File _update_archivename = null;

    /** Creates new form UpdateCheckDialogs */
    public UpdateCheckDialog(TreeScanApplication applicationFrame) {
        super(applicationFrame, true);
        initComponents();
        _applicationFrame = applicationFrame;
        _checkFrequency.setModel(new javax.swing.DefaultComboBoxModel(ApplicationPreferences.updateFrequencyChoices()));
        _checkFrequency.setSelectedItem(ApplicationPreferences.getUpdateFrequency());
        setLocationRelativeTo(applicationFrame);
        _installer_download_link.setVisible(false);
        _installer_download_progress.setVisible(false);        
        if (ApplicationPreferences.shouldCheckUpdate()) {
            new CheckUpdateTask().execute();
        }
    }

    /** Returns whether the application has to restart for update. */
    public boolean restartRequired() {
        return _restart_required;
    }

    public static File getDownloadTempDirectory() {
        return new File(System.getProperty("java.io.tmpdir"));
    }

    /* Retutns users 'Download' directory or user home directory. s*/
    public static File getUserDownloadDirectory() {
        File test = new File(System.getProperty("user.home") + File.separator + "Downloads");
        return test.exists() ? test : new File(System.getProperty("user.home"));
    }      
    
    static public String getErrorText(int errorcode) {
        StringBuilder text = new StringBuilder();
        text.append("<html>Unable to check for update. This might be because the update site is temporarily unavailable or your internet connection is down. ");
        text.append("If this problem persists, you can always download the latest version from the web site directly.");
        if (errorcode > 0) text.append(" (code=").append(errorcode).append(")");
        text.append("</html>");
        return text.toString();
    }

    /**
     * Overrides setVisible() method
     */
    @Override
    public void setVisible(boolean show) {
        try {
            if (show) {
                if (!_updateExists) { // An update isn't known to exist -- check now.
                    CardLayout cl = (CardLayout) (_cardsPanel.getLayout());
                    cl.show(_cardsPanel, CARD_CHECK);
                    new CheckUpdateTask().execute();
                } else if (SystemUtils.IS_OS_WINDOWS || SystemUtils.IS_OS_MAC) // Display status and option to download installer/app.
                    ((CardLayout)_cardsPanel.getLayout()).show(_cardsPanel, CARD_UPDATEINFOONLY);                    
                else // Display status and option to perform application update.
                    ((CardLayout)_cardsPanel.getLayout()).show(_cardsPanel, CARD_UPDATE);                    
            }
            super.setVisible(show);
        } catch (Throwable t) {
            throw new RuntimeException(t.getMessage(), t);
        }
    }

    /** returns full path of download file */
    private File getFile(String fileName) {
        try {
            return File.createTempFile("TreeScan", fileName, getDownloadTempDirectory());
        } catch (IOException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /* Return indication of whether updates exist. */
    public boolean hasUpdate() {
        return _updateExists;
    }

    /* Returns HttpURLConnection for URL -- applying Authorization as requested by user. */
    private HttpURLConnection getHttpURLConnection(URL url, boolean setDoOutput) throws IOException {
        HttpURLConnection connection = (HttpURLConnection)url.openConnection();
        HttpURLConnection.setFollowRedirects(true);
        if (setDoOutput) connection.setDoInput(true);
        // Apply authentication if specified by user.s
        if (TreeScanApplication.getDebugAuth().length() > 0) {
            connection.setRequestProperty(
                "Authorization", "Basic " + new String(Base64.getEncoder().encode(TreeScanApplication.getDebugAuth().getBytes()))
            );
        }
        connection.connect();
        switch (connection.getResponseCode()) {
            case HttpURLConnection.HTTP_OK:
                return connection;
            case HttpURLConnection.HTTP_MOVED_PERM:
            case HttpURLConnection.HTTP_MOVED_TEMP:
            case HttpURLConnection.HTTP_SEE_OTHER:
                connection.disconnect();
                return (HttpURLConnection)getHttpURLConnection(new URL(connection.getHeaderField("Location")), setDoOutput);
            default:
                throw new RuntimeException("Application update failed with error code " + connection.getResponseCode());
        }
    }    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _cardsPanel = new javax.swing.JPanel();
        _checkingForUpdatePanel = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        _noUpdatePanel = new javax.swing.JPanel();
        jLabel2 = new javax.swing.JLabel();
        _noUpdateOkButton = new javax.swing.JButton();
        jLabel3 = new javax.swing.JLabel();
        _checkFrequency = new javax.swing.JComboBox();
        _updatePanel = new javax.swing.JPanel();
        _updateLabel = new javax.swing.JLabel();
        _doUpdateButton = new javax.swing.JButton();
        _doNotUpdateButton = new javax.swing.JButton();
        _downloadPanel = new javax.swing.JPanel();
        _stepLabel = new javax.swing.JLabel();
        _downloadProgressBar = new javax.swing.JProgressBar();
        jButton3 = new javax.swing.JButton();
        failedPanel = new javax.swing.JPanel();
        errorText = errorText = new javax.swing.JLabel(getErrorText(0));
        jLabel6 = new javax.swing.JLabel();
        webSiteLabel = new JHyperLink(AppConstants.getWebSite());
        _updateInfoOnlyPanel = new javax.swing.JPanel();
        _updateInfoOnlyText = new javax.swing.JLabel();
        jButton1 = new javax.swing.JButton();
        _installer_download_link = new JHyperLink(SystemUtils.IS_OS_MAC ? "Download App Now" : "Download Installer Now");
        _installer_download_progress = new javax.swing.JProgressBar();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("TreeScan Update");
        setResizable(false);

        _cardsPanel.setLayout(new java.awt.CardLayout());

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Checking for updates ...");

        javax.swing.GroupLayout _checkingForUpdatePanelLayout = new javax.swing.GroupLayout(_checkingForUpdatePanel);
        _checkingForUpdatePanel.setLayout(_checkingForUpdatePanelLayout);
        _checkingForUpdatePanelLayout.setHorizontalGroup(
            _checkingForUpdatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_checkingForUpdatePanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, 379, Short.MAX_VALUE)
                .addContainerGap())
        );
        _checkingForUpdatePanelLayout.setVerticalGroup(
            _checkingForUpdatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_checkingForUpdatePanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, 102, Short.MAX_VALUE)
                .addContainerGap())
        );

        _cardsPanel.add(_checkingForUpdatePanel, "check");

        jLabel2.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        jLabel2.setText("You are running the most current version of TreeScan.");

        _noUpdateOkButton.setText("Ok");
        _noUpdateOkButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                ApplicationPreferences.setUpdateFrequency(_checkFrequency.getSelectedItem().toString());
                setVisible(false);
            }
        });

        jLabel3.setText("Automatically check for updates:");

        javax.swing.GroupLayout _noUpdatePanelLayout = new javax.swing.GroupLayout(_noUpdatePanel);
        _noUpdatePanel.setLayout(_noUpdatePanelLayout);
        _noUpdatePanelLayout.setHorizontalGroup(
            _noUpdatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_noUpdatePanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_noUpdatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel2, javax.swing.GroupLayout.DEFAULT_SIZE, 379, Short.MAX_VALUE)
                    .addComponent(jLabel3, javax.swing.GroupLayout.DEFAULT_SIZE, 379, Short.MAX_VALUE)
                    .addComponent(_checkFrequency, 0, 379, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _noUpdatePanelLayout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(_noUpdateOkButton)))
                .addContainerGap())
        );
        _noUpdatePanelLayout.setVerticalGroup(
            _noUpdatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_noUpdatePanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel3)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_checkFrequency, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_noUpdateOkButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _cardsPanel.add(_noUpdatePanel, "noUpdate");

        _updateLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _updateLabel.setText("TreeScan 1.1.0 is available. Do you want to install now?");

        _doUpdateButton.setText("Yes");
        _doUpdateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    if (UpdateCheckDialog.this._update_archivename == null) {
                        CardLayout cl = (CardLayout) (_cardsPanel.getLayout());
                        cl.show(_cardsPanel, CARD_DOWNLOAD);
                        new UpdaterDownloadTask().execute();
                    } else {
                        _runUpdateOnTerminate = true;
                        setVisible(false);
                    }
                } catch (Exception ex) {
                    throw new RuntimeException(ex.getMessage(), ex);
                }
            }
        });

        _doNotUpdateButton.setText("No");
        _doNotUpdateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisible(false);
            }
        });

        javax.swing.GroupLayout _updatePanelLayout = new javax.swing.GroupLayout(_updatePanel);
        _updatePanel.setLayout(_updatePanelLayout);
        _updatePanelLayout.setHorizontalGroup(
            _updatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_updatePanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_updatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_updateLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 379, Short.MAX_VALUE)
                    .addGroup(_updatePanelLayout.createSequentialGroup()
                        .addGap(97, 97, 97)
                        .addComponent(_doUpdateButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_doNotUpdateButton)))
                .addContainerGap())
        );
        _updatePanelLayout.setVerticalGroup(
            _updatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_updatePanelLayout.createSequentialGroup()
                .addGap(37, 37, 37)
                .addComponent(_updateLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_updatePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_doUpdateButton)
                    .addComponent(_doNotUpdateButton))
                .addContainerGap(44, Short.MAX_VALUE))
        );

        _cardsPanel.add(_updatePanel, "update");

        _stepLabel.setText("Contacting to host ...");

        _downloadProgressBar.setStringPainted(true);

        jButton3.setText("Cancel");

        javax.swing.GroupLayout _downloadPanelLayout = new javax.swing.GroupLayout(_downloadPanel);
        _downloadPanel.setLayout(_downloadPanelLayout);
        _downloadPanelLayout.setHorizontalGroup(
            _downloadPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_downloadPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_downloadPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_stepLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 379, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _downloadPanelLayout.createSequentialGroup()
                        .addComponent(_downloadProgressBar, javax.swing.GroupLayout.DEFAULT_SIZE, 308, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jButton3)))
                .addContainerGap())
        );
        _downloadPanelLayout.setVerticalGroup(
            _downloadPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _downloadPanelLayout.createSequentialGroup()
                .addContainerGap(46, Short.MAX_VALUE)
                .addComponent(_stepLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_downloadPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(jButton3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_downloadProgressBar, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(35, 35, 35))
        );

        _cardsPanel.add(_downloadPanel, "download");

        errorText.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        errorText.setVerticalTextPosition(javax.swing.SwingConstants.TOP);

        jLabel6.setText("Web site:");

        ((JHyperLink)webSiteLabel).addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                BareBonesBrowserLaunch.openURL(webSiteLabel.getText());
            }
        } );

        javax.swing.GroupLayout failedPanelLayout = new javax.swing.GroupLayout(failedPanel);
        failedPanel.setLayout(failedPanelLayout);
        failedPanelLayout.setHorizontalGroup(
            failedPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(failedPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(failedPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(errorText, javax.swing.GroupLayout.DEFAULT_SIZE, 0, Short.MAX_VALUE)
                    .addGroup(failedPanelLayout.createSequentialGroup()
                        .addComponent(jLabel6, javax.swing.GroupLayout.PREFERRED_SIZE, 54, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(webSiteLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 319, Short.MAX_VALUE)))
                .addContainerGap())
        );
        failedPanelLayout.setVerticalGroup(
            failedPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(failedPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(errorText, javax.swing.GroupLayout.DEFAULT_SIZE, 77, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(failedPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel6)
                    .addComponent(webSiteLabel))
                .addContainerGap())
        );

        _cardsPanel.add(failedPanel, "failed");

        _updateInfoOnlyText.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        _updateInfoOnlyText.setText("Newer version is available.");

        jButton1.setText("Ok");
        jButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisible(false);
            }
        });

        _installer_download_link.setText("Download Installer Now");
        _installer_download_link.setToolTipText("");
        _installer_download_link.setText(SystemUtils.IS_OS_MAC ? "Download App Now" : "Download Installer Now");
        ((JHyperLink)_installer_download_link).addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                try {
                    StringBuilder builder = new StringBuilder();
                    builder.append("%scgi-bin/treescan/register.pl/?todo=process_download&os=");
                    builder.append(SystemUtils.IS_OS_MAC ? "mac" : "win");
                    builder.append("&passwd=scantree&release-type=standard");
                    String baseurl = (TreeScanApplication.getDebugURL().length() > 0 ? TreeScanApplication.getDebugURL(): AppConstants.getWebSite());
                    URL url = URI.create(String.format(builder.toString(), baseurl).replace(" ", "%20")).toURL();
                    _download_file_info = new FileInfo(getFile(_new_update_version + ".tmp"), url);
                    InstallerDownloadTask downloader = new InstallerDownloadTask();
                    downloader.addPropertyChangeListener((PropertyChangeEvent evt) -> {
                        if ("progress".equals(evt.getPropertyName())) {
                            _installer_download_progress.setValue((Integer)evt.getNewValue());
                        }
                    });
                    downloader.execute();
                } catch (IOException ex) {
                    Logger.getLogger(UpdateCheckDialog.class.getName()).log(Level.WARNING, null, ex);
                    JOptionPane.showMessageDialog(UpdateCheckDialog.this, "Windows installer download failed.\nPlease visit website for latest release.", "Operation Failed", JOptionPane.WARNING_MESSAGE);
                }
            }
        } );

        _installer_download_progress.setStringPainted(true);

        javax.swing.GroupLayout _updateInfoOnlyPanelLayout = new javax.swing.GroupLayout(_updateInfoOnlyPanel);
        _updateInfoOnlyPanel.setLayout(_updateInfoOnlyPanelLayout);
        _updateInfoOnlyPanelLayout.setHorizontalGroup(
            _updateInfoOnlyPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_updateInfoOnlyPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_updateInfoOnlyPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_updateInfoOnlyPanelLayout.createSequentialGroup()
                        .addComponent(_updateInfoOnlyText, javax.swing.GroupLayout.DEFAULT_SIZE, 330, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jButton1))
                    .addComponent(_installer_download_progress, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_installer_download_link, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _updateInfoOnlyPanelLayout.setVerticalGroup(
            _updateInfoOnlyPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_updateInfoOnlyPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_updateInfoOnlyPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_updateInfoOnlyText, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jButton1))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_installer_download_link, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(_installer_download_progress, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(32, 32, 32))
        );

        _cardsPanel.add(_updateInfoOnlyPanel, "updateinfoonly");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_cardsPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_cardsPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    /* Download file information. */
    public class FileInfo {
        final File _file;
        final URL _url;

        FileInfo(File file, URL url) {
            _file = file;
            _url = url;
        }
    };
    
    class CheckUpdateTask extends SwingWorker<Void, Void> {

        private final String _URLFormat = "%scgi-bin/treescan/update/treescan_version_update.pl?todo=return_update_version_info";
        private final int _updateTokenCount = 9;
        private final int _updateVersionIdIndex = 3;
        private final int _updateVersionIndex = 4;
        private final int _updateAppNameIndex = 5;
        private final int _updateAppUrlIndex = 6;
        private final int _updateDataNameIndex = 7;
        private final int _updateDataUrlIndex = 8;
        private final WaitCursor waitCursor = new WaitCursor(UpdateCheckDialog.this);
        private final String _get_params = ("&form_current_version_id=" + AppConstants.getVersionId() +
                              "&form_current_version_number=" + AppConstants.getVersion() +
                              "&os=" + System.getProperty("os.name") +
                              "&java.vm.version=" + System.getProperty("java.vm.version") +
                              "&java.runtime.version=" + System.getProperty("java.runtime.version") +
                              "&java.specification.version=" + System.getProperty("java.specification.version") +
                              "&java.vm.vendor=" + System.getProperty("java.vm.vendor") +
                              "&os.arch=" + System.getProperty("os.arch") +
                              "&os.arch.data.model=" + System.getProperty("os.arch.data.model")).replace(" ", "%20");

        protected void readUpdateInfo(HttpURLConnection http) throws IOException {
            /*Input stream to read from our connection*/
            InputStream is = http.getInputStream();
            BufferedReader in = new BufferedReader(new InputStreamReader(is));
            String str = in.readLine();
            System.out.println(str);
            String[] httpBody = str.split(",");
            if (httpBody.length < _updateTokenCount || httpBody[0].equals("no")) {
                _updateExists = false;
            } else if (Integer.parseInt(httpBody[_updateVersionIdIndex]) > Integer.parseInt(AppConstants.getVersionId())) {
                _updateExists = true;
                //get update information
                _new_update_version = httpBody[_updateVersionIndex];
                _update_application_info = new FileInfo(getFile(httpBody[_updateAppNameIndex]), new URL(httpBody[_updateAppUrlIndex] + _get_params));
                String updateArchiveUrl = httpBody[_updateDataUrlIndex];
                if (updateArchiveUrl.endsWith("\n")) {
                    updateArchiveUrl = updateArchiveUrl.split("\n")[0];
                }
                _update_archive_info = new FileInfo(getFile(httpBody[_updateDataNameIndex]), new URL(updateArchiveUrl + _get_params));
            }
        }

        @Override
        protected Void doInBackground() throws Exception {
            try {
                String baseurl = (TreeScanApplication.getDebugURL().length() > 0 ? TreeScanApplication.getDebugURL(): AppConstants.getWebSite());
                String updateURL = String.format(_URLFormat, baseurl);
                updateURL = updateURL.replace(" ", "%20") + _get_params;
                readUpdateInfo((HttpURLConnection)getHttpURLConnection(new URL(updateURL), false));
            } catch (FileNotFoundException e) {
                _error_code = HttpURLConnection.HTTP_GONE;
                System.out.println(e.getMessage());
            } catch (java.net.UnknownHostException e) {
                _error_code = HttpURLConnection.HTTP_NOT_FOUND;
                System.out.println(e.getMessage());
            } catch (IOException e) {
                _error_code = 1;
                System.out.println(e.getMessage());
            }
            return null;
        }

        @Override
        public void done() {
            waitCursor.restore();
            try { // Show panel card given update status.
                _applicationFrame.softwareUpdateAvailable.setVisible(_updateExists);
                _applicationFrame._versionUpdateToolButton.setVisible(!_updateExists);
                if (_updateExists) {
                    if (SystemUtils.IS_OS_WINDOWS || SystemUtils.IS_OS_MAC) {
                        _installer_download_link.setVisible(true);
                        _updateInfoOnlyText.setText("Newer version " + _new_update_version + " is available.");
                        ((CardLayout)_cardsPanel.getLayout()).show(_cardsPanel, CARD_UPDATEINFOONLY);
                    } else {
                        _updateLabel.setText("TreeScan " + _new_update_version + " is available. Do you want to install now?");
                        ((CardLayout)_cardsPanel.getLayout()).show(_cardsPanel, CARD_UPDATE);
                    }
                } else if (_error_code > 0){
                    errorText.setText(getErrorText(_error_code));
                    ((CardLayout)_cardsPanel.getLayout()).show(_cardsPanel, CARD_FAILED);
                } else {
                    ((CardLayout)_cardsPanel.getLayout()).show(_cardsPanel, CARD_NOUPDATE);
                }
            } catch (Exception e) {
                throw new RuntimeException(e.getMessage(), e);
            }
        }
    }

    class UpdaterDownloadTask extends SwingWorker<Void, Void> {
        /* Worker task which downloads application update application and update zip file. 
           Once download is complete, marks trigger which signals we need to restart.
           This feature is only available on Linux at this point since the updater 
           application is a Java program -- we're bundling Java w/ Windows and Mac. */
        
        private final int _readBufferSize = 4096;
        private final WaitCursor waitCursor = new WaitCursor(UpdateCheckDialog.this);

        private void downloadFile(FileInfo info, int index) {
            try {
                // Open a connection
                HttpURLConnection connection = (HttpURLConnection)getHttpURLConnection(info._url, true);
                // Read from the connection
                int contentSize = connection.getContentLength();
                _downloadProgressBar.setValue(0);
                _downloadProgressBar.setMaximum(contentSize);
                _stepLabel.setText(String.format("Downloading file %d of %d (%d bytes) ...", index + 1, 2, contentSize));
                byte[] b = new byte[_readBufferSize];
                InputStream input = connection.getInputStream();
                FileOutputStream out = new FileOutputStream(info._file);
                int count = input.read(b);
                while (count != -1) {
                    _downloadProgressBar.setValue(_downloadProgressBar.getValue() + count);
                    out.write(b, 0, count);
                    count = input.read(b);
                }
                out.close();
                _downloadProgressBar.setValue(_downloadProgressBar.getMaximum());
            } catch (Exception e) {
                System.out.println(e.getMessage());
                throw new RuntimeException(e.getMessage(), e);
            }
        }

        @Override
        protected Void doInBackground() throws Exception {
            try {
                if (_update_application_info != null && _update_archive_info != null) {
                    downloadFile(_update_application_info, 0);
                    _updater_filename = _update_application_info._file;
                    downloadFile(_update_archive_info, 1);
                    _download_completed = true;
                    _restart_required = true;
                }
            } catch (Exception e) {
                System.out.println(e.getMessage());
                throw new RuntimeException(e.getMessage(), e);
            }

            return null;
        }

        @Override
        public void done() {
            if (_download_completed) {
                _update_archivename = _update_archive_info._file;
                _runUpdateOnTerminate = true;
            }
            waitCursor.restore();
            setVisible(false); //message about restart!!!???
        }
    }
    class InstallerDownloadTask extends SwingWorker<Void, Void> {
        private final int _readBufferSize = 4096;
        private final WaitCursor waitCursor = new WaitCursor(UpdateCheckDialog.this);
        private String _filename = SystemUtils.IS_OS_MAC ? ("TreeScan_" + _new_update_version + "_mac.zip") : ("install-TreeScan" + _new_update_version + ".exe");
        private void downloadFile(FileInfo info) {                        
            try {
                // Open a connection
                System.out.println(info._url);
                HttpURLConnection connection = (HttpURLConnection)getHttpURLConnection(info._url, true);
                int contentSize = connection.getContentLength();
                String contentDisposition = connection.getHeaderField("Content-Disposition");
                if (contentDisposition != null) {
                    Matcher matcher = Pattern.compile(".+filename=\"?(.+)\"?").matcher(contentDisposition);
                    if (matcher.find()) 
                        _filename = matcher.group(1);
                }                
                byte[] b = new byte[_readBufferSize];
                InputStream input = connection.getInputStream();
                FileOutputStream out = new FileOutputStream(info._file);
                int count = input.read(b);
                int read = count;
                while (count != -1) {
                    setProgress((int)(100.0 * ((double)read / (double)contentSize)));
                    out.write(b, 0, count);
                    count = input.read(b);
                    read += count;
                }
                out.close();
                setProgress(100);
            } catch (IOException e) {
                System.out.println(e.getMessage());
                throw new RuntimeException(e.getMessage(), e);
            }
        }
        @Override
        protected Void doInBackground() throws Exception {
            try {
                _installer_download_progress.setVisible(true);
                _installer_download_progress.setString(null);
                _installer_download_link.setEnabled(false);
                downloadFile(_download_file_info);
                FileUtils.copyFile(_download_file_info._file, 
                    new File(getUserDownloadDirectory() + File.separator + _filename),
                    false, StandardCopyOption.REPLACE_EXISTING
                );
            } catch (IOException e) {
                System.out.println(e.getMessage());
                throw new RuntimeException(e.getMessage(), e);
            }
            return null;
        }
        @Override
        public void done() {
            waitCursor.restore();
            _installer_download_link.setEnabled(true);
            _installer_download_progress.setString("Completed to 'Downloads' directory.");
        }
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _cardsPanel;
    private javax.swing.JComboBox _checkFrequency;
    private javax.swing.JPanel _checkingForUpdatePanel;
    private javax.swing.JButton _doNotUpdateButton;
    private javax.swing.JButton _doUpdateButton;
    private javax.swing.JPanel _downloadPanel;
    private javax.swing.JProgressBar _downloadProgressBar;
    private javax.swing.JLabel _installer_download_link;
    private javax.swing.JProgressBar _installer_download_progress;
    private javax.swing.JButton _noUpdateOkButton;
    private javax.swing.JPanel _noUpdatePanel;
    private javax.swing.JLabel _stepLabel;
    private javax.swing.JPanel _updateInfoOnlyPanel;
    private javax.swing.JLabel _updateInfoOnlyText;
    private javax.swing.JLabel _updateLabel;
    private javax.swing.JPanel _updatePanel;
    private javax.swing.JLabel errorText;
    private javax.swing.JPanel failedPanel;
    private javax.swing.JButton jButton1;
    private javax.swing.JButton jButton3;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel webSiteLabel;
    // End of variables declaration//GEN-END:variables

}
