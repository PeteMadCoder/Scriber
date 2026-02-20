#include "sidebarfileexplorer.h"
#include "toastnotification.h"
#include "thememanager.h"
#include <QTreeView>
#include <QHeaderView>
#include <QFileSystemModel>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>

SidebarFileExplorer::SidebarFileExplorer(QWidget *parent)
    : QWidget(parent)
    , fileTreeView(nullptr)
    , fileSystemModel(nullptr)
    , inlineRenameEditor(nullptr)
    , contextMenu(nullptr)
    , toast(nullptr)
{
    createWidgets();
    createActions();
    createContextMenu();
    setupConnections();
}

SidebarFileExplorer::~SidebarFileExplorer()
{
}

void SidebarFileExplorer::createWidgets()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Navigation toolbar
    QHBoxLayout *navLayout = new QHBoxLayout();
    navLayout->setSpacing(2);
    
    parentDirButton = new QPushButton();
    parentDirButton->setFixedSize(28, 28);
    parentDirButton->setToolTip(tr("Go to Parent Directory (Backspace)"));
    QIcon parentIcon = QIcon::fromTheme("go-up", QIcon::fromTheme("folder-open"));
    parentDirButton->setIcon(parentIcon.isNull() ? QIcon() : parentIcon);
    if (parentIcon.isNull()) parentDirButton->setText("↑");
    navLayout->addWidget(parentDirButton);
    
    pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText(tr("Current directory"));
    navLayout->addWidget(pathEdit);
    
    refreshButton = new QPushButton();
    refreshButton->setFixedSize(28, 28);
    refreshButton->setToolTip(tr("Refresh (F5)"));
    QIcon refreshIcon = QIcon::fromTheme("view-refresh", QIcon::fromTheme("reload"));
    refreshButton->setIcon(refreshIcon.isNull() ? QIcon() : refreshIcon);
    if (refreshIcon.isNull()) refreshButton->setText("⟳");
    navLayout->addWidget(refreshButton);
    
    mainLayout->addLayout(navLayout);
    
    // File tree view
    fileTreeView = new QTreeView();
    fileSystemModel = new QFileSystemModel(this);
    
    // Set root path FIRST before setting the model index
    QString initialPath = QDir::currentPath();
    fileSystemModel->setRootPath(initialPath);
    fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    QStringList nameFilters;
    nameFilters << "*.md" << "*.markdown" << "*.txt";
    fileSystemModel->setNameFilters(nameFilters);
    fileSystemModel->setNameFilterDisables(false);

    fileTreeView->setModel(fileSystemModel);
    fileTreeView->setRootIndex(fileSystemModel->index(initialPath));
    fileTreeView->setColumnHidden(1, true);
    fileTreeView->setColumnHidden(2, true);
    fileTreeView->setColumnHidden(3, true);
    fileTreeView->header()->setVisible(false);
    
    // Set initial path in path edit
    pathEdit->setText(initialPath);

    // VS Code-style features
    fileTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileTreeView->setDragEnabled(true);
    fileTreeView->setAcceptDrops(true);
    fileTreeView->setDropIndicatorShown(true);
    fileTreeView->setDragDropMode(QAbstractItemView::DragDrop);
    fileTreeView->setDefaultDropAction(Qt::MoveAction);
    
    // Enable context menu
    fileTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    mainLayout->addWidget(fileTreeView);
    
    // Inline rename editor (hidden by default)
    inlineRenameEditor = new QLineEdit(fileTreeView);
    inlineRenameEditor->hide();
    inlineRenameEditor->setFrame(false);
    
    // Toast notification
    toast = new ToastNotification(this);
}

void SidebarFileExplorer::createActions()
{
    // Actions are created in createContextMenu
}

