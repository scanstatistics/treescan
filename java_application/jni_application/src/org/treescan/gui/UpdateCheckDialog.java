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
import java.net.URLConnection;
import java.util.prefs.Preferences;
import javax.swing.SwingWorker;
import org.treescan.gui.utils.WaitCursor;
import org.treescan.app.AppConstants;
import org.treescan.utils.BareBonesBrowserLaunch;
import org.treescan.gui.utils.JHyperLink;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.FileNotFoundException;
import org.apache.commons.lang3.SystemUtils;
import org.apache.xmlbeans.impl.util.Base64;

/** @author  Hostovic */
public class UpdateCheckDialog extends javax.swing.JDialog {
    private Preferences _prefs = Preferences.userNodeForPackage(getClass());
    private final TreeScanApplication _applicationFrame;

    private boolean _updateExists = false;
    private int _error_code = 0;
    private FileInfo _updateApplication = null;
    private FileInfo _updateArchive = null;
    private String _updateVersion = "";
    private boolean _download_completed = false;
    private boolean _restartRequired = false;

    private static String CARD_CHECK = "check";
    private static String CARD_UPDATE = "update";
    private static String CARD_NOUPDATE = "noUpdate";
    private static String CARD_DOWNLOAD = "download";
    private static String CARD_FAILED = "failed";
    private static String CARD_UPDATEINFOONLY = "updateinfoonly";

    public static boolean _runUpdateOnTerminate = false;
    public static File _updaterFilename = null;
    public static File _updateArchiveName = null;

    /** Creates new form UpdateCheckDialogs */
    public UpdateCheckDialog(TreeScanApplication applicationFrame) {
        super(applicationFrame, true);
        initComponents();
        _applicationFrame = applicationFrame;
        _checkFrequency.setModel(new javax.swing.DefaultComboBoxModel(ApplicationPreferences.updateFrequencyChoices()));
        _checkFrequency.setSelectedItem(ApplicationPreferences.getUpdateFrequency());
        setLocationRelativeTo(applicationFrame);
        if (ApplicationPreferences.shouldCheckUpdate()) {
            new CheckUpdateTask().execute();
        }
    }

    /** Returns whether the application has to restart for update. */
    public boolean restartRequired() {
        return _restartRequired;
    }

    public static File getDownloadTempDirectory() {
        return new File(System.getProperty("java.io.tmpdir"));
    }

    static public String getErrorText(int errorcode) {
        StringBuilder text = new StringBuilder();
        text.append("<html>Unable to check for update. This might be because the update site is temporarily unavailable or your internet connection is down. ");
        text.append("If this problem persists, you can always download the latest version from the web site directly.");
        if (errorcode > 0) {
            text.append(" (code=").append(errorcode).append(")");
        }
        text.append("</html>");
        return text.toString();
    }

    /**
     * Overrides setVisible() method
     */
    @Override
    public void setVisible(boolean show) {
        if (show) {
            if (!_updateExists) {
                connectToServerForUpdateCheck();
            } else {
                CardLayout cl = (CardLayout) (_cardsPanel.getLayout());
                cl.show(_cardsPanel, CARD_UPDATE);
            }
        }
        super.setVisible(show);
    }

    /**
     * This function connects to the server to request the update information.
     */
    private void connectToServerForUpdateCheck() {
        try {
            CardLayout cl = (CardLayout) (_cardsPanel.getLayout());
            cl.show(_cardsPanel, CARD_CHECK);
            new CheckUpdateTask().execute();
        } catch (Throwable t) {
            throw new RuntimeException(t.getMessage(), t);
        }
    }

