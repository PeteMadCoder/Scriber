#pragma once

#include <QWidget>
#include <QDir>
#include <QModelIndex>
#include <QSet>

class QTreeView;
class QFileSystemModel;
class QMenu;
class QAction;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class ToastNotification;

/**
 * @brief File clipboard structure for cut/copy/paste operations
 */
struct FileClipboard {
    QStringList paths;
    bool isCut;
    
    FileClipboard() : isCut(false) {}
};

/**
 * @brief Sidebar file explorer with VS Code-style file management
 *
 * Provides a tree view of the file system with operations like:
 * - Create new file/folder
 * - Rename (inline editor)
 * - Delete (with confirmation)
 * - Cut/Copy/Paste
 * - Duplicate
 * - Reveal in file manager
 * - Drag & drop support
 */
class SidebarFileExplorer : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarFileExplorer(QWidget *parent = nullptr);
    ~SidebarFileExplorer();
    
    /// Set the initial directory to display
    void setRootPath(const QString &path);
    
    /// Get the current root path
    QString currentPath() const;
    
    /// Refresh the file tree view
    void refresh();
    
    /// Set path editable (for typing path directly)
    void setPathEditable(bool editable);

signals:
    /// Emitted when a file is double-clicked
    void fileActivated(const QString &filePath);
    
    /// Emitted when a directory is navigated to
    void directoryChanged(const QString &path);
    
    /// Emitted when context menu action needs to open a file
    void openFileRequested(const QString &filePath);

public slots:
    void onNewFile();
    void onNewFolder();
    void onDelete();
    void onDeletePermanently();
    void onCut();
    void onCopy();
    void onPaste();
    void onDuplicate();
    void onRevealInFileManager();
    void onOpenContainingFolder();
    void onRefresh();
    void onParentDirectory();
    
    // Keyboard shortcut handlers
    void handleF2();
    void handleDelete(bool shiftPressed);
    void handleCtrlD();
    void handleCtrlC();
    void handleCtrlX();
    void handleCtrlV();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onFileTreeDoubleClicked(const QModelIndex &index);
    void onFileTreeContextMenu(const QPoint &pos);
    void onSelectionChanged();
    void onPathEdited(const QString &path);
    
    // Inline rename
    void startRenameEditor(const QModelIndex &index);
    void finishRenameEditor();
    void cancelRenameEditor();

private:
    void createWidgets();
    void createActions();
    void createContextMenu();
    void setupConnections();
    void updateActionsState();
    
    bool copyDirectory(const QDir &source, const QDir &destination);
    void deleteFiles(const QStringList &paths, bool permanent);
    
    // Navigation widgets
    QPushButton *parentDirButton;
    QLineEdit *pathEdit;
    QPushButton *refreshButton;
    
    // File tree
    QTreeView *fileTreeView;
    QFileSystemModel *fileSystemModel;
    
    // Inline rename editor
    QLineEdit *inlineRenameEditor;
    QModelIndex renameEditorIndex;
    
    // Context menu
    QMenu *contextMenu;
    QAction *newFileAct;
    QAction *newFolderAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *duplicateAct;
    QAction *renameAct;
    QAction *deleteAct;
    QAction *revealAct;
    QAction *openContainingFolderAct;
    QAction *refreshAct;
    
    // Clipboard
    FileClipboard clipboard;
    
    // Toast notification
    ToastNotification *toast;
};
