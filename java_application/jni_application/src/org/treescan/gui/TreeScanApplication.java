package org.treescan.gui;

import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.Transferable;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowFocusListener;
import java.awt.event.WindowListener;
import java.beans.PropertyVetoException;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.Locale;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.help.HelpBroker;
import javax.help.HelpSet;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JFileChooser;
import javax.swing.JInternalFrame;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.UIManager;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import org.treescan.app.AppConstants;
import org.treescan.utils.BareBonesBrowserLaunch;
import org.treescan.utils.FileAccess;
import org.treescan.app.OutputFileRegister;
import org.treescan.app.ParameterHistory;
import org.treescan.app.Parameters;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.gui.utils.WaitCursor;
import org.treescan.gui.utils.WindowsMenu;
import ca.guydavis.swing.desktop.CascadingWindowPositioner;
import java.awt.Desktop;
import java.awt.datatransfer.ClipboardOwner;
import java.awt.event.KeyEvent;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

/*
 * TreeScanApplication.java
 *
 * Created on December 5, 2007, 11:14 AM
 */
import javax.help.SwingHelpUtilities;
import javax.swing.KeyStroke;
import org.treescan.gui.utils.MacOSApplication;

/**
 *
 * @author  Hostovic
 */
public class TreeScanApplication extends javax.swing.JFrame implements WindowFocusListener, WindowListener, InternalFrameListener, ClipboardOwner {

    private static final String _application = System.getProperty("user.dir") + System.getProperty("file.separator") + "TreeScan.jar";
    private static final String _user_guide = System.getProperty("user.dir") + System.getProperty("file.separator") + "TreeScan_Users_Guide.pdf";
    private static Boolean _debug_url = new Boolean(false);
    private static String _run_args[] = new String[]{};
    private static final long serialVersionUID = 1L;
    private final ExecuteSessionAction _executeSessionAction;
    private final ExecuteOptionsAction _executeOptionsAction;
    private final CloseSessionAction _closeSessionAction;
    private final SaveSessionAction _saveSessionAction;
    private final SaveSessionAsAction _saveSessionAsAction;
    private final PrintResultsAction _printResultsAction;
    private JInternalFrame _focusedInternalFrame = null;
    private boolean gbShowStartWindow = true;
    private Vector<JInternalFrame> allOpenFrames = new Vector<JInternalFrame>();
    private static TreeScanApplication _instance;
    public File lastBrowseDirectory = new File(System.getProperty("user.dir"));
    WindowsMenu windowsMenu = null;
    private final String HEIGHT_KEY = "height";
    private final String WIDTH_KEY = "width";
    private final String DEFAULT_HEIGHT = "768";
    private final String DEFAULT_WIDTH = "1024";
    private static String RELAUNCH_ARGS_OPTION = "relaunch_args=";
    private static String RELAUNCH_TOKEN = "&";
    private MacOSApplication _mac_os_app = new MacOSApplication();