void SidebarFileExplorer::createContextMenu()
{
    contextMenu = new QMenu(this);
    
    // New File/Folder section
    newFileAct = new QAction(tr("New File"), this);
    newFileAct->setIcon(QIcon::fromTheme("document-new", QIcon::fromTheme("text-x-generic")));
    newFileAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    
    newFolderAct = new QAction(tr("New Folder"), this);
    newFolderAct->setIcon(QIcon::fromTheme("folder-new", QIcon::fromTheme("folder")));
    newFolderAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    
    contextMenu->addAction(newFileAct);
    contextMenu->addAction(newFolderAct);
    contextMenu->addSeparator();
    
    // Clipboard operations
    cutAct = new QAction(tr("Cut"), this);
    cutAct->setIcon(QIcon::fromTheme("edit-cut"));
    cutAct->setShortcut(QKeySequence::Cut);
    
    copyAct = new QAction(tr("Copy"), this);
    copyAct->setIcon(QIcon::fromTheme("edit-copy"));
    copyAct->setShortcut(QKeySequence::Copy);
    
    pasteAct = new QAction(tr("Paste"), this);
    pasteAct->setIcon(QIcon::fromTheme("edit-paste"));
    pasteAct->setShortcut(QKeySequence::Paste);
    
    duplicateAct = new QAction(tr("Duplicate"), this);
    duplicateAct->setIcon(QIcon::fromTheme("edit-copy"));
    duplicateAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    
    contextMenu->addAction(cutAct);
    contextMenu->addAction(copyAct);
    contextMenu->addAction(pasteAct);
    contextMenu->addAction(duplicateAct);
    contextMenu->addSeparator();
    
    // File operations
    renameAct = new QAction(tr("Rename"), this);
    renameAct->setIcon(QIcon::fromTheme("edit-rename"));
    renameAct->setShortcut(QKeySequence(Qt::Key_F2));
    
    deleteAct = new QAction(tr("Delete"), this);
    deleteAct->setIcon(QIcon::fromTheme("edit-delete"));
    deleteAct->setShortcut(QKeySequence::Delete);
    
    contextMenu->addAction(renameAct);
    contextMenu->addAction(deleteAct);
    contextMenu->addSeparator();
    
    // Reveal/Open containing folder
    revealAct = new QAction(tr("Reveal in File Manager"), this);
    revealAct->setIcon(QIcon::fromTheme("system-file-manager"));
#ifdef Q_OS_WIN
    revealAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
#else
    revealAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
#endif
    
    openContainingFolderAct = new QAction(tr("Open Containing Folder"), this);
    openContainingFolderAct->setIcon(QIcon::fromTheme("folder-open"));
    
    contextMenu->addAction(revealAct);
    contextMenu->addAction(openContainingFolderAct);
    contextMenu->addSeparator();
    
    // Refresh
    refreshAct = new QAction(tr("Refresh"), this);
    refreshAct->setIcon(QIcon::fromTheme("view-refresh"));
    refreshAct->setShortcut(QKeySequence(Qt::Key_F5));
    
    contextMenu->addAction(refreshAct);
}

void SidebarFileExplorer::setupConnections()
{
    // Navigation buttons
    connect(parentDirButton, &QPushButton::clicked, this, &SidebarFileExplorer::onParentDirectory);
    connect(pathEdit, &QLineEdit::editingFinished, [this]() {
        onPathEdited(pathEdit->text());
    });
    connect(refreshButton, &QPushButton::clicked, this, &SidebarFileExplorer::onRefresh);
    
    // File tree
    connect(fileTreeView, &QTreeView::doubleClicked, this, &SidebarFileExplorer::onFileTreeDoubleClicked);
    connect(fileTreeView, &QTreeView::customContextMenuRequested, this, &SidebarFileExplorer::onFileTreeContextMenu);
    connect(fileTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &SidebarFileExplorer::onSelectionChanged);
    
    // Context menu actions
    connect(newFileAct, &QAction::triggered, this, &SidebarFileExplorer::onNewFile);
    connect(newFolderAct, &QAction::triggered, this, &SidebarFileExplorer::onNewFolder);
    connect(cutAct, &QAction::triggered, this, &SidebarFileExplorer::onCut);
    connect(copyAct, &QAction::triggered, this, &SidebarFileExplorer::onCopy);
    connect(pasteAct, &QAction::triggered, this, &SidebarFileExplorer::onPaste);
    connect(duplicateAct, &QAction::triggered, this, &SidebarFileExplorer::onDuplicate);
    connect(renameAct, &QAction::triggered, [this]() {
        QModelIndex index = fileTreeView->currentIndex();
        if (index.isValid()) startRenameEditor(index);
    });
    connect(deleteAct, &QAction::triggered, this, &SidebarFileExplorer::onDelete);
    connect(revealAct, &QAction::triggered, this, &SidebarFileExplorer::onRevealInFileManager);
    connect(openContainingFolderAct, &QAction::triggered, this, &SidebarFileExplorer::onOpenContainingFolder);
    connect(refreshAct, &QAction::triggered, this, &SidebarFileExplorer::onRefresh);
    
    // Inline rename
    connect(inlineRenameEditor, &QLineEdit::editingFinished, this, &SidebarFileExplorer::finishRenameEditor);
    connect(inlineRenameEditor, &QLineEdit::returnPressed, this, &SidebarFileExplorer::finishRenameEditor);
}

