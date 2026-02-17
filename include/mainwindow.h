// mainwindow.h
#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QLabel>
#include <QMap>

class EditorWidget;
class FileManager;
class QString;
class QAction;
class QLineEdit;
class QLabel;
class QCheckBox;
class QPushButton;
class QWidget;
class QDockWidget;
class QTreeView;
class QFileSystemModel;
class QTimer;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

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
    void updateOutline();
    void onOutlineItemClicked(QTreeWidgetItem *item, int column);
    void open();
    bool save();
    bool saveAs();
    void exportToHtml();
    void exportToPdf();
    void toggleTheme();
    void about();
    void find();
    void onFindNext();
    void onFindPrevious();
    void onFindTextEdited();
    void hideFindBar();
    void documentWasModified();
    void onTabChanged(int index);
    void closeTab(int index);

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void loadSettings();
    void saveSettings();
    bool maybeSave();
    bool maybeSaveCurrentTab();
    void setCurrentFile(const QString &fileName);
    void createFindBar();
    void createSidebar();
    void openFileInNewTab(const QString &fileName);
    void updateTabTitle(int index);
    void updateWindowTitle();
    void updateActionsState();

    void updateWordCount();
    QLabel *wordCountLabel;
    QLabel *charCountLabel;

    QTabWidget *tabWidget;
    QList<EditorTab> editorTabs;
    QPointer<FileManager> fileManager;

    // Sidebar members
    QDockWidget *sidebarDock;
    QTabWidget *sidebarTabs;
    QTreeView *fileTreeView;
    QTreeWidget *outlineTree;
    QFileSystemModel *fileSystemModel;
    QAction *toggleSidebarAct;

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
    QAction toggleThemeAct;
    QAction aboutAct;
    QAction findAct;
    QAction closeTabAct;

    // Find bar members
    bool isFindBarVisible;
    QWidget *findBarWidget;
    QLineEdit *findLineEdit;
    QLabel *findStatusLabel;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *wholeWordsCheckBox;
    QPushButton *findNextButton;
    QPushButton *findPreviousButton;
    QPushButton *closeFindBarButton;
};