    /**
     * Creates new form TreeScanApplication
     */
    public TreeScanApplication() {
        _instance = this;
        System.out.println(System.getProperties());
        _executeSessionAction = new ExecuteSessionAction();
        _executeOptionsAction = new ExecuteOptionsAction();
        _closeSessionAction = new CloseSessionAction();
        _saveSessionAction = new SaveSessionAction();
        _saveSessionAsAction = new SaveSessionAsAction();
        _printResultsAction = new PrintResultsAction();
        initComponents();
        Preferences _prefs = Preferences.userNodeForPackage(TreeScanApplication.class);
        setSize(Integer.parseInt(_prefs.get(WIDTH_KEY, DEFAULT_WIDTH)), Integer.parseInt(_prefs.get(HEIGHT_KEY, DEFAULT_HEIGHT)));
        windowsMenu = new WindowsMenu(this.desktopPane);
        windowsMenu.setWindowPositioner(new CascadingWindowPositioner(this.desktopPane));
        windowsMenu.setMnemonic(KeyEvent.VK_W);
        menuBar.add(windowsMenu, 2);
        setTitle(AppConstants.getSoftwareTitle());
        setIconImage(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/TreeScan.png")));
        enableActions(false, false);
        addWindowFocusListener(this);
        addWindowListener(this);
        refreshOpenList();
        setLocationRelativeTo(null);
    }

    public static TreeScanApplication getInstance() {
        return _instance;
    }

    /**
     * Loads shared object libray.
     * @param is64bitEnabled
     */
    public static void loadSharedLibray() {
        if (System.getProperty("os.name").toLowerCase().startsWith("mac")) {
          // JNI library combined into one universal binary on Mac
            System.out.println("Loading libtreescan");
            System.loadLibrary("treescan");
        } else {
          // other platforms require checking to determine whether VM is 32 or 64 bit
          // in order to load appropriate JNI library
        
          boolean is64BitVM = false;
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

          // educated guess has been determined 
          String firstTryLibrary = is64BitVM ? "treescan64" : "treescan32";
          String secondTryLibrary = is64BitVM ? "treescan32" : "treescan64";
          try {
            System.out.println("Loading " + firstTryLibrary);
            System.loadLibrary(firstTryLibrary);
          } catch (Throwable t) { // last-ditch effort                
            System.out.println("Loading " + secondTryLibrary);
            System.loadLibrary(secondTryLibrary);
          } 
        }
    }

    /**
     * Stores run arguments for application update.
     * @param args -- run parameters
     */
    public static void setRunArgs(final String args[]) {
        _run_args = args;
    }

    public static final String getRelaunchArgs() {
        StringBuilder args = new StringBuilder();
        if (_run_args.length > 0) {
            args.append(RELAUNCH_ARGS_OPTION);
        }
        for (int i = 0; i < _run_args.length; ++i) {
            args.append(_run_args[i]).append(RELAUNCH_TOKEN);
        }
        return args.toString();
    }

    /**
     * Sets flag for debug application update.
     * @param is64bitEnabled
     */
    public static void setDebugURL(boolean debugURL) {
        _debug_url = new Boolean(debugURL);
    }

    public static Boolean getDebugURL() {
        return _debug_url;
    }

    /**
     * Open new session action.
     */
    public class NewSessionFileAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public NewSessionFileAction() {
            super("New Session");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                openNewParameterSessionWindow("");
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Create a new ParameterSettingsFrame internal frame.
     */
    protected void openNewParameterSessionWindow(String sFilename) {
        WaitCursor waitCursor = new WaitCursor(TreeScanApplication.this);

        try {
            ParameterSettingsFrame frame = new ParameterSettingsFrame(getRootPane(), sFilename);
            frame.addInternalFrameListener(this);
            frame.setVisible(true);
            desktopPane.add(frame);
            try {
                frame.setSelected(true);
            } catch (java.beans.PropertyVetoException e) {
            }
        } finally {
            waitCursor.restore();
        }
    }

    /**
     * Opens an existing session from file.
     */
    public class OpenSessionFileAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public OpenSessionFileAction() {
            super("Open Session File");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                openParameterSessionWindow();
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Create a new ParameterSettingsFrame internal frame, loading control
     * settings from file.
     */
    private void openParameterSessionWindow() {
        //Create a file chooser
        JFileChooser fc = new JFileChooser(lastBrowseDirectory);
        fc.setDialogTitle("Select Parameter File");
        fc.addChoosableFileFilter(new InputFileFilter("txt", "Text Files (*.txt)"));
        fc.addChoosableFileFilter(new InputFileFilter("prm", "Parameter Files (*.prm)"));
        int returnVal = fc.showOpenDialog(TreeScanApplication.this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            lastBrowseDirectory = fc.getCurrentDirectory();
            openNewParameterSessionWindow(fc.getSelectedFile().getAbsolutePath());
        }
    }

    /**
     * Save session action; calls ParameterSettingsFrame::WriteSession to
     * write settings to file.
     */
    public class SaveSessionAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public SaveSessionAction() {
            super("Save Session");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                if (_focusedInternalFrame != null && _focusedInternalFrame instanceof ParameterSettingsFrame) {
                    ((ParameterSettingsFrame) _focusedInternalFrame).WriteSession("");
                }
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Save As session action; calls ParameterSettingsFrame::WriteSession to
     * write settings to specified file.
     */
    public class SaveSessionAsAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public SaveSessionAsAction() {
            super("Save Session As");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                if (_focusedInternalFrame != null && _focusedInternalFrame instanceof ParameterSettingsFrame) {
                    ((ParameterSettingsFrame) _focusedInternalFrame).SaveAs();
                }
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Close session action; attmepts to close active session window.
     */
    public class CloseSessionAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public CloseSessionAction() {
            super("Close Session");
        }

        public void actionPerformed(ActionEvent e) {
            if (_focusedInternalFrame != null && _focusedInternalFrame instanceof ParameterSettingsFrame) {
                try {
                    _focusedInternalFrame.setClosed(true);
                } catch (PropertyVetoException e1) {
                }
            }
        }
    }

    /**
     * Print results action; attempts to print results from run window.
     */
    public class PrintResultsAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public PrintResultsAction() {
            super("Print");

        }

        public void actionPerformed(ActionEvent e) {
            try {
                if (_focusedInternalFrame instanceof AnalysisRunInternalFrame) {
                    ((AnalysisRunInternalFrame) _focusedInternalFrame).printWindow();
                }
            //JOptionPane.showMessageDialog(TreeScanApplication.this, "PrintResultsAsAction::actionPerformed() not implemented.", "Note", JOptionPane.INFORMATION_MESSAGE);
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Application exist action; closes application.
     */
    public class ExitAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public ExitAction() {
            super("Exit");
        }

        public void actionPerformed(ActionEvent e) {
            windowClosing(new WindowEvent(TreeScanApplication.this, WindowEvent.WINDOW_CLOSING));
        }
    }

    /**
     * Session execute action; attmepts to start execution of specified
     * analysis settings.
     */
    public class ExecuteSessionAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public ExecuteSessionAction() {
            super("Execute");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                executeAnalysis();
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Executes analysis from focused session window.
     */
    protected void executeAnalysis() {
        if (_focusedInternalFrame != null && _focusedInternalFrame instanceof ParameterSettingsFrame) {
            if (((ParameterSettingsFrame) _focusedInternalFrame).ValidateParams()) {
                Parameters parameters = ((ParameterSettingsFrame) _focusedInternalFrame).getParameterSettings();
                if (OutputFileRegister.getInstance().isRegistered(parameters.getOutputFileName())) {
                    JOptionPane.showMessageDialog(TreeScanApplication.this, "The results file for this analysis is currently being written.\n" +
                            "Please specify another filename or wait for analysis to complete.", "Note", JOptionPane.INFORMATION_MESSAGE);
                } else {
                    AnalysisRunInternalFrame frame = new AnalysisRunInternalFrame(parameters);
                    OutputFileRegister.getInstance().register(parameters.getOutputFileName());
                    frame.addInternalFrameListener(this);
                    frame.setVisible(true);
                    desktopPane.add(frame);
                    try {
                        frame.setSelected(true);
                    } catch (java.beans.PropertyVetoException e) {
                    }

                }
            }
        }
    }

    /**
     * Execute options action; displays the execution options dialog.
     */
    public class ExecuteOptionsAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public ExecuteOptionsAction() {
            super("Execute Options");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                if (_focusedInternalFrame instanceof ParameterSettingsFrame) {
                    ((ParameterSettingsFrame) _focusedInternalFrame).showExecOptionsDialog(TreeScanApplication.this);
                }
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Help system action, launches the help system.
     */
    public class HelpSystemAction extends AbstractAction {

        static final String helpsetName = "TreeScan_Help";
        private static final long serialVersionUID = 1L;

        public HelpSystemAction() {
            super("Help System");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                SwingHelpUtilities.setContentViewerUI("org.treescan.gui.utils.ExternalLinkContentViewerUI");
                ClassLoader cl = TreeScanApplication.class.getClassLoader();
                URL url = HelpSet.findHelpSet(cl, helpsetName, "", Locale.getDefault());
                if (url == null) {
                    url = HelpSet.findHelpSet(cl, helpsetName, ".hs", Locale.getDefault());
                    if (url == null) {
                        JOptionPane.showMessageDialog(null, "The help system could not be located.", " Help", JOptionPane.WARNING_MESSAGE);
                        return;
                    }
                }
                HelpSet mainHS = new HelpSet(cl, url);
                HelpBroker mainHB = mainHS.createHelpBroker();
                mainHB.setDisplayed(true);
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }

        //JOptionPane.showMessageDialog(TreeScanApplication.this, "HelpSystemAction::actionPerformed() not implemented.", "Note", JOptionPane.INFORMATION_MESSAGE);
        }
    }

    /**
     * User guide actions; launches Adbode to view user guide in PDF format.
     */
    public class UserGuideAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public UserGuideAction() {
            super("User Guide");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                File path = new File(_user_guide);
                if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.OPEN)) {
                    Desktop.getDesktop().open(path);
                } else {
                    String userGuide = "file://localhost/" + path.getAbsolutePath();
                    userGuide = userGuide.replace('\\', '/');
                    System.out.println(userGuide);
                    BareBonesBrowserLaunch.openURL(userGuide);
                }
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Check version action; checks whether a new version of TreeScan is available.
     * TODO: download and updated process not implemented yet.
     */
    public class CheckNewVersionAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public CheckNewVersionAction() {
            super("Check for New Version");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                JOptionPane.showMessageDialog(TreeScanApplication.this, "The lastest version of TreeScan can be found at " + AppConstants.getWebSite() + ".",
                                              "TreeScan Update", JOptionPane.INFORMATION_MESSAGE);
                /*
                UpdateCheckDialog updateCheck = new UpdateCheckDialog(TreeScanApplication.this);
                updateCheck.setVisible(true);
                if (updateCheck.getRestartRequired()) {
                    if (getAnalysesRunning()) {
                        JOptionPane.showMessageDialog(TreeScanApplication.this, "TreeScan can not update will analyses are executing. " +
                                "Please cancel or wait for analyses then close TreeScan.", "Error", JOptionPane.INFORMATION_MESSAGE);
                        return;
                    }
                    //trigger windowClosing event manually ...
                    windowClosing(new WindowEvent(TreeScanApplication.this, WindowEvent.WINDOW_CLOSING));
                }*/
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * returns whether there are actively running analyses
     */
    private boolean getAnalysesRunning() {
        boolean bReturn = false;

        for (int i = 0; i < allOpenFrames.size() && !bReturn; i++) {
            if (allOpenFrames.get(i) instanceof AnalysisRunInternalFrame) {
                bReturn = !((AnalysisRunInternalFrame) allOpenFrames.get(i)).GetCanClose();
            }
        }
        return bReturn;
    }

    /**
     * Suggested citation action; displays the suggested citation dialog.
     */
    public class SuggestedCitationAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public SuggestedCitationAction() {
            super("Suggested Citation");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                URL n = TreeScanApplication.this.getClass().getResource("/suggested_citation.html");
                new SuggestedCitationDialog(TreeScanApplication.this, n).setVisible(true);
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * About dialog action; displays the about dialog.
     */
    public class AboutAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public AboutAction() {
            super("About TreeScan");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                AboutDialog aboutDialog = new AboutDialog(TreeScanApplication.this);
                aboutDialog.setVisible(true);
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Enables/disables various actions based upon whether they are related to
     * the setting windows or the execution/results window.
     */
    private void enableActions(boolean enableSettingActions, boolean enableResultsActions) {
        _executeSessionAction.setEnabled(enableSettingActions);
        _executeOptionsAction.setEnabled(enableSettingActions);
        _closeSessionAction.setEnabled(enableSettingActions);
        _saveSessionAction.setEnabled(enableSettingActions);
        _saveSessionAsAction.setEnabled(enableSettingActions);
        _printResultsAction.setEnabled(enableResultsActions);
    }

    /** Attempt to close parameter settings windows. Returns false if any windows would
     * not close, otherwise true. */
    private boolean CloseParameterSettingsWindows() {
        for (int i = allOpenFrames.size() - 1; i >= 0; i--) {
            if (allOpenFrames.get(i) instanceof ParameterSettingsFrame) {
                if (!((ParameterSettingsFrame) allOpenFrames.get(i)).QueryWindowCanClose()) {
                    return false;
                } else {
                    try {
                        ((ParameterSettingsFrame) allOpenFrames.get(i)).setClosed(true);
                    } catch (PropertyVetoException ex) {
                        Logger.getLogger(TreeScanApplication.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
            }
        }
        return true;
    }

    /** closes all running analysis child windows -- this method is primarily used when closing the
     * application and the user has indicated to close regardless of executing analyses. */
    private void CloseRunningAnalysesWindows() {
        for (int i = 0; i < allOpenFrames.size(); i++) {
            if (allOpenFrames.get(i) instanceof AnalysisRunInternalFrame) {
                ((AnalysisRunInternalFrame) allOpenFrames.get(i)).forceClose();
            }
        }
    }

    /**
     * About dialog action; displays the about dialog.
     */
    public class ReopenAction extends AbstractAction {

        private static final long serialVersionUID = 1L;
        private final File _file;

        public ReopenAction(File file) {
            super(file.getName());
            try {
                putValue(Action.NAME, file.getCanonicalPath().toString());
            } catch (IOException e) {
            }
            _file = file;
        }

        public void actionPerformed(ActionEvent e) {
            try {
                openNewParameterSessionWindow(_file.getAbsolutePath());
            } catch (Throwable t) {
                //The JNI method to read the parameters file might have initiated
                //this exception; so check file access to see if that was infact the problem.
                if (!FileAccess.ValidateFileAccess(_file.getAbsolutePath(), false)) {
                    JOptionPane.showMessageDialog(TreeScanApplication.this,
                            "The parameter file could not be opened for reading:\n " +
                            _file.getAbsolutePath() +
                            "\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.",
                            "Note", JOptionPane.INFORMATION_MESSAGE);
                } else {
                    new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
                }
            }
        }
    }

    /** refreshes 'reopen' menu item to reflect possibly updated history list */
    public void refreshOpenList() {
        _reopenSessionMenu.removeAll();
        for (int i = 0; i < ParameterHistory.getInstance().getHistoryList().size(); ++i) {
            JMenuItem item = new JMenuItem();
            item.setIcon(null);
            item.setAction(new ReopenAction(ParameterHistory.getInstance().getHistoryList().get(i)));
            _reopenSessionMenu.add(item);
        }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        desktopPane = new javax.swing.JDesktopPane();
        _ToolBar = new javax.swing.JToolBar();
        _newSessionToolButton = new javax.swing.JButton();
        jSeparator3 = new javax.swing.JToolBar.Separator();
        _openSessionToolButton = new javax.swing.JButton();
        _saveSessionToolButton = new javax.swing.JButton();
        jSeparator1 = new javax.swing.JToolBar.Separator();
        _executeSessionToolButton = new javax.swing.JButton();
        jSeparator2 = new javax.swing.JToolBar.Separator();
        _printToolButton = new javax.swing.JButton();
        jSeparator4 = new javax.swing.JToolBar.Separator();
        _versionUpdateToolButton = new javax.swing.JButton();
        jSeparator5 = new javax.swing.JToolBar.Separator();
        helpSystemToolButton = new javax.swing.JButton();
        menuBar = new javax.swing.JMenuBar();
        _fileMenu = new javax.swing.JMenu();
        _newSessionMenuItem = new javax.swing.JMenuItem();
        _openSessionMenuItem = new javax.swing.JMenuItem();
        _reopenSessionMenu = new javax.swing.JMenu();
        _closeSessionMenuItem = new javax.swing.JMenuItem();
        _fileMenuSeparator1 = new javax.swing.JSeparator();
        _saveSessionMenuItem = new javax.swing.JMenuItem();
        _saveSessionAsMenuItem = new javax.swing.JMenuItem();
        _fileMenuSeparator2 = new javax.swing.JSeparator();
        _printMenuItem = new javax.swing.JMenuItem();
        _fileMenuSeparator3 = new javax.swing.JSeparator();
        _exitMenuItem = new javax.swing.JMenuItem();
        _sessionMenu = new javax.swing.JMenu();
        _executeSessionMenuItem = new javax.swing.JMenuItem();
        _executeOptionsMenuItem = new javax.swing.JMenuItem();
        _helpMenu = new javax.swing.JMenu();
        _helpContentMenuItem = new javax.swing.JMenuItem();
        _userGuideMenuItem = new javax.swing.JMenuItem();
        _helpMenuSeparator1 = new javax.swing.JSeparator();
        _chechVersionMenuItem = new javax.swing.JMenuItem();
        _helpMenuSeparator2 = new javax.swing.JSeparator();
        _suggestedCitationMenuItem = new javax.swing.JMenuItem();
        _helpMenuSeparator3 = new javax.swing.JSeparator();
        _aboutMenuItem = new javax.swing.JMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setLocationByPlatform(true);

        desktopPane.setBackground(new java.awt.Color(255, 255, 255));

        _ToolBar.setFloatable(false);

        _newSessionToolButton.setAction(new NewSessionFileAction());
        _newSessionToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/New.png"))); // NOI18N
        _newSessionToolButton.setToolTipText("New Session"); // NOI18N
        _newSessionToolButton.setHideActionText(true);
        _ToolBar.add(_newSessionToolButton);
        _ToolBar.add(jSeparator3);

        _openSessionToolButton.setAction(new OpenSessionFileAction());
        _openSessionToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Folder.png"))); // NOI18N
        _openSessionToolButton.setToolTipText("Open Session"); // NOI18N
        _openSessionToolButton.setHideActionText(true);
        _ToolBar.add(_openSessionToolButton);

        _saveSessionToolButton.setAction(_saveSessionAction);
        _saveSessionToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/save.png"))); // NOI18N
        _saveSessionToolButton.setToolTipText("Save Session"); // NOI18N
        _saveSessionToolButton.setHideActionText(true);
        _ToolBar.add(_saveSessionToolButton);
        _ToolBar.add(jSeparator1);

        _executeSessionToolButton.setAction(_executeSessionAction);
        _executeSessionToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Execute.gif"))); // NOI18N
        _executeSessionToolButton.setToolTipText("Execute Session"); // NOI18N
        _executeSessionToolButton.setHideActionText(true);
        _ToolBar.add(_executeSessionToolButton);
        _ToolBar.add(jSeparator2);

        _printToolButton.setAction(_printResultsAction);
        _printToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/print.png"))); // NOI18N
        _printToolButton.setToolTipText("Print Analysis Results"); // NOI18N
        _printToolButton.setHideActionText(true);
        _ToolBar.add(_printToolButton);
        _ToolBar.add(jSeparator4);

        _versionUpdateToolButton.setAction(new CheckNewVersionAction());
        _versionUpdateToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/update.png"))); // NOI18N
        _versionUpdateToolButton.setToolTipText("Check for New Version"); // NOI18N
        _versionUpdateToolButton.setHideActionText(true);
        _ToolBar.add(_versionUpdateToolButton);
        _ToolBar.add(jSeparator5);

        helpSystemToolButton.setAction(new HelpSystemAction());
        helpSystemToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Help.gif"))); // NOI18N
        helpSystemToolButton.setToolTipText("Help"); // NOI18N
        helpSystemToolButton.setHideActionText(true);
        _ToolBar.add(helpSystemToolButton);

        _fileMenu.setMnemonic(KeyEvent.VK_F);
        _fileMenu.setText("File"); // NOI18N

        _newSessionMenuItem.setAction(new NewSessionFileAction());
        _newSessionMenuItem.setText("New Session"); // NOI18N
        _newSessionMenuItem.setIcon(null);
        _newSessionMenuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _fileMenu.add(_newSessionMenuItem);

        _openSessionMenuItem.setAction(new OpenSessionFileAction());
        _openSessionMenuItem.setText("Open Session File"); // NOI18N
        _openSessionMenuItem.setIcon(null);
        _openSessionMenuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _fileMenu.add(_openSessionMenuItem);

        _reopenSessionMenu.setText("Reopen Session File"); // NOI18N
        _reopenSessionMenu.setIcon(null);
        _fileMenu.add(_reopenSessionMenu);

        _closeSessionMenuItem.setAction(_closeSessionAction);
        _closeSessionMenuItem.setIcon(null);
        _closeSessionMenuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_W, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _fileMenu.add(_closeSessionMenuItem);
        _fileMenu.add(_fileMenuSeparator1);

        _saveSessionMenuItem.setAction(_saveSessionAction);
        _saveSessionMenuItem.setIcon(null);
        _saveSessionMenuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _fileMenu.add(_saveSessionMenuItem);

        _saveSessionAsMenuItem.setAction(_saveSessionAsAction);
        _saveSessionAsMenuItem.setIcon(null);
        _saveSessionAsMenuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, (java.awt.event.InputEvent.SHIFT_MASK | (Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()))));
        _fileMenu.add(_saveSessionAsMenuItem);
        _fileMenu.add(_fileMenuSeparator2);

        _printMenuItem.setAction(_printResultsAction);
        _printMenuItem.setIcon(null);
        _printMenuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_P, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _fileMenu.add(_printMenuItem);
        _fileMenu.add(_fileMenuSeparator3);

        _exitMenuItem.setAction(new ExitAction());
        _exitMenuItem.setText("Exit"); // NOI18N
        _exitMenuItem.setIcon(null);
        _fileMenu.add(_exitMenuItem);

        menuBar.add(_fileMenu);

        _sessionMenu.setMnemonic(KeyEvent.VK_S);
        _sessionMenu.setText("Session"); // NOI18N

        _executeSessionMenuItem.setAction(_executeSessionAction);
        _executeSessionMenuItem.setIcon(null);
        _executeSessionMenuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
        _sessionMenu.add(_executeSessionMenuItem);

        _executeOptionsMenuItem.setAction(_executeOptionsAction);
        _executeOptionsMenuItem.setIcon(null);
        _executeOptionsMenuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, (java.awt.event.InputEvent.SHIFT_MASK | (Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()))));
        _sessionMenu.add(_executeOptionsMenuItem);

        menuBar.add(_sessionMenu);

        _helpMenu.setMnemonic(KeyEvent.VK_H);
        _helpMenu.setText("Help"); // NOI18N

        _helpContentMenuItem.setAction(new HelpSystemAction());
        _helpContentMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_F1, 0));
        _helpContentMenuItem.setText("Help Contents"); // NOI18N
        _helpContentMenuItem.setIcon(null);
        _helpMenu.add(_helpContentMenuItem);

        _userGuideMenuItem.setAction(new UserGuideAction());
        _userGuideMenuItem.setText("User Guide"); // NOI18N
        _userGuideMenuItem.setIcon(null);
        _helpMenu.add(_userGuideMenuItem);
        _helpMenu.add(_helpMenuSeparator1);

        _chechVersionMenuItem.setAction(new CheckNewVersionAction());
        _chechVersionMenuItem.setText("Check for New Version"); // NOI18N
        _chechVersionMenuItem.setIcon(null);
        _helpMenu.add(_chechVersionMenuItem);
        _helpMenu.add(_helpMenuSeparator2);

        _suggestedCitationMenuItem.setAction(new SuggestedCitationAction());
        _suggestedCitationMenuItem.setText("Suggested Citation"); // NOI18N
        _suggestedCitationMenuItem.setIcon(null);
        _helpMenu.add(_suggestedCitationMenuItem);
        _helpMenu.add(_helpMenuSeparator3);

        _aboutMenuItem.setAction(new AboutAction());
        _aboutMenuItem.setText("About TreeScan"); // NOI18N
        _aboutMenuItem.setIcon(null);
        _helpMenu.add(_aboutMenuItem);

        menuBar.add(_helpMenu);

        setJMenuBar(menuBar);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_ToolBar, javax.swing.GroupLayout.DEFAULT_SIZE, 768, Short.MAX_VALUE)
            .addComponent(desktopPane, javax.swing.GroupLayout.DEFAULT_SIZE, 768, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(_ToolBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(desktopPane, javax.swing.GroupLayout.DEFAULT_SIZE, 493, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    /**
     * @param args the command line arguments
     */
    public static void main(final String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {

            public void run() {
                try {
                    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
                } catch (Exception e) {
                }
                try {
                    // Window versions prior to 8.0 downloaded updates to application directory.
                    // Attempt to delete update application.
                    if (System.getProperty("os.name").startsWith("Windows")) {
                        String update_app = System.getProperty("user.dir") + System.getProperty("file.separator") + "update_app.exe";
                        new File(update_app).delete();
                    }
                } catch (Throwable e) {
                }
                //check and show End User License Agreement if not "unrequested":
                String DEBUG_URL_STRING = "-debug-url";
                boolean debugURL = false;
                for (int i = 0; i < args.length; ++i) {
                    if (args[i].startsWith(DEBUG_URL_STRING)) {
                        debugURL = true;
                    }
                }
                try {
                    TreeScanApplication.loadSharedLibray();
                } catch (Throwable e) {
                    new ExceptionDialog(null, e).setVisible(true);
                    return;
                }
                TreeScanApplication.setDebugURL(debugURL);
                TreeScanApplication.setRunArgs(args);
                new TreeScanApplication().setVisible(true);
            }
        });
    }

    /**
     * When the main window first shows, displays the start window.
     */
    public void windowGainedFocus(WindowEvent e) {
        if (gbShowStartWindow) {
            try {
                StartDialog startDialog = new StartDialog(TreeScanApplication.this);
                startDialog.setVisible(true);
                gbShowStartWindow = false;
                switch (startDialog.GetOpenType()) {
                    case NEW:
                        openNewParameterSessionWindow("");
                        break;
                    case SAVED:
                        openParameterSessionWindow();
                        break;
                    case LAST:
                        if (FileAccess.ValidateFileAccess(ParameterHistory.getInstance().getHistoryList().get(0).getAbsolutePath(), false)) {
                            openNewParameterSessionWindow(ParameterHistory.getInstance().getHistoryList().get(0).getAbsolutePath());
                        }
                        break;
                    case CANCEL:
                    default:
                        break;
                }
            } catch (Throwable t) {
                new ExceptionDialog(TreeScanApplication.this, t).setVisible(true);
            }
        }
    }

    public void windowLostFocus(WindowEvent e) {
    }

    public void windowOpened(WindowEvent e) {
    }

    /**
     * form close event -- checks whether there are actively running analyses and
     * prompts user as to whether to continue closing accordingly. The ForceClose()
     * method is used to ensure that all child windows will close.
     */
    public void windowClosing(WindowEvent e) {
        if (getAnalysesRunning() &&
                JOptionPane.showConfirmDialog(this, "There are analyses currently executing. Are you sure you want to exit TreeScan?", "Warning", JOptionPane.YES_NO_OPTION) == JOptionPane.NO_OPTION) {
            return;
        }
        if (!CloseParameterSettingsWindows()) {
            return;
        }
        CloseRunningAnalysesWindows();
        if (UpdateCheckDialog._runUpdateOnTerminate) {
            try { //launch updater application and close
                // Get java path from System
                StringBuilder java_path = new StringBuilder();
                java_path.append(System.getProperty("java.home")).append(System.getProperty("file.separator")).append("bin").append(System.getProperty("file.separator")).append("java");
                String[] commandline = new String[]{java_path.toString(),
                    "-jar",
                    UpdateCheckDialog._updaterFilename.getName(),
                    UpdateCheckDialog._updateArchiveName.getName(),
                    _application,
                    getRelaunchArgs()
                };
                Runtime.getRuntime().exec(commandline, null, UpdateCheckDialog.getDownloadTempDirectory());
            } catch (IOException ex) {
                Logger.getLogger(TreeScanApplication.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        //saving window dimensions
        Preferences _prefs = Preferences.userNodeForPackage(TreeScanApplication.class);
        _prefs.put(WIDTH_KEY, Integer.toString(getSize().width));
        _prefs.put(HEIGHT_KEY, Integer.toString(getSize().height));
        try {
            _prefs.flush();
        } catch (BackingStoreException ex) {
        }
        this.dispose();
        System.exit(0);
    }

    public void windowClosed(WindowEvent e) {
    }

    public void windowIconified(WindowEvent e) {
    }

    public void windowDeiconified(WindowEvent e) {
    }

    public void windowActivated(WindowEvent e) {
    }

    public void windowDeactivated(WindowEvent e) {
    }

    /**
     * Adds frame to collection of internal frames.
     */
    public void internalFrameOpened(InternalFrameEvent e) {
        allOpenFrames.addElement(e.getInternalFrame());
    }

    public void internalFrameClosing(InternalFrameEvent e) {
    }

    /**
     * Responds to the start window closing event by invoking user response.
     */
    public void internalFrameClosed(InternalFrameEvent e) {
        allOpenFrames.removeElement(e.getInternalFrame());
        if (e.getInternalFrame() instanceof ParameterSettingsFrame) {
            refreshOpenList();
        }
    }

    public void internalFrameIconified(InternalFrameEvent e) {
    }

    public void internalFrameDeiconified(InternalFrameEvent e) {
    }

    /**
     * Responds to the activation of an internal frame; enabling various actions.
     * The focused internal frame is noted for reference.
     */
    public void internalFrameActivated(InternalFrameEvent e) {
        _focusedInternalFrame = e.getInternalFrame();
        enableActions((_focusedInternalFrame instanceof ParameterSettingsFrame), (_focusedInternalFrame instanceof AnalysisRunInternalFrame));
    }

    /**
     * Responds to the activation of an internal frame; enabling various actions.
     */
    public void internalFrameDeactivated(InternalFrameEvent e) {
        if (_focusedInternalFrame == e.getInternalFrame()) {
            _focusedInternalFrame = null;
        }
        enableActions(false, false);
    }

    public void lostOwnership(Clipboard clipboard, Transferable contents) {
    //throw new UnsupportedOperationException("Not supported yet.");
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JToolBar _ToolBar;
    private javax.swing.JMenuItem _aboutMenuItem;
    private javax.swing.JMenuItem _chechVersionMenuItem;
    private javax.swing.JMenuItem _closeSessionMenuItem;
    private javax.swing.JMenuItem _executeOptionsMenuItem;
    private javax.swing.JMenuItem _executeSessionMenuItem;
    private javax.swing.JButton _executeSessionToolButton;
    private javax.swing.JMenuItem _exitMenuItem;
    private javax.swing.JMenu _fileMenu;
    private javax.swing.JSeparator _fileMenuSeparator1;
    private javax.swing.JSeparator _fileMenuSeparator2;
    private javax.swing.JSeparator _fileMenuSeparator3;
    private javax.swing.JMenuItem _helpContentMenuItem;
    private javax.swing.JMenu _helpMenu;
    private javax.swing.JSeparator _helpMenuSeparator1;
    private javax.swing.JSeparator _helpMenuSeparator2;
    private javax.swing.JSeparator _helpMenuSeparator3;
    private javax.swing.JMenuItem _newSessionMenuItem;
    private javax.swing.JButton _newSessionToolButton;
    private javax.swing.JMenuItem _openSessionMenuItem;
    private javax.swing.JButton _openSessionToolButton;
    private javax.swing.JMenuItem _printMenuItem;
    private javax.swing.JButton _printToolButton;
    private javax.swing.JMenu _reopenSessionMenu;
    private javax.swing.JMenuItem _saveSessionAsMenuItem;
    private javax.swing.JMenuItem _saveSessionMenuItem;
    private javax.swing.JButton _saveSessionToolButton;
    private javax.swing.JMenu _sessionMenu;
    private javax.swing.JMenuItem _suggestedCitationMenuItem;
    private javax.swing.JMenuItem _userGuideMenuItem;
    private javax.swing.JButton _versionUpdateToolButton;
    private javax.swing.JDesktopPane desktopPane;
    private javax.swing.JButton helpSystemToolButton;
    private javax.swing.JToolBar.Separator jSeparator1;
    private javax.swing.JToolBar.Separator jSeparator2;
    private javax.swing.JToolBar.Separator jSeparator3;
    private javax.swing.JToolBar.Separator jSeparator4;
    private javax.swing.JToolBar.Separator jSeparator5;
    private javax.swing.JMenuBar menuBar;
    // End of variables declaration//GEN-END:variables
}