void SidebarFileExplorer::setRootPath(const QString &path)
{
    QFileInfo fi(path);
    QString dirPath = fi.absolutePath();
    
    QModelIndex index = fileSystemModel->index(dirPath);
    if (index.isValid()) {
        fileSystemModel->setRootPath(dirPath);
        fileTreeView->setRootIndex(index);
        pathEdit->setText(dirPath);
    } else {
        // Fallback: try to set root path directly
        fileSystemModel->setRootPath(dirPath);
        pathEdit->setText(dirPath);
    }
}

QString SidebarFileExplorer::currentPath() const
{
    QModelIndex currentIndex = fileTreeView->rootIndex();
    if (currentIndex.isValid()) {
        return fileSystemModel->filePath(currentIndex);
    }
    return fileSystemModel->rootPath();
}

void SidebarFileExplorer::refresh()
{
    QModelIndex currentIndex = fileTreeView->rootIndex();
    if (currentIndex.isValid()) {
        QString path = fileSystemModel->filePath(currentIndex);
        fileSystemModel->setRootPath(path);
        fileTreeView->setRootIndex(currentIndex);
    } else {
        QString rootPath = fileSystemModel->rootPath();
        fileSystemModel->setRootPath(rootPath);
        fileTreeView->setRootIndex(fileSystemModel->index(rootPath));
    }
}

void SidebarFileExplorer::setPathEditable(bool editable)
{
    pathEdit->setReadOnly(!editable);
}

void SidebarFileExplorer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && inlineRenameEditor && inlineRenameEditor->isVisible()) {
        cancelRenameEditor();
        event->accept();
        return;
    }
    
    if (fileTreeView && fileTreeView->hasFocus()) {
        if (event->key() == Qt::Key_Backspace) {
            onParentDirectory();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_F2) {
            handleF2();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Delete) {
            handleDelete(event->modifiers() == Qt::ShiftModifier);
            event->accept();
            return;
        }
        if (event->modifiers() == Qt::ControlModifier) {
            if (event->key() == Qt::Key_D) {
                handleCtrlD();
                event->accept();
                return;
            }
            if (event->key() == Qt::Key_C) {
                handleCtrlC();
                event->accept();
                return;
            }
            if (event->key() == Qt::Key_X) {
                handleCtrlX();
                event->accept();
                return;
            }
            if (event->key() == Qt::Key_V) {
                handleCtrlV();
                event->accept();
                return;
            }
        }
    }
    
    QWidget::keyPressEvent(event);
}

// ============================================================================
// Slot Implementations
// ============================================================================

void SidebarFileExplorer::onFileTreeDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    QString filePath = fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);
    
    if (fileInfo.isDir()) {
        fileTreeView->setRootIndex(index);
        pathEdit->setText(filePath);
        emit directoryChanged(filePath);
    } else {
        emit fileActivated(filePath);
    }
}

void SidebarFileExplorer::onFileTreeContextMenu(const QPoint &pos)
{
    QModelIndex index = fileTreeView->indexAt(pos);
    
    // Select the item under cursor if not already selected
    if (index.isValid() && !fileTreeView->selectionModel()->isSelected(index)) {
        fileTreeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    }
    
    updateActionsState();
    contextMenu->exec(fileTreeView->viewport()->mapToGlobal(pos));
}

void SidebarFileExplorer::onSelectionChanged()
{
    updateActionsState();
}

void SidebarFileExplorer::updateActionsState()
{
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    int selectedCount = 0;
    for (const QModelIndex &index : selectedIndices) {
        if (index.column() == 0) selectedCount++;
    }
    
    renameAct->setEnabled(selectedCount == 1);
    deleteAct->setEnabled(selectedCount > 0);
    cutAct->setEnabled(selectedCount > 0);
    copyAct->setEnabled(selectedCount > 0);
    duplicateAct->setEnabled(selectedCount == 1);
    revealAct->setEnabled(selectedCount == 1);
    openContainingFolderAct->setEnabled(selectedCount > 0);
    pasteAct->setEnabled(!clipboard.paths.isEmpty());
}

void SidebarFileExplorer::onPathEdited(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists() && fi.isDir()) {
        setRootPath(path);
        emit directoryChanged(path);
    } else {
        pathEdit->setText(currentPath());
    }
}

void SidebarFileExplorer::onParentDirectory()
{
    QModelIndex currentIndex = fileTreeView->rootIndex();
    if (currentIndex.isValid()) {
        QModelIndex parentIndex = currentIndex.parent();
        if (parentIndex.isValid()) {
            fileTreeView->setRootIndex(parentIndex);
            pathEdit->setText(fileSystemModel->filePath(parentIndex));
            emit directoryChanged(fileSystemModel->filePath(parentIndex));
        } else {
            QString currentPath = fileSystemModel->filePath(currentIndex);
            QFileInfo fi(currentPath);
            QString parentPath = fi.absolutePath();
            if (parentPath != currentPath) {
                QModelIndex parentIdx = fileSystemModel->index(parentPath);
                if (parentIdx.isValid()) {
                    fileTreeView->setRootIndex(parentIdx);
                    pathEdit->setText(parentPath);
                    emit directoryChanged(parentPath);
                }
            }
        }
    }
}

void SidebarFileExplorer::onRefresh()
{
    refresh();
}

void SidebarFileExplorer::onNewFile()
{
    QString dirPath = currentPath();
    
    QString baseName = "untitled.md";
    QString filePath = QDir(dirPath).filePath(baseName);
    
    int counter = 1;
    while (QFile::exists(filePath)) {
        baseName = QString("untitled%1.md").arg(counter++);
        filePath = QDir(dirPath).filePath(baseName);
    }
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.close();
        refresh();
        emit fileActivated(filePath);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not create file: %1").arg(file.errorString()));
    }
}

void SidebarFileExplorer::onNewFolder()
{
    QString dirPath = currentPath();
    
    QString baseName = "New Folder";
    QString folderPath = QDir(dirPath).filePath(baseName);
    
    int counter = 1;
    while (QDir(folderPath).exists()) {
        baseName = QString("New Folder %1").arg(counter++);
        folderPath = QDir(dirPath).filePath(baseName);
    }
    
    if (QDir().mkdir(folderPath)) {
        refresh();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not create folder: %1").arg(folderPath));
    }
}

void SidebarFileExplorer::startRenameEditor(const QModelIndex &index)
{
    if (!index.isValid() || !inlineRenameEditor) return;
    
    renameEditorIndex = index;
    QString oldPath = fileSystemModel->filePath(index);
    QFileInfo fi(oldPath);
    
    QRect rect = fileTreeView->visualRect(index);
    if (!rect.isValid()) return;
    
    ThemeManager *themeManager = ThemeManager::instance();
    QColor bgColor = themeManager->backgroundColor();
    QColor textColor = themeManager->textColor();
    QColor highlightColor = themeManager->highlightColor();
    
    inlineRenameEditor->setGeometry(rect);
    inlineRenameEditor->setText(fi.fileName());
    inlineRenameEditor->selectAll();
    inlineRenameEditor->setStyleSheet(
        QString(
            "QLineEdit { "
            "  background-color: %1; "
            "  color: %2; "
            "  border: 2px solid %3; "
            "  padding: 2px 4px; "
            "  font-weight: bold;"
            "  selection-background-color: %3;"
            "  selection-color: %1;"
            "}"
        ).arg(bgColor.name()).arg(textColor.name()).arg(highlightColor.name())
    );
    
    inlineRenameEditor->show();
    inlineRenameEditor->setFocus();
}

void SidebarFileExplorer::finishRenameEditor()
{
    if (!inlineRenameEditor || !inlineRenameEditor->isVisible() || !renameEditorIndex.isValid()) return;
    
    QString newName = inlineRenameEditor->text().trimmed();
    inlineRenameEditor->hide();
    
    if (newName.isEmpty()) {
        cancelRenameEditor();
        return;
    }
    
    QString oldPath = fileSystemModel->filePath(renameEditorIndex);
    QFileInfo fi(oldPath);
    
    if (newName == fi.fileName()) {
        renameEditorIndex = QModelIndex();
        return;
    }
    
    QString newPath = QDir(fi.absolutePath()).filePath(newName);
    
    if (QFile::rename(oldPath, newPath)) {
        toast->showMessage(tr("Renamed to \"%1\"").arg(newName));
        refresh();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not rename file/folder to \"%1\"").arg(newName));
    }
    
    renameEditorIndex = QModelIndex();
}

void SidebarFileExplorer::cancelRenameEditor()
{
    if (inlineRenameEditor) inlineRenameEditor->hide();
    renameEditorIndex = QModelIndex();
}

void SidebarFileExplorer::onDelete()
{
    QSet<QString> uniquePaths;
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    for (const QModelIndex &index : selectedIndices) {
        if (index.isValid() && index.column() == 0) {
            uniquePaths << fileSystemModel->filePath(index);
        }
    }
    
    QStringList pathsToDelete = uniquePaths.values();
    if (pathsToDelete.isEmpty()) return;
    
    int fileCount = 0, dirCount = 0;
    for (const QString &path : pathsToDelete) {
        QFileInfo fi(path);
        if (fi.isDir()) dirCount++;
        else fileCount++;
    }
    
    QString message;
    if (fileCount > 0 && dirCount > 0) {
        message = tr("Are you sure you want to delete %1 file(s) and %2 folder(s)?\nThis will move them to trash.").arg(fileCount).arg(dirCount);
    } else if (dirCount > 0) {
        message = tr("Are you sure you want to delete %1 folder(s)?\nThis will move them to trash.").arg(dirCount);
    } else {
        message = tr("Are you sure you want to delete %1 file(s)?\nThis will move them to trash.").arg(fileCount);
    }
    
    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("Confirm Delete"), message,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    deleteFiles(pathsToDelete, false);
}

