// mainwindow.h
#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QLabel>

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
class QTimer; // Forward declaration

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

private:
    void createActions();
    void createMenus();
    void createStatusBar();
    void loadSettings();
    void saveSettings();
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    void createFindBar();
    void createSidebar();

    void updateWordCount();
    QLabel *wordCountLabel;
    QLabel *charCountLabel;

    QPointer<EditorWidget> editor;
    QPointer<FileManager> fileManager;
    QString currentFile;

    // Sidebar members
    QDockWidget *sidebarDock;
    QTreeView *fileTreeView;
    QFileSystemModel *fileSystemModel;
    QAction *toggleSidebarAct;
    
    QTimer *wordCountTimer; // Debounce word count updates

    // Menu actions - Member objects
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

    // --- Embedded Find Bar Members ---
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