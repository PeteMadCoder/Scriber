// mainwindow.h
#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QLabel>
#include <QTabWidget>
#include <QList>

class EditorWidget;
class FileManager;
class QString;
class QAction;
class QMenu;
class QDockWidget;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;

class FindBarWidget;
class DocumentOutlineWidget;
class SidebarFileExplorer;
class ToastNotification;
class OutlineDelegate;

// Structure to track editor and file path per tab
struct EditorTab {
    EditorWidget *editor;
    QString filePath;
    bool isModified;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void openFile(const QString &path = QString());
    void newFile();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // Tab management
    void onTabChanged(int index);
    void closeTab(int index);
    void documentWasModified();
    
    // File operations
    void open();
    bool save();
    bool saveAs();
    void exportToHtml();
    void exportToPdf();
    
    // UI
    void selectTheme();
    void about();
    void find();
    bool isFindBarVisible() const;
    
    // Outline
    void updateOutline();
    
    // Word count
    void updateWordCount();

private:
    // Initialization
    void createActions();
    void createMenus();
    void createStatusBar();
    void createSidebar();
    void loadSettings();
    void saveSettings();
    
    // Tab management
    void openFileInNewTab(const QString &fileName);
    void updateTabTitle(int index);
    void updateWindowTitle();
    void updateActionsState();
    bool maybeSave();
    bool maybeSaveCurrentTab();
    void setCurrentFile(const QString &fileName);
    
    // UI components
    void setupEditorConnections(EditorWidget *editor);
    
    // Members
    QTabWidget *tabWidget;
    QList<EditorTab> editorTabs;
    QPointer<FileManager> fileManager;
    
    // Sidebar
    QDockWidget *sidebarDock;
    QTabWidget *sidebarTabs;
    SidebarFileExplorer *fileExplorer;
    DocumentOutlineWidget *outlineWidget;
    QAction *toggleSidebarAct;
    OutlineDelegate *outlineDelegate;
    
    // Find bar
    FindBarWidget *findBarWidget;
    
    // Toast notification
    ToastNotification *toast;
    
    // Status bar
    QLabel *wordCountLabel;
    QLabel *charCountLabel;
    
    // Timers
    QTimer *wordCountTimer;
    QTimer *outlineTimer;
    
    // Menu actions
    QAction newAct;
    QAction openAct;
    QAction saveAct;
    QAction saveAsAct;
    QAction exportHtmlAct;
    QAction exportPdfAct;
    QAction exitAct;
    QAction selectThemeAct;
    QAction aboutAct;
    QAction findAct;
    QAction closeTabAct;
};