void SidebarFileExplorer::onDeletePermanently()
{
    QSet<QString> uniquePaths;
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    for (const QModelIndex &index : selectedIndices) {
        if (index.isValid() && index.column() == 0) {
            uniquePaths << fileSystemModel->filePath(index);
        }
    }
    
    QStringList pathsToDelete = uniquePaths.values();
    if (pathsToDelete.isEmpty()) return;
    
    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("Permanently Delete"),
        tr("Are you sure you want to PERMANENTLY delete these items?\nThis action cannot be undone!"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    deleteFiles(pathsToDelete, true);
}

void SidebarFileExplorer::deleteFiles(const QStringList &paths, bool permanent)
{
    Q_UNUSED(permanent); // On Linux, QFile::remove() doesn't support trash
    
    int successCount = 0;
    for (const QString &path : paths) {
        QFileInfo fi(path);
        bool success = false;
        
        if (fi.isDir()) {
            QDir dir(path);
            success = dir.removeRecursively();
        } else {
            success = QFile::remove(path);
        }
        
        if (success) successCount++;
    }
    
    if (successCount > 0) {
        toast->showMessage(tr("Deleted %1 item(s)").arg(successCount));
        refresh();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not delete items"));
    }
}

void SidebarFileExplorer::onCut()
{
    QSet<QString> uniquePaths;
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    for (const QModelIndex &index : selectedIndices) {
        if (index.isValid() && index.column() == 0) {
            uniquePaths << fileSystemModel->filePath(index);
        }
    }
    
    clipboard.paths = uniquePaths.values();
    clipboard.isCut = true;
    
    toast->showMessage(tr("Cut %1 file(s)").arg(clipboard.paths.size()));
}

void SidebarFileExplorer::onCopy()
{
    QSet<QString> uniquePaths;
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    for (const QModelIndex &index : selectedIndices) {
        if (index.isValid() && index.column() == 0) {
            uniquePaths << fileSystemModel->filePath(index);
        }
    }
    
    clipboard.paths = uniquePaths.values();
    clipboard.isCut = false;
    
    toast->showMessage(tr("Copied %1 file(s)").arg(clipboard.paths.size()));
}

void SidebarFileExplorer::onPaste()
{
    if (clipboard.paths.isEmpty()) return;
    
    QString destDirPath = currentPath();
    int pasteCount = 0;
    
    for (const QString &sourcePath : clipboard.paths) {
        QFileInfo fi(sourcePath);
        QString destPath = QDir(destDirPath).filePath(fi.fileName());
        
        int counter = 1;
        while (QFile::exists(destPath)) {
            QString baseName = fi.baseName();
            QString suffix = fi.suffix();
            if (fi.isDir()) {
                destPath = QDir(destDirPath).filePath(QString("%1 %2").arg(fi.fileName()).arg(counter++));
            } else {
                destPath = QDir(destDirPath).filePath(QString("%1 %2.%3").arg(baseName).arg(counter++).arg(suffix));
            }
        }
        
        bool success = false;
        if (clipboard.isCut) {
            success = QFile::rename(sourcePath, destPath);
        } else {
            if (fi.isDir()) {
                QDir sourceDir(sourcePath);
                QDir destDir(destPath);
                success = copyDirectory(sourceDir, destDir);
            } else {
                success = QFile::copy(sourcePath, destPath);
            }
        }
        
        if (success) pasteCount++;
    }
    
    if (pasteCount > 0) {
        toast->showMessage(tr("Pasted %1 item(s)").arg(pasteCount));
        refresh();
        
        if (clipboard.isCut) {
            clipboard.paths.clear();
            clipboard.isCut = false;
        }
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not paste items"));
    }
}

void SidebarFileExplorer::onDuplicate()
{
    QModelIndex index = fileTreeView->currentIndex();
    if (!index.isValid()) return;
    
    QString sourcePath = fileSystemModel->filePath(index);
    QFileInfo fi(sourcePath);
    
    QString baseName = fi.baseName();
    QString suffix = fi.suffix();
    QString destPath;
    int counter = 1;
    
    do {
        if (fi.isDir()) {
            destPath = QDir(fi.absolutePath()).filePath(QString("%1 (copy %2)").arg(fi.fileName()).arg(counter++));
        } else {
            destPath = QDir(fi.absolutePath()).filePath(QString("%1 (copy %2).%3").arg(baseName).arg(counter++).arg(suffix));
        }
    } while (QFile::exists(destPath));
    
    bool success = false;
    if (fi.isDir()) {
        QDir sourceDir(sourcePath);
        QDir destDir(destPath);
        success = copyDirectory(sourceDir, destDir);
    } else {
        success = QFile::copy(sourcePath, destPath);
    }
    
    if (success) {
        toast->showMessage(tr("Duplicated \"%1\"").arg(fi.fileName()));
        refresh();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not duplicate item"));
    }
}

bool SidebarFileExplorer::copyDirectory(const QDir &source, const QDir &destination)
{
    if (!destination.exists()) {
        if (!destination.mkpath(".")) return false;
    }
    
    QStringList files = source.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString &file : files) {
        if (!QFile::copy(source.filePath(file), destination.filePath(file))) {
            return false;
        }
    }
    
    QStringList dirs = source.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &dir : dirs) {
        QDir sourceSubDir(source.filePath(dir));
        QDir destinationSubDir(destination.filePath(dir));
        if (!copyDirectory(sourceSubDir, destinationSubDir)) {
            return false;
        }
    }
    
    return true;
}

void SidebarFileExplorer::onRevealInFileManager()
{
    QModelIndex index = fileTreeView->currentIndex();
    if (!index.isValid()) return;
    
    QString path = fileSystemModel->filePath(index);
    QFileInfo fi(path);
    
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.isDir() ? path : fi.absolutePath()));
}

void SidebarFileExplorer::onOpenContainingFolder()
{
    QString dirPath = currentPath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
}

// Keyboard shortcut handlers
void SidebarFileExplorer::handleF2()
{
    QModelIndex index = fileTreeView->currentIndex();
    if (index.isValid()) startRenameEditor(index);
}

void SidebarFileExplorer::handleDelete(bool shiftPressed)
{
    if (shiftPressed) {
        onDeletePermanently();
    } else {
        onDelete();
    }
}

void SidebarFileExplorer::handleCtrlD()
{
    onDuplicate();
}

void SidebarFileExplorer::handleCtrlC()
{
    onCopy();
}

void SidebarFileExplorer::handleCtrlX()
{
    onCut();
}

void SidebarFileExplorer::handleCtrlV()
{
    onPaste();
}