    /**
     * Show panel card given update status.
     */
    private void showStatus() {
        try {
            /* Application updater is not working currently -- only inform user about update. */
            if (SystemUtils.IS_OS_WINDOWS) {
                _applicationFrame.softwareUpdateAvailable.setVisible(false);
                _applicationFrame._versionUpdateToolButton.setVisible(true);
            } else {
                _applicationFrame.softwareUpdateAvailable.setVisible(_updateExists);
                _applicationFrame._versionUpdateToolButton.setVisible(!_updateExists);
            }
            CardLayout cl = (CardLayout) (_cardsPanel.getLayout());
            if (_updateExists) {
                if (SystemUtils.IS_OS_WINDOWS) {
                    cl.show(_cardsPanel, CARD_UPDATEINFOONLY);
                    _updateInfoOnlyText.setText("Newer version " + getNewVersionNumber() + " is available at:");
                } else {
                    _updateLabel.setText("TreeScan " + getNewVersionNumber() + " is available. Do you want to install now?");
                    cl.show(_cardsPanel, CARD_UPDATE);
                }
            } else if (_error_code > 0){
                errorText.setText(getErrorText(_error_code));
                cl.show(_cardsPanel, CARD_FAILED);
            } else {
                cl.show(_cardsPanel, CARD_NOUPDATE);
            }
        } catch (Exception e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /** downloads files */
    private void downloadFiles() {
        try {
            if (this._updateArchiveName == null) {
                CardLayout cl = (CardLayout) (_cardsPanel.getLayout());
                cl.show(_cardsPanel, CARD_DOWNLOAD);
                new DownloadTask().execute();
            } else {
 	 	_runUpdateOnTerminate = true;
 	 	setVisible(false);
            }
        } catch (Exception e) {
            throw new RuntimeException(e.getMessage(), e);
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
        webSiteLabel1 = new JHyperLink(AppConstants.getWebSite());

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
                .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, 99, Short.MAX_VALUE)
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
                downloadFiles();
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
                .addContainerGap(41, Short.MAX_VALUE))
        );

        _cardsPanel.add(_updatePanel, "update");

        _stepLabel.setText("Contacting to host ...");

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
                .addContainerGap(43, Short.MAX_VALUE)
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
                .addComponent(errorText, javax.swing.GroupLayout.DEFAULT_SIZE, 74, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(failedPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel6)
                    .addComponent(webSiteLabel))
                .addContainerGap())
        );

        _cardsPanel.add(failedPanel, "failed");

        _updateInfoOnlyText.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        _updateInfoOnlyText.setText("Newer version 1.1.0 is available at:");

        jButton1.setText("Ok");
        jButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisible(false);
            }
        });

        ((JHyperLink)webSiteLabel1).addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                BareBonesBrowserLaunch.openURL(webSiteLabel1.getText());
            }
        } );

        javax.swing.GroupLayout _updateInfoOnlyPanelLayout = new javax.swing.GroupLayout(_updateInfoOnlyPanel);
        _updateInfoOnlyPanel.setLayout(_updateInfoOnlyPanelLayout);
        _updateInfoOnlyPanelLayout.setHorizontalGroup(
            _updateInfoOnlyPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_updateInfoOnlyPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_updateInfoOnlyPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_updateInfoOnlyText, javax.swing.GroupLayout.DEFAULT_SIZE, 379, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _updateInfoOnlyPanelLayout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(jButton1))
                    .addComponent(webSiteLabel1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _updateInfoOnlyPanelLayout.setVerticalGroup(
            _updateInfoOnlyPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_updateInfoOnlyPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_updateInfoOnlyText, javax.swing.GroupLayout.PREFERRED_SIZE, 26, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(webSiteLabel1)
                .addGap(30, 30, 30)
                .addComponent(jButton1)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _cardsPanel.add(_updateInfoOnlyPanel, "updateinfoonly");
        _updateInfoOnlyPanel.getAccessibleContext().setAccessibleName("");

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
    /** returns full path of download file */
    private File getFile(String fileName) {
        try {
            File file = File.createTempFile("TreeScan", fileName, getDownloadTempDirectory());
            return file;
        } catch (IOException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /**
     * Returns version number of update.
     */
    public String getNewVersionNumber() {
        return _updateVersion;
    }

    /**
     * Return indication of whether updates exist.
     */
    public boolean hasUpdate() {
        return _updateExists;
    }

    class CheckUpdateTask extends SwingWorker<Void, Void> {

        private final static String _URLFormat = "%scgi-bin/treescan/update/treescan_version_update.pl?todo=return_update_version_info";
        private final static int _updateTokenCount = 9;
        private final static int _updateIndicatorIndex = 0;
        private final static int _updateVersionIdIndex = 3;
        private final static int _updateVersionIndex = 4;
        private final static int _updateAppNameIndex = 5;
        private final static int _updateAppUrlIndex = 6;
        private final static int _updateDataNameIndex = 7;
        private final static int _updateDataUrlIndex = 8;
        private WaitCursor waitCursor = new WaitCursor(UpdateCheckDialog.this);
        private String _get_params = ("&form_current_version_id=" + AppConstants.getVersionId() +
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
                _updateVersion = httpBody[_updateVersionIndex];
                _updateApplication = new FileInfo(getFile(httpBody[_updateAppNameIndex]), new URL(httpBody[_updateAppUrlIndex] + _get_params));
                String updateArchiveUrl = httpBody[_updateDataUrlIndex];
                if (updateArchiveUrl.endsWith("\n")) {
                    updateArchiveUrl = updateArchiveUrl.split("\n")[0];
                }
                _updateArchive = new FileInfo(getFile(httpBody[_updateDataNameIndex]), new URL(updateArchiveUrl + _get_params));
            }
        }

        protected Void doInBackground() throws Exception {
            HttpURLConnection connection=null;
            try {
                String baseurl = (TreeScanApplication.getDebugURL().length() > 0 ? TreeScanApplication.getDebugURL(): AppConstants.getWebSite());
                String updateURL = String.format(_URLFormat, baseurl);
                updateURL = updateURL.replace(" ", "%20") + _get_params;
                boolean completed=false;
                do {
                    URL u = new URL(updateURL);
                    connection = (HttpURLConnection)u.openConnection();
                    connection.setFollowRedirects(true);
                    if (TreeScanApplication.getDebugAuth().length() > 0) {
                        byte[] bytesEncoded = Base64.encode(TreeScanApplication.getDebugAuth().getBytes());
                        String authEncoded = new String(bytesEncoded);
                        connection.setRequestProperty("Authorization", "Basic "+authEncoded);
                    }
                    /*GET will be our method to download a file*/
                    connection.setRequestMethod("GET");
                    /*Stablishing the connection*/
                    connection.connect();
                    /*The served agreed to send us the file*/
                    switch (connection.getResponseCode()) {
                        case HttpURLConnection.HTTP_OK:
                            readUpdateInfo(connection);
                            completed = true;
                            break;
                        case HttpURLConnection.HTTP_MOVED_PERM:
                        case HttpURLConnection.HTTP_MOVED_TEMP:
                        case HttpURLConnection.HTTP_SEE_OTHER:
                            updateURL = connection.getHeaderField("Location");
                            break;
                        default: _error_code = connection.getResponseCode(); completed = true;
                    }
                    connection.disconnect();
                } while(!completed);
            } catch (FileNotFoundException e) {
                _error_code = HttpURLConnection.HTTP_GONE;
                System.out.println(e.getMessage());
            } catch (java.net.UnknownHostException e) {
                _error_code = HttpURLConnection.HTTP_NOT_FOUND;
                System.out.println(e.getMessage());
            } catch (IOException e) {
                _error_code = 1;
                System.out.println(e.getMessage());
            } finally {
                if (connection != null) connection.disconnect();
            }
            return null;
        }

        @Override
        public void done() {
            waitCursor.restore();
            showStatus();
        }
    }

    class DownloadTask extends SwingWorker<Void, Void> {

        private final static int _readBufferSize = 4096;
        private WaitCursor waitCursor = new WaitCursor(UpdateCheckDialog.this);

        private void downloadFile(FileInfo info, int index) {
            try {
                File f = info._file;
                URL url = info._url;
                // Open a connection
                URLConnection connection = url.openConnection();
                if (TreeScanApplication.getDebugAuth().length() > 0) {
                    byte[] bytesEncoded = Base64.encode(TreeScanApplication.getDebugAuth().getBytes());
                    String authEncoded = new String(bytesEncoded);
                    connection.setRequestProperty("Authorization", "Basic "+authEncoded);
                }
                connection.setDoOutput(true);
                // Read from the connection
                _downloadProgressBar.setValue(0);
                int contentSize = connection.getContentLength();
                _downloadProgressBar.setMaximum(contentSize);
                _stepLabel.setText(String.format("Downloading file %d of %d (%d bytes) ...", index + 1, 2, contentSize));
                byte[] b = new byte[_readBufferSize];
                InputStream input = connection.getInputStream();
                FileOutputStream out = new FileOutputStream(f);
                int count = input.read(b);
                while (count != -1) {
                    _downloadProgressBar.setValue(_downloadProgressBar.getValue() + count);
                    out.write(b, 0, count);
                    count = input.read(b);
                }
                out.close();
            } catch (Exception e) {
                System.out.println(e.getMessage());
                throw new RuntimeException(e.getMessage(), e);
            }
        }

        protected Void doInBackground() throws Exception {
            try {
                if (_updateApplication != null && _updateArchive != null) {
                    downloadFile(_updateApplication, 0);
                    _updaterFilename = _updateApplication._file;
                    downloadFile(_updateArchive, 1);
                    _download_completed = true;
                    _restartRequired = true;
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
                _updateArchiveName = _updateArchive._file;
                _runUpdateOnTerminate = true;
            }
            waitCursor.restore();
            setVisible(false); //message about restart!!!???
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
    private javax.swing.JLabel webSiteLabel1;
    // End of variables declaration//GEN-END:variables
    /**
     * Download file information.
     */
    public class FileInfo {

        final File _file;
        final URL _url;

        FileInfo(File file, URL url) {
            _file = file;
            _url = url;
        }
    }




;
}
