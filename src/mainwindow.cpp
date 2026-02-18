// mainwindow.cpp
#include "mainwindow.h"
#include "editorwidget.h"
#include "filemanager.h"
#include "markdownhighlighter.h"
#include "thememanager.h"
#include "themedialog.h"
#include "outlinedelegate.h"
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QStatusBar>
#include <QLabel>
#include <QActionGroup>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QDockWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QModelIndex>
#include <QMimeData>
#include <QDesktopServices>
#include <QUrl>
#include <QStorageInfo>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSet>
#include <cmark.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      tabWidget(nullptr), fileManager(nullptr),
      findBarWidget(nullptr), findLineEdit(nullptr), findStatusLabel(nullptr),
      caseSensitiveCheckBox(nullptr), wholeWordsCheckBox(nullptr),
      findNextButton(nullptr), findPreviousButton(nullptr), closeFindBarButton(nullptr),
      sidebarTabs(nullptr), outlineTree(nullptr), outlineTimer(nullptr),
      fileExplorerWidget(nullptr), fileExplorerLayout(nullptr), fileNavLayout(nullptr),
      parentDirButton(nullptr), currentPathEdit(nullptr), refreshButton(nullptr),
      fileContextMenu(nullptr),
      newFileAct(nullptr), newFolderAct(nullptr), renameAct(nullptr),
      deleteAct(nullptr), refreshAct(nullptr),
      cutFileAct(nullptr), copyFileAct(nullptr), pasteFileAct(nullptr),
      duplicateFileAct(nullptr), revealInFileManagerAct(nullptr),
      openContainingFolderAct(nullptr),
      inlineRenameEditor(nullptr), toastLabel(nullptr), toastTimer(nullptr)
{
    fileManager = new FileManager(this);

    resize(1200, 800);

    createActions();
    createSidebar();
    createMenus();
    createFindBar();  // This sets up tabWidget and central widget
    createStatusBar();
    loadSettings();

    // Initialize timers
    outlineTimer = new QTimer(this);
    outlineTimer->setSingleShot(true);
    outlineTimer->setInterval(1000);
    connect(outlineTimer, &QTimer::timeout, this, &MainWindow::updateOutline);

    // Initialize toast timer
    toastTimer = new QTimer(this);
    toastTimer->setSingleShot(true);
    connect(toastTimer, &QTimer::timeout, [this]() {
        if (toastLabel) toastLabel->hide();
    });

    // Create initial empty tab
    newFile();

    // Now connect tab signals (after initial tab is created)
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    statusBar()->showMessage(tr("Ready"));

    // Show window maximized
    showMaximized();
}

MainWindow::~MainWindow()
{
    // Disconnect signals to prevent callbacks during destruction
    for (const EditorTab &tab : editorTabs) {
        if (tab.editor) {
            disconnect(tab.editor->document(), nullptr, this, nullptr);
            disconnect(tab.editor, nullptr, this, nullptr);
        }
    }
    editorTabs.clear();
    
    // Qt's parent-child hierarchy will handle widget cleanup
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check all tabs for unsaved changes (iterate in reverse to handle potential removals)
    for (int i = editorTabs.size() - 1; i >= 0; --i) {
        const EditorTab &tab = editorTabs[i];
        if (tab.editor && tab.editor->document()->isModified()) {
            tabWidget->setCurrentIndex(i);
            const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Scriber"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            switch (ret) {
            case QMessageBox::Save:
                if (!save()) {
                    event->ignore();
                    return;
                }
                break;
            case QMessageBox::Cancel:
                event->ignore();
                return;
            default:
                break;
            }
        }
    }

    saveSettings();
    event->accept();
}

void MainWindow::newFile()
{
    // Create a new editor for the tab
    EditorWidget *newEditor = new EditorWidget(tabWidget);

    // Connect modification signal
    connect(newEditor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::documentWasModified);

    // Connect text changed for outline and word count updates
    connect(newEditor, &QPlainTextEdit::textChanged, [this]() {
        outlineTimer->start();
        wordCountTimer->start();
    });

    // Add to editorTabs FIRST, before adding to tabWidget
    // This ensures the list is ready when currentChanged signal fires
    EditorTab tab;
    tab.editor = newEditor;
    tab.filePath = QString();
    tab.isModified = false;
    editorTabs.append(tab);

    // Add the new tab
    int tabIndex = tabWidget->addTab(newEditor, tr("Untitled"));

    // Switch to the new tab
    tabWidget->setCurrentIndex(tabIndex);

    updateWindowTitle();
}

void MainWindow::open()
{
    if (maybeSaveCurrentTab()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Markdown File"), "",
                                                        tr("Markdown Files (*.md);;Text Files (*.txt);;All Files (*)"));
        if (!fileName.isEmpty()) {
            openFileInNewTab(fileName);
        }
    }
}

void MainWindow::openFileInNewTab(const QString &fileName)
{
    // Check if file is already open in a tab
    for (int i = 0; i < editorTabs.size(); ++i) {
        if (editorTabs[i].filePath == fileName) {
            tabWidget->setCurrentIndex(i);
            return;
        }
    }

    // Create a new editor for the tab
    EditorWidget *newEditor = new EditorWidget(tabWidget);

    // Load the file
    if (!fileManager->loadFile(fileName, newEditor)) {
        delete newEditor;
        return;
    }

    // Connect modification signal
    connect(newEditor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::documentWasModified);

    // Connect text changed for outline and word count updates
    connect(newEditor, &QPlainTextEdit::textChanged, [this]() {
        outlineTimer->start();
        wordCountTimer->start();
    });

    // Add to editorTabs FIRST, before adding to tabWidget
    EditorTab tab;
    tab.editor = newEditor;
    tab.filePath = fileName;
    tab.isModified = false;
    editorTabs.append(tab);

    // Add the new tab
    int tabIndex = tabWidget->addTab(newEditor, QFileInfo(fileName).fileName());

    // Switch to the new tab
    tabWidget->setCurrentIndex(tabIndex);

    updateWindowTitle();
}

bool MainWindow::save()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return false;
    
    EditorTab &tab = editorTabs[currentIndex];
    
    if (tab.filePath.isEmpty()) {
        return saveAs();
    }
    
    if (fileManager->saveFile(tab.filePath, tab.editor)) {
        tab.isModified = false;
        updateTabTitle(currentIndex);
        updateWindowTitle();
        return true;
    }
    return false;
}

bool MainWindow::saveAs()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return false;
    
    EditorTab &tab = editorTabs[currentIndex];
    
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save As"), "",
                                                    tr("Markdown Files (*.md);;Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty())
        return false;

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().isEmpty()) {
         fileName += ".md";
    }

    if (fileManager->saveFile(fileName, tab.editor)) {
        tab.filePath = fileName;
        tab.isModified = false;
        updateTabTitle(currentIndex);
        updateWindowTitle();
        return true;
    }
    return false;
}

void MainWindow::exportToHtml() {
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return;
    
    EditorTab &tab = editorTabs[currentIndex];
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to HTML"), QString(), tr("HTML Files (*.html)"));
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".html", Qt::CaseInsensitive))
            fileName += ".html";
        fileManager->exportToHtml(fileName, tab.editor);
    }
}

void MainWindow::exportToPdf() {
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return;
    
    EditorTab &tab = editorTabs[currentIndex];
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PDF"), QString(), tr("PDF Files (*.pdf)"));
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
            fileName += ".pdf";
        fileManager->exportToPdf(fileName, tab.editor);
    }
}

void MainWindow::selectTheme() {
    ThemeDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Theme already applied via preview in dialog
        // Just save the preference if needed
    }
}

void MainWindow::about()
{
     QMessageBox::about(this, tr("About Scriber"),
                        tr("<b>Scriber</b><br/>"
                           "A distraction-free Markdown editor.<br/>"
                           "Version %1")
                        .arg(QApplication::applicationVersion()));
}

void MainWindow::onTabChanged(int index)
{
    updateActionsState();
    if (index >= 0 && index < editorTabs.size()) {
        updateWindowTitle();
        updateWordCount();
        updateOutline();
    }
}

void MainWindow::closeTab(int index)
{
    if (index < 0 || index >= editorTabs.size()) return;

    EditorTab tab = editorTabs[index];  // Copy, not reference

    // Check for unsaved changes
    if (tab.editor && tab.editor->document()->isModified()) {
        tabWidget->setCurrentIndex(index);
        QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Scriber"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch (ret) {
        case QMessageBox::Save:
            if (!save()) return; // Save failed or cancelled
            break;
        case QMessageBox::Cancel:
            return;
        default:
            break;
        }
    }

    // Block signals to prevent recursive tabCloseRequested emissions
    tabWidget->blockSignals(true);
    
    // Get the widget BEFORE removing from lists
    QWidget *widget = tabWidget->widget(index);
    
    // Remove the tab from tabWidget first
    tabWidget->removeTab(index);
    
    // Then remove from our list
    editorTabs.removeAt(index);
    
    // Re-enable signals
    tabWidget->blockSignals(false);
    
    // Delete the widget
    delete widget;

    // Manually trigger updates since signals were blocked
    updateActionsState();
    
    // Update window title
    if (tabWidget->count() == 0) {
        // All tabs closed - just update the title, don't create a new tab
        setWindowTitle(QString("%1 - %2").arg(tr("No Open Files"), QApplication::applicationName()));
    } else {
        updateWindowTitle();
        updateWordCount();
        updateOutline();
    }
}

bool MainWindow::maybeSaveCurrentTab()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return true;
    
    const EditorTab &tab = editorTabs[currentIndex];
    
    if (!tab.editor || !tab.editor->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Scriber"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

bool MainWindow::maybeSave()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return true;
    
    const EditorTab &tab = editorTabs[currentIndex];
    
    if (!tab.editor || !tab.editor->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Scriber"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0) return;
    
    EditorTab &tab = editorTabs[currentIndex];
    tab.filePath = fileName;
    
    if (tab.editor) {
        tab.editor->document()->setModified(false);
    }
    
    updateTabTitle(currentIndex);
    updateWindowTitle();
    
    // Update sidebar to show the directory of the current file
    if (fileTreeView && fileSystemModel && !fileName.isEmpty()) {
        QFileInfo fi(fileName);
        QString dirPath = fi.absolutePath();
        fileTreeView->setRootIndex(fileSystemModel->setRootPath(dirPath));
    }
}

void MainWindow::updateTabTitle(int index)
{
    if (index < 0 || index >= editorTabs.size()) return;
    
    const EditorTab &tab = editorTabs[index];
    QString fileName = tab.filePath.isEmpty() ? tr("Untitled") : QFileInfo(tab.filePath).fileName();
    
    if (tab.editor && tab.editor->document()->isModified()) {
        fileName = "*" + fileName;
    }
    
    tabWidget->setTabText(index, fileName);
}

void MainWindow::updateWindowTitle()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= editorTabs.size()) {
        setWindowTitle(QString("%1 - %2").arg(tr("No Open Files"), QApplication::applicationName()));
        return;
    }

    const EditorTab &tab = editorTabs[currentIndex];
    QString fileName = tab.filePath.isEmpty() ? tr("Untitled") : tab.filePath;

    setWindowTitle(QString("%1 - %2").arg(fileName, QApplication::applicationName()));
}

void MainWindow::updateActionsState()
{
    bool hasTabs = editorTabs.size() > 0;
    saveAct.setEnabled(hasTabs);
    saveAsAct.setEnabled(hasTabs);
    exportHtmlAct.setEnabled(hasTabs);
    exportPdfAct.setEnabled(hasTabs);
    closeTabAct.setEnabled(hasTabs);
    findAct.setEnabled(hasTabs);
}

void MainWindow::createStatusBar() {
    // Sidebar toggle button - positioned on the left side of the status bar
    QPushButton *sidebarToggleBtn = new QPushButton(this);
    sidebarToggleBtn->setCheckable(true);
    sidebarToggleBtn->setFlat(true);
    sidebarToggleBtn->setToolTip(tr("Toggle Sidebar"));

    QIcon sidebarIcon = QIcon::fromTheme("view-sidebar", QIcon::fromTheme("sidebar", QIcon::fromTheme("format-justify-left")));
    if (sidebarIcon.isNull()) {
        sidebarToggleBtn->setText("Sidebar");
    } else {
        sidebarToggleBtn->setIcon(sidebarIcon);
    }

    if (sidebarDock) {
        sidebarToggleBtn->setChecked(sidebarDock->isVisible());
        connect(sidebarToggleBtn, &QPushButton::clicked, toggleSidebarAct, &QAction::trigger);
        connect(toggleSidebarAct, &QAction::toggled, sidebarToggleBtn, &QPushButton::setChecked);
    }

    statusBar()->insertPermanentWidget(0, sidebarToggleBtn);

    wordCountLabel = new QLabel(tr("Words: 0"));
    charCountLabel = new QLabel(tr("Chars: 0"));

    statusBar()->addPermanentWidget(wordCountLabel);
    statusBar()->addPermanentWidget(charCountLabel);

    // Toast notification label (overlay on status bar)
    toastLabel = new QLabel(statusBar());
    toastLabel->setStyleSheet(
        "QLabel { "
        "  background-color: #333333; "
        "  color: #ffffff; "
        "  padding: 8px 16px; "
        "  border-radius: 4px; "
        "  font-weight: bold;"
        "}"
    );
    toastLabel->hide();
    toastLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    wordCountTimer = new QTimer(this);
    wordCountTimer->setSingleShot(true);
    wordCountTimer->setInterval(1000);

    connect(wordCountTimer, &QTimer::timeout, this, &MainWindow::updateWordCount);

    updateWordCount();
}

void MainWindow::updateWordCount() {
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= editorTabs.size()) return;
    
    EditorWidget *currentEditor = editorTabs[currentIndex].editor;
    if (!currentEditor) return;
    
    int charCount = currentEditor->document()->characterCount() - 1;
    if (charCount < 0) charCount = 0;

    charCountLabel->setText(tr("Chars: %1").arg(charCount));

    if (charCount > 500000) {
        wordCountLabel->setText(tr("Words: ..."));
        return;
    }

    QString text = currentEditor->toPlainText();
    int wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    wordCountLabel->setText(tr("Words: %1").arg(wordCount));
}

void MainWindow::openFile(const QString &path)
{
    QString fileName = path;

    if (fileName.isEmpty()) {
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                              QString(),
                                              tr("Markdown Files (*.md *.markdown);;All Files (*)"));
    }

    if (!fileName.isEmpty()) {
        openFileInNewTab(fileName);
    }
}

void MainWindow::createSidebar()
{
    sidebarDock = new QDockWidget(tr("Sidebar"), this);
    sidebarDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    sidebarTabs = new QTabWidget(sidebarDock);
    sidebarTabs->setTabPosition(QTabWidget::South);

    // --- Tab 1: File Explorer ---
    fileExplorerWidget = new QWidget(sidebarTabs);
    fileExplorerLayout = new QVBoxLayout(fileExplorerWidget);
    fileExplorerLayout->setContentsMargins(0, 0, 0, 0);
    fileExplorerLayout->setSpacing(0);

    // Navigation toolbar
    fileNavLayout = new QHBoxLayout();
    fileNavLayout->setSpacing(2);

    // Parent directory button
    parentDirButton = new QPushButton();
    parentDirButton->setFixedSize(28, 28);
    parentDirButton->setToolTip(tr("Go to Parent Directory (Backspace)"));
    QIcon parentIcon = QIcon::fromTheme("go-up", QIcon::fromTheme("folder-open"));
    if (!parentIcon.isNull()) {
        parentDirButton->setIcon(parentIcon);
    } else {
        parentDirButton->setText("↑");
    }
    connect(parentDirButton, &QPushButton::clicked, this, &MainWindow::onParentDirectoryClicked);
    fileNavLayout->addWidget(parentDirButton);

    // Current path display
    currentPathEdit = new QLineEdit();
    currentPathEdit->setReadOnly(true);
    currentPathEdit->setPlaceholderText(tr("Current directory"));
    connect(currentPathEdit, &QLineEdit::editingFinished, [this]() {
        currentPathEdit->setReadOnly(true);
        onPathEdited(currentPathEdit->text());
    });
    fileNavLayout->addWidget(currentPathEdit);

    // Refresh button
    refreshButton = new QPushButton();
    refreshButton->setFixedSize(28, 28);
    refreshButton->setToolTip(tr("Refresh (F5)"));
    QIcon refreshIcon = QIcon::fromTheme("view-refresh", QIcon::fromTheme("reload"));
    if (!refreshIcon.isNull()) {
        refreshButton->setIcon(refreshIcon);
    } else {
        refreshButton->setText("⟳");
    }
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    fileNavLayout->addWidget(refreshButton);

    fileExplorerLayout->addLayout(fileNavLayout);

    // File tree view
    fileTreeView = new QTreeView(fileExplorerWidget);
    fileSystemModel = new QFileSystemModel(this);

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

    // VS Code-style features
    fileTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);  // Multi-select
    fileTreeView->setDragEnabled(true);
    fileTreeView->setAcceptDrops(true);
    fileTreeView->setDropIndicatorShown(true);
    fileTreeView->setDragDropMode(QAbstractItemView::DragDrop);
    fileTreeView->setDefaultDropAction(Qt::MoveAction);

    // Set initial path
    currentPathEdit->setText(initialPath);

    // Double-click to open files
    connect(fileTreeView, &QTreeView::doubleClicked, this, &MainWindow::onFileTreeDoubleClicked);

    // Context menu for file operations
    fileTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fileTreeView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::onFileTreeContextMenu);
    
    // Selection changed signal for multi-select operations
    connect(fileTreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    fileExplorerLayout->addWidget(fileTreeView);
    sidebarTabs->addTab(fileExplorerWidget, tr("Files"));

    // Create inline rename editor (hidden by default)
    inlineRenameEditor = new QLineEdit(fileTreeView);
    inlineRenameEditor->hide();
    inlineRenameEditor->setFrame(false);
    inlineRenameEditor->setStyleSheet(
        "QLineEdit { "
        "  background-color: #ffffff; "
        "  border: 2px solid #007fd4; "
        "  padding: 2px 4px; "
        "  font-weight: bold;"
        "}"
    );
    connect(inlineRenameEditor, &QLineEdit::editingFinished, this, &MainWindow::finishRenameEditor);
    connect(inlineRenameEditor, &QLineEdit::returnPressed, this, &MainWindow::finishRenameEditor);

    // Create context menu with VS Code-style organization
    fileContextMenu = new QMenu(fileTreeView);
    
    // New File/Folder section
    newFileAct = new QAction(tr("New File"), fileContextMenu);
    newFileAct->setIcon(QIcon::fromTheme("document-new", QIcon::fromTheme("text-x-generic")));
    newFileAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(newFileAct, &QAction::triggered, this, &MainWindow::onNewFile);

    newFolderAct = new QAction(tr("New Folder"), fileContextMenu);
    newFolderAct->setIcon(QIcon::fromTheme("folder-new", QIcon::fromTheme("folder")));
    newFolderAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    connect(newFolderAct, &QAction::triggered, this, &MainWindow::onNewFolder);

    fileContextMenu->addAction(newFileAct);
    fileContextMenu->addAction(newFolderAct);
    fileContextMenu->addSeparator();

    // Clipboard operations section
    cutFileAct = new QAction(tr("Cut"), fileContextMenu);
    cutFileAct->setIcon(QIcon::fromTheme("edit-cut"));
    cutFileAct->setShortcut(QKeySequence::Cut);
    connect(cutFileAct, &QAction::triggered, this, &MainWindow::onCutFile);

    copyFileAct = new QAction(tr("Copy"), fileContextMenu);
    copyFileAct->setIcon(QIcon::fromTheme("edit-copy"));
    copyFileAct->setShortcut(QKeySequence::Copy);
    connect(copyFileAct, &QAction::triggered, this, &MainWindow::onCopyFile);

    pasteFileAct = new QAction(tr("Paste"), fileContextMenu);
    pasteFileAct->setIcon(QIcon::fromTheme("edit-paste"));
    pasteFileAct->setShortcut(QKeySequence::Paste);
    connect(pasteFileAct, &QAction::triggered, this, &MainWindow::onPasteFile);

    duplicateFileAct = new QAction(tr("Duplicate"), fileContextMenu);
    duplicateFileAct->setIcon(QIcon::fromTheme("edit-copy"));
    duplicateFileAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(duplicateFileAct, &QAction::triggered, this, &MainWindow::onDuplicateFile);

    fileContextMenu->addAction(cutFileAct);
    fileContextMenu->addAction(copyFileAct);
    fileContextMenu->addAction(pasteFileAct);
    fileContextMenu->addAction(duplicateFileAct);
    fileContextMenu->addSeparator();

    // File operations section
    renameAct = new QAction(tr("Rename"), fileContextMenu);
    renameAct->setIcon(QIcon::fromTheme("edit-rename"));
    renameAct->setShortcut(QKeySequence(Qt::Key_F2));
    connect(renameAct, &QAction::triggered, [this]() {
        QModelIndex index = fileTreeView->currentIndex();
        if (index.isValid()) startRenameEditor(index);
    });

    deleteAct = new QAction(tr("Delete"), fileContextMenu);
    deleteAct->setIcon(QIcon::fromTheme("edit-delete"));
    deleteAct->setShortcut(QKeySequence::Delete);
    connect(deleteAct, &QAction::triggered, this, &MainWindow::onDeleteToTrash);

    fileContextMenu->addAction(renameAct);
    fileContextMenu->addAction(deleteAct);
    fileContextMenu->addSeparator();

    // Reveal/Open containing folder section
    revealInFileManagerAct = new QAction(tr("Reveal in File Manager"), fileContextMenu);
    revealInFileManagerAct->setIcon(QIcon::fromTheme("system-file-manager"));
#ifdef Q_OS_WIN
    revealInFileManagerAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
#else
    revealInFileManagerAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
#endif
    connect(revealInFileManagerAct, &QAction::triggered, this, &MainWindow::onRevealInFileManager);

    openContainingFolderAct = new QAction(tr("Open Containing Folder"), fileContextMenu);
    openContainingFolderAct->setIcon(QIcon::fromTheme("folder-open"));
    connect(openContainingFolderAct, &QAction::triggered, this, &MainWindow::onOpenContainingFolder);

    fileContextMenu->addAction(revealInFileManagerAct);
    fileContextMenu->addAction(openContainingFolderAct);
    fileContextMenu->addSeparator();

    // Refresh
    refreshAct = new QAction(tr("Refresh"), fileContextMenu);
    refreshAct->setIcon(QIcon::fromTheme("view-refresh"));
    refreshAct->setShortcut(QKeySequence(Qt::Key_F5));
    connect(refreshAct, &QAction::triggered, this, &MainWindow::onRefresh);

    fileContextMenu->addAction(refreshAct);

    // --- Tab 2: Outline ---
    outlineTree = new QTreeWidget(sidebarTabs);
    outlineTree->setHeaderHidden(true);
    outlineTree->setColumnCount(1);
    connect(outlineTree, &QTreeWidget::itemClicked, this, &MainWindow::onOutlineItemClicked);
    
    // Set up custom delegate for theme-aware arrow icons
    outlineDelegate = new OutlineDelegate(outlineTree);
    outlineTree->setItemDelegate(outlineDelegate);
    
    // Connect to theme changes to update arrow colors
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &MainWindow::updateOutlineTreeStyle);
    updateOutlineTreeStyle();

    sidebarTabs->addTab(outlineTree, tr("Outline"));

    sidebarDock->setWidget(sidebarTabs);
    addDockWidget(Qt::LeftDockWidgetArea, sidebarDock);

    toggleSidebarAct = sidebarDock->toggleViewAction();
    toggleSidebarAct->setText(tr("&Sidebar"));
    toggleSidebarAct->setStatusTip(tr("Show or hide the sidebar"));
}

void MainWindow::updateOutlineTreeStyle()
{
    if (!outlineDelegate || !outlineTree) return;
    
    // Get the current text color from the theme and use it for arrows
    ThemeManager *themeManager = ThemeManager::instance();
    outlineDelegate->setArrowColor(themeManager->textColor());
    
    // Force the tree to repaint
    outlineTree->viewport()->update();
}

void MainWindow::updateOutline()
{
    if (!outlineTree) return;
    
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= editorTabs.size()) return;
    
    EditorWidget *currentEditor = editorTabs[currentIndex].editor;
    if (!currentEditor) return;

    outlineTree->clear();

    QString markdown = currentEditor->toPlainText();
    QByteArray utf8 = markdown.toUtf8();

    cmark_node *doc = cmark_parse_document(utf8.constData(), utf8.size(), CMARK_OPT_DEFAULT);
    if (!doc) return;

    cmark_iter *iter = cmark_iter_new(doc);
    cmark_event_type ev_type;

    QList<QTreeWidgetItem*> parents;
    parents.append(nullptr);

    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        cmark_node *cur = cmark_iter_get_node(iter);

        if (ev_type == CMARK_EVENT_ENTER && cmark_node_get_type(cur) == CMARK_NODE_HEADING) {
            int level = cmark_node_get_heading_level(cur);
            int startLine = cmark_node_get_start_line(cur);

            QString headingText;
            cmark_iter *subIter = cmark_iter_new(cur);
            while (cmark_iter_next(subIter) != CMARK_EVENT_DONE) {
                cmark_node *subNode = cmark_iter_get_node(subIter);
                if (cmark_node_get_type(subNode) == CMARK_NODE_TEXT ||
                    cmark_node_get_type(subNode) == CMARK_NODE_CODE) {
                    const char *text = cmark_node_get_literal(subNode);
                    if (text) headingText += QString::fromUtf8(text);
                }
            }
            cmark_iter_free(subIter);

            if (headingText.isEmpty()) headingText = tr("(Empty Heading)");

            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, headingText);
            item->setData(0, Qt::UserRole, startLine);

            while (parents.size() <= level) parents.append(nullptr);
            while (parents.size() > level + 1) parents.removeLast();

            QTreeWidgetItem *parentItem = nullptr;
            if (level > 0 && level < parents.size()) {
                parentItem = parents[level - 1];
            }

            if (parentItem) {
                parentItem->addChild(item);
            } else {
                outlineTree->addTopLevelItem(item);
            }

            if (parents.size() > level) {
                parents[level] = item;
            } else {
                parents.append(item);
            }

            item->setExpanded(true);
        }
    }

    cmark_iter_free(iter);
    cmark_node_free(doc);
}

void MainWindow::onOutlineItemClicked(QTreeWidgetItem *item, int column)
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= editorTabs.size()) return;

    EditorWidget *currentEditor = editorTabs[currentIndex].editor;
    if (!currentEditor) return;

    int line = item->data(0, Qt::UserRole).toInt();
    if (line > 0) {
        QTextCursor cursor = currentEditor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
        currentEditor->setTextCursor(cursor);
        currentEditor->centerCursor();
        currentEditor->setFocus();
    }
}

// ============================================================================
// Enhanced Sidebar Methods
// ============================================================================

void MainWindow::onParentDirectoryClicked()
{
    QModelIndex currentIndex = fileTreeView->rootIndex();
    if (currentIndex.isValid()) {
        QModelIndex parentIndex = currentIndex.parent();
        if (parentIndex.isValid()) {
            fileTreeView->setRootIndex(parentIndex);
        } else {
            // Already at root, try to go to parent directory
            QString currentPath = fileSystemModel->filePath(currentIndex);
            QFileInfo fi(currentPath);
            QString parentPath = fi.absolutePath();
            if (parentPath != currentPath) {
                QModelIndex parentIdx = fileSystemModel->index(parentPath);
                if (parentIdx.isValid()) {
                    fileTreeView->setRootIndex(parentIdx);
                }
            }
        }
    }
}

void MainWindow::onPathEdited(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists() && fi.isDir()) {
        QModelIndex index = fileSystemModel->index(path);
        if (index.isValid()) {
            fileTreeView->setRootIndex(index);
        }
    } else {
        // Invalid path, revert to current
        QModelIndex currentIndex = fileTreeView->rootIndex();
        if (currentIndex.isValid()) {
            currentPathEdit->setText(fileSystemModel->filePath(currentIndex));
        } else {
            currentPathEdit->setText(fileSystemModel->rootPath());
        }
    }
}

void MainWindow::onRefreshClicked()
{
    onRefresh();
}

void MainWindow::onRefresh()
{
    QModelIndex currentIndex = fileTreeView->rootIndex();
    if (currentIndex.isValid()) {
        fileSystemModel->setRootPath(fileSystemModel->filePath(currentIndex));
        fileTreeView->setRootIndex(currentIndex);
    } else {
        QString rootPath = fileSystemModel->rootPath();
        fileSystemModel->setRootPath(rootPath);
        fileTreeView->setRootIndex(fileSystemModel->index(rootPath));
    }
}

void MainWindow::onFileTreeContextMenu(const QPoint &pos)
{
    QModelIndex index = fileTreeView->indexAt(pos);
    
    // Select the item under cursor if not already selected
    if (index.isValid() && !fileTreeView->selectionModel()->isSelected(index)) {
        fileTreeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    }
    
    // Update actions based on current selection
    onSelectionChanged();

    // Show context menu
    fileContextMenu->exec(fileTreeView->viewport()->mapToGlobal(pos));
}

void MainWindow::onNewFile()
{
    QModelIndex currentIndex = fileTreeView->rootIndex();
    QString dirPath;
    
    if (currentIndex.isValid()) {
        dirPath = fileSystemModel->filePath(currentIndex);
    } else {
        dirPath = fileSystemModel->rootPath();
    }
    
    // Create a new file with a default name
    QString baseName = "untitled.md";
    QString filePath = QDir(dirPath).filePath(baseName);
    
    // Ensure unique filename
    int counter = 1;
    while (QFile::exists(filePath)) {
        baseName = QString("untitled%1.md").arg(counter++);
        filePath = QDir(dirPath).filePath(baseName);
    }
    
    // Create the file
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.close();
        
        // Refresh the view
        onRefresh();
        
        // Open the new file in editor
        openFileInNewTab(filePath);
    } else {
        QMessageBox::warning(this, tr("Error"),
                            tr("Could not create file: %1").arg(file.errorString()));
    }
}

void MainWindow::onNewFolder()
{
    QModelIndex currentIndex = fileTreeView->rootIndex();
    QString dirPath;
    
    if (currentIndex.isValid()) {
        dirPath = fileSystemModel->filePath(currentIndex);
    } else {
        dirPath = fileSystemModel->rootPath();
    }
    
    // Create a new folder with a default name
    QString baseName = "New Folder";
    QString folderPath = QDir(dirPath).filePath(baseName);
    
    // Ensure unique folder name
    int counter = 1;
    while (QDir(folderPath).exists()) {
        baseName = QString("New Folder %1").arg(counter++);
        folderPath = QDir(dirPath).filePath(baseName);
    }
    
    // Create the directory
    if (QDir().mkdir(folderPath)) {
        // Refresh the view
        onRefresh();
    } else {
        QMessageBox::warning(this, tr("Error"),
                            tr("Could not create folder: %1").arg(folderPath));
    }
}

void MainWindow::onRename()
{
    QModelIndex index = fileTreeView->currentIndex();
    if (!index.isValid()) return;
    
    QString oldPath = fileSystemModel->filePath(index);
    QFileInfo fi(oldPath);
    
    // Show input dialog for new name
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename"),
                                            tr("Enter new name:"),
                                            QLineEdit::Normal,
                                            fi.fileName(), &ok);
    
    if (ok && !newName.isEmpty() && newName != fi.fileName()) {
        QString newPath = QDir(fi.absolutePath()).filePath(newName);
        
        if (QFile::rename(oldPath, newPath)) {
            onRefresh();
        } else {
            QMessageBox::warning(this, tr("Error"),
                                tr("Could not rename file/folder."));
        }
    }
}

void MainWindow::onDelete()
{
    QModelIndex index = fileTreeView->currentIndex();
    if (!index.isValid()) return;
    
    QString path = fileSystemModel->filePath(index);
    QFileInfo fi(path);
    
    // Confirm deletion
    QString itemType = fi.isDir() ? tr("folder") : tr("file");
    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("Confirm Delete"),
        tr("Are you sure you want to delete the %1 \"%2\"?\n"
           "This action cannot be undone.").arg(itemType, fi.fileName()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    bool success = false;
    if (fi.isDir()) {
        // Remove directory and its contents
        QDir dir(path);
        success = dir.removeRecursively();
    } else {
        // Remove file
        success = QFile::remove(path);
    }
    
    if (!success) {
        QMessageBox::warning(this, tr("Error"),
                            tr("Could not delete %1").arg(fi.fileName()));
    } else {
        onRefresh();
    }
}

void MainWindow::onDirectoryChanged(const QString &path)
{
    // This slot can be used to track directory changes if needed
    Q_UNUSED(path);
}

// ============================================================================
// VS Code-Style File Management Methods
// ============================================================================

void MainWindow::onFileTreeDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    QString filePath = fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);
    if (fileInfo.isDir()) {
        fileTreeView->setRootIndex(index);
        currentPathEdit->setText(filePath);
    } else {
        openFileInNewTab(filePath);
    }
}

void MainWindow::onSelectionChanged()
{
    // Update context menu actions based on selection
    // Only count first column indexes to avoid duplicates from multi-column model
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    int selectedCount = 0;
    for (const QModelIndex &index : selectedIndices) {
        if (index.column() == 0) selectedCount++;
    }
    
    // Enable/disable actions based on selection
    renameAct->setEnabled(selectedCount == 1);
    deleteAct->setEnabled(selectedCount > 0);
    cutFileAct->setEnabled(selectedCount > 0);
    copyFileAct->setEnabled(selectedCount > 0);
    duplicateFileAct->setEnabled(selectedCount == 1);
    revealInFileManagerAct->setEnabled(selectedCount == 1);
    openContainingFolderAct->setEnabled(selectedCount > 0);
    
    // Paste is enabled if we have something in clipboard
    pasteFileAct->setEnabled(!fileClipboard.paths.isEmpty());
}

void MainWindow::startRenameEditor(const QModelIndex &index)
{
    if (!index.isValid() || !inlineRenameEditor) return;
    
    renameEditorIndex = index;
    QString oldPath = fileSystemModel->filePath(index);
    QFileInfo fi(oldPath);
    
    // Get the visual rectangle for the item
    QRect rect = fileTreeView->visualRect(index);
    if (!rect.isValid()) return;
    
    // Get theme colors for the rename editor styling
    ThemeManager *themeManager = ThemeManager::instance();
    QColor bgColor = themeManager->backgroundColor();
    QColor textColor = themeManager->textColor();
    QColor highlightColor = themeManager->highlightColor();
    
    // Position the editor over the item
    inlineRenameEditor->setGeometry(rect);
    inlineRenameEditor->setText(fi.fileName());
    inlineRenameEditor->selectAll();
    
    // Theme-aware stylesheet
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

void MainWindow::finishRenameEditor()
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
        // Name didn't change
        renameEditorIndex = QModelIndex();
        return;
    }
    
    QString newPath = QDir(fi.absolutePath()).filePath(newName);
    
    if (QFile::rename(oldPath, newPath)) {
        showToast(tr("Renamed to \"%1\"").arg(newName));
        onRefresh();
    } else {
        QMessageBox::warning(this, tr("Error"),
                            tr("Could not rename file/folder to \"%1\"").arg(newName));
    }
    
    renameEditorIndex = QModelIndex();
}

void MainWindow::cancelRenameEditor()
{
    if (inlineRenameEditor) {
        inlineRenameEditor->hide();
    }
    renameEditorIndex = QModelIndex();
}

void MainWindow::onDeleteToTrash()
{
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    if (selectedIndices.isEmpty()) return;
    
    // Collect all unique paths to delete (only use first column to avoid duplicates)
    QSet<QString> uniquePaths;
    for (const QModelIndex &index : selectedIndices) {
        if (index.isValid() && index.column() == 0) {
            uniquePaths << fileSystemModel->filePath(index);
        }
    }
    
    QStringList pathsToDelete = uniquePaths.values();
    if (pathsToDelete.isEmpty()) return;
    
    // Confirm deletion
    int fileCount = 0;
    int dirCount = 0;
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
        this, tr("Confirm Delete"),
        message,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    // Delete files/folders
    int successCount = 0;
    for (const QString &path : pathsToDelete) {
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
        showToast(tr("Deleted %1 item(s)").arg(successCount));
        onRefresh();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not delete items"));
    }
}

void MainWindow::onDeletePermanently()
{
    // For Shift+Delete - permanent deletion (more dangerous)
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    if (selectedIndices.isEmpty()) return;
    
    // Collect all unique paths to delete (only use first column to avoid duplicates)
    QSet<QString> uniquePaths;
    for (const QModelIndex &index : selectedIndices) {
        if (index.isValid() && index.column() == 0) {
            uniquePaths << fileSystemModel->filePath(index);
        }
    }
    
    QStringList pathsToDelete = uniquePaths.values();
    if (pathsToDelete.isEmpty()) return;
    
    // Stronger warning for permanent deletion
    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("Permanently Delete"),
        tr("Are you sure you want to PERMANENTLY delete these items?\nThis action cannot be undone!"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    // Delete files/folders
    int successCount = 0;
    for (const QString &path : pathsToDelete) {
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
        showToast(tr("Permanently deleted %1 item(s)").arg(successCount));
        onRefresh();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not delete items"));
    }
}

void MainWindow::onCutFile()
{
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    if (selectedIndices.isEmpty()) return;
    
    fileClipboard.paths.clear();
    for (const QModelIndex &index : selectedIndices) {
        if (index.isValid() && index.column() == 0) {
            fileClipboard.paths << fileSystemModel->filePath(index);
        }
    }
    fileClipboard.isCut = true;
    
    showToast(tr("Cut %1 file(s)").arg(fileClipboard.paths.size()));
}

void MainWindow::onCopyFile()
{
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    if (selectedIndices.isEmpty()) return;
    
    fileClipboard.paths.clear();
    for (const QModelIndex &index : selectedIndices) {
        if (index.isValid() && index.column() == 0) {
            fileClipboard.paths << fileSystemModel->filePath(index);
        }
    }
    fileClipboard.isCut = false;
    
    showToast(tr("Copied %1 file(s)").arg(fileClipboard.paths.size()));
}

void MainWindow::onPasteFile()
{
    if (fileClipboard.paths.isEmpty()) return;
    
    // Get destination directory
    QModelIndex currentIndex = fileTreeView->rootIndex();
    QString destDirPath;
    if (currentIndex.isValid()) {
        destDirPath = fileSystemModel->filePath(currentIndex);
    } else {
        destDirPath = fileSystemModel->rootPath();
    }
    
    int pasteCount = 0;
    for (const QString &sourcePath : fileClipboard.paths) {
        QFileInfo fi(sourcePath);
        QString destPath = QDir(destDirPath).filePath(fi.fileName());
        
        // Ensure unique filename if already exists
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
        if (fileClipboard.isCut) {
            // Move operation
            success = QFile::rename(sourcePath, destPath);
        } else {
            // Copy operation
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
        showToast(tr("Pasted %1 item(s)").arg(pasteCount));
        onRefresh();
        
        // Clear clipboard if it was a cut operation
        if (fileClipboard.isCut) {
            fileClipboard.paths.clear();
            fileClipboard.isCut = false;
        }
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not paste items"));
    }
}

bool MainWindow::copyDirectory(const QDir &source, const QDir &destination)
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

void MainWindow::onDuplicateFile()
{
    QModelIndex index = fileTreeView->currentIndex();
    if (!index.isValid()) return;
    
    QString sourcePath = fileSystemModel->filePath(index);
    QFileInfo fi(sourcePath);
    
    // Create duplicate name
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
        showToast(tr("Duplicated \"%1\"").arg(fi.fileName()));
        onRefresh();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not duplicate item"));
    }
}

void MainWindow::onRevealInFileManager()
{
    QModelIndex index = fileTreeView->currentIndex();
    if (!index.isValid()) return;
    
    QString path = fileSystemModel->filePath(index);
    QFileInfo fi(path);
    
    // Open the containing folder and select the file
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.isDir() ? path : fi.absolutePath()));
}

void MainWindow::onOpenContainingFolder()
{
    QModelIndexList selectedIndices = fileTreeView->selectionModel()->selectedIndexes();
    if (selectedIndices.isEmpty()) return;
    
    // Just open the current directory in file manager
    QModelIndex currentIndex = fileTreeView->rootIndex();
    QString dirPath;
    if (currentIndex.isValid()) {
        dirPath = fileSystemModel->filePath(currentIndex);
    } else {
        dirPath = fileSystemModel->rootPath();
    }
    
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
}

void MainWindow::showToast(const QString &message, int durationMs)
{
    if (!toastLabel) return;
    
    toastLabel->setText(message);
    toastLabel->adjustSize();
    
    // Position the toast at the bottom center of the window
    int x = (width() - toastLabel->width()) / 2;
    int y = height() - toastLabel->height() - 50;  // 50px from bottom
    toastLabel->move(x, y);
    
    toastLabel->show();
    toastLabel->raise();
    
    // Fade in animation
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(toastLabel);
    toastLabel->setGraphicsEffect(effect);
    QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start();
    
    // Auto-hide after duration
    toastTimer->setInterval(durationMs);
    toastTimer->start();
}

void MainWindow::createFindBar()
{
    // Create the tab widget first
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);

    findBarWidget = new QWidget(this);
    findBarWidget->setObjectName("findBar");
    findBarWidget->setVisible(false);
    isFindBarVisible = false;

    QLabel *findLabel = new QLabel(tr("Find:"), findBarWidget);
    findLineEdit = new QLineEdit(findBarWidget);
    findStatusLabel = new QLabel(findBarWidget);
    findStatusLabel->setMinimumWidth(150);
    findStatusLabel->setStyleSheet("QLabel { color : gray; }");
    findStatusLabel->setText("");

    caseSensitiveCheckBox = new QCheckBox(tr("Case sensitive"), findBarWidget);
    wholeWordsCheckBox = new QCheckBox(tr("Whole words"), findBarWidget);

    findNextButton = new QPushButton(tr("Next"), findBarWidget);
    findPreviousButton = new QPushButton(tr("Previous"), findBarWidget);
    closeFindBarButton = new QPushButton(tr("Close"), findBarWidget);
    closeFindBarButton->setFixedSize(24, 24);
    closeFindBarButton->setText("X");
    closeFindBarButton->setToolTip(tr("Close find bar (Escape)"));

    QHBoxLayout *optionsLayout = new QHBoxLayout;
    optionsLayout->addWidget(caseSensitiveCheckBox);
    optionsLayout->addWidget(wholeWordsCheckBox);
    optionsLayout->addStretch();

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(findStatusLabel);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(findNextButton);
    buttonsLayout->addWidget(findPreviousButton);
    buttonsLayout->addWidget(closeFindBarButton);

    QVBoxLayout *findBarLayout = new QVBoxLayout(findBarWidget);
    findBarLayout->setContentsMargins(5, 5, 5, 5);
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(findLabel);
    topLayout->addWidget(findLineEdit);
    topLayout->addLayout(optionsLayout);
    findBarLayout->addLayout(topLayout);
    findBarLayout->addLayout(buttonsLayout);

    QWidget *centralContainer = new QWidget(this);
    QVBoxLayout *centralLayout = new QVBoxLayout(centralContainer);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(tabWidget);
    centralLayout->addWidget(findBarWidget);

    setCentralWidget(centralContainer);

    connect(findLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onFindNext);
    connect(findNextButton, &QPushButton::clicked, this, &MainWindow::onFindNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &MainWindow::onFindPrevious);
    connect(closeFindBarButton, &QPushButton::clicked, this, &MainWindow::hideFindBar);
    connect(findLineEdit, &QLineEdit::textEdited, this, &MainWindow::onFindTextEdited);
}

void MainWindow::find()
{
    if (!isFindBarVisible) {
        findBarWidget->setVisible(true);
        isFindBarVisible = true;
        findLineEdit->setFocus();
        findLineEdit->selectAll();
    }
}

void MainWindow::hideFindBar()
{
    findBarWidget->setVisible(false);
    isFindBarVisible = false;
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0 && currentIndex < editorTabs.size()) {
        editorTabs[currentIndex].editor->setFocus();
    }
}

void MainWindow::onFindTextEdited()
{
    onFindNext();
}

void MainWindow::onFindNext()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= editorTabs.size()) return;
    
    EditorWidget *currentEditor = editorTabs[currentIndex].editor;
    if (!currentEditor) return;
    
    QString searchText = findLineEdit->text();
    if (searchText.isEmpty()) {
        findStatusLabel->setText("");
        return;
    }

    Qt::CaseSensitivity caseSensitivity = caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool wholeWords = wholeWordsCheckBox->isChecked();

    QTextDocument::FindFlags flags;
    if (caseSensitivity == Qt::CaseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    bool found = false;
    QTextCursor cursor;
    if (wholeWords) {
        QRegularExpression regex("\\b" + QRegularExpression::escape(searchText) + "\\b",
                                  caseSensitivity == Qt::CaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        cursor = currentEditor->document()->find(regex, currentEditor->textCursor());
        found = !cursor.isNull();
    } else {
        found = currentEditor->find(searchText, flags);
        cursor = currentEditor->textCursor();
    }

    if (found) {
        currentEditor->setTextCursor(cursor);
        currentEditor->ensureCursorVisible();
        findStatusLabel->setText(tr("Found"));
    } else {
        findStatusLabel->setText(tr("Not found"));
    }
}

void MainWindow::onFindPrevious()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= editorTabs.size()) return;
    
    EditorWidget *currentEditor = editorTabs[currentIndex].editor;
    if (!currentEditor) return;
    
    QString searchText = findLineEdit->text();
    if (searchText.isEmpty()) {
        findStatusLabel->setText("");
        return;
    }

    Qt::CaseSensitivity caseSensitivity = caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool wholeWords = wholeWordsCheckBox->isChecked();

    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    if (caseSensitivity == Qt::CaseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    bool found = false;
    if (wholeWords) {
        QRegularExpression regex("\\b" + QRegularExpression::escape(searchText) + "\\b",
                                  caseSensitivity == Qt::CaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        QTextCursor cursor = currentEditor->textCursor();
        cursor = currentEditor->document()->find(regex, cursor, flags);
        found = !cursor.isNull();
        if (found) {
            currentEditor->setTextCursor(cursor);
        }
    } else {
        found = currentEditor->find(searchText, flags);
    }

    if (found) {
        findStatusLabel->setText(tr("Found"));
    } else {
        findStatusLabel->setText(tr("Not found"));
    }
}

void MainWindow::documentWasModified()
{
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= editorTabs.size()) return;
    
    EditorWidget *currentEditor = editorTabs[currentIndex].editor;
    if (!currentEditor) return;
    
    bool modified = currentEditor->document()->isModified();
    editorTabs[currentIndex].isModified = modified;
    
    updateTabTitle(currentIndex);
    updateWindowTitle();
}

void MainWindow::createActions()
{
    newAct.setText(tr("&New"));
    newAct.setShortcuts(QKeySequence::New);
    newAct.setStatusTip(tr("Create a new file"));
    connect(&newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct.setText(tr("&Open..."));
    openAct.setShortcuts(QKeySequence::Open);
    openAct.setStatusTip(tr("Open an existing file"));
    connect(&openAct, &QAction::triggered, this, &MainWindow::open);

    saveAct.setText(tr("&Save"));
    saveAct.setShortcuts(QKeySequence::Save);
    saveAct.setStatusTip(tr("Save the document to disk"));
    connect(&saveAct, &QAction::triggered, this, &MainWindow::save);

    saveAsAct.setText(tr("Save &As..."));
    saveAsAct.setShortcuts(QKeySequence::SaveAs);
    saveAsAct.setStatusTip(tr("Save the document under a new name"));
    connect(&saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);

    exportHtmlAct.setText(tr("Export to &HTML..."));
    exportHtmlAct.setStatusTip(tr("Export the document to HTML"));
    connect(&exportHtmlAct, &QAction::triggered, this, &MainWindow::exportToHtml);

    exportPdfAct.setText(tr("Export to &PDF..."));
    exportPdfAct.setStatusTip(tr("Export the document to PDF"));
    connect(&exportPdfAct, &QAction::triggered, this, &MainWindow::exportToPdf);

    exitAct.setText(tr("E&xit"));
    exitAct.setShortcuts(QKeySequence::Quit);
    exitAct.setStatusTip(tr("Exit the application"));
    connect(&exitAct, &QAction::triggered, this, &MainWindow::close);

    selectThemeAct.setText(tr("&Select Theme..."));
    selectThemeAct.setStatusTip(tr("Choose application theme (Light, Dark, Pitch Black)"));
    connect(&selectThemeAct, &QAction::triggered, this, &MainWindow::selectTheme);

    aboutAct.setText(tr("&About"));
    aboutAct.setStatusTip(tr("Show the application's About box"));
    connect(&aboutAct, &QAction::triggered, this, &MainWindow::about);

    findAct.setText(tr("&Find"));
    findAct.setShortcuts(QKeySequence::Find);
    findAct.setStatusTip(tr("Find text in the document"));
    connect(&findAct, &QAction::triggered, this, &MainWindow::find);
    addAction(&findAct);

    closeTabAct.setText(tr("&Close Tab"));
    closeTabAct.setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
    closeTabAct.setStatusTip(tr("Close the current tab"));
    connect(&closeTabAct, &QAction::triggered, [this]() {
        closeTab(tabWidget->currentIndex());
    });
    addAction(&closeTabAct);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(&newAct);
    fileMenu->addAction(&openAct);
    fileMenu->addAction(&saveAct);
    fileMenu->addAction(&saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(&exportHtmlAct);
    fileMenu->addAction(&exportPdfAct);
    fileMenu->addSeparator();
    fileMenu->addAction(&exitAct);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(&findAct);
    editMenu->addAction(&closeTabAct);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(&selectThemeAct);
    viewMenu->addAction(toggleSidebarAct);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));

    QAction *spellCheckToggleAction = new QAction(tr("&Spell Check"), this);
    spellCheckToggleAction->setCheckable(true);
    spellCheckToggleAction->setChecked(true);
    connect(spellCheckToggleAction, &QAction::toggled, [this](bool checked) {
        int currentIndex = tabWidget->currentIndex();
        if (currentIndex >= 0 && currentIndex < editorTabs.size()) {
            editorTabs[currentIndex].editor->setSpellCheckEnabled(checked);
        }
    });
    toolsMenu->addAction(spellCheckToggleAction);

    QMenu *languageMenu = toolsMenu->addMenu(tr("&Language"));

    struct LanguageInfo {
        QString code;
        QString name;
    };
    const QList<LanguageInfo> languages = {
        {"en_US", "English (US)"},
        {"en_GB", "English (UK)"},
        {"fr_FR", "Français"},
        {"de_DE", "Deutsch"},
        {"es_ES", "Español"},
        {"pt_PT", "Português (Portugal)"},
        {"pt_BR", "Português (Brasil)"},
    };

    QActionGroup *languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);

    for (const LanguageInfo &langInfo : languages) {
        QAction *langAction = new QAction(langInfo.name, this);
        langAction->setCheckable(true);
        langAction->setData(langInfo.code);
        languageGroup->addAction(langAction);
        languageMenu->addAction(langAction);

        if (langInfo.code == "en_US") {
             langAction->setChecked(true);
        }

        connect(langAction, &QAction::triggered, [this, langInfo]() {
            int currentIndex = tabWidget->currentIndex();
            if (currentIndex >= 0 && currentIndex < editorTabs.size()) {
                editorTabs[currentIndex].editor->setSpellCheckLanguage(langInfo.code);
            }
        });
    }

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(&aboutAct);
}

void MainWindow::loadSettings()
{
    QSettings settings;
}

void MainWindow::saveSettings()
{
    QSettings settings;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Handle Ctrl+Tab for next tab
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Tab) {
        int currentIndex = tabWidget->currentIndex();
        int nextIndex = (currentIndex + 1) % tabWidget->count();
        tabWidget->setCurrentIndex(nextIndex);
        event->accept();
        return;
    }

    // Handle Ctrl+Shift+Tab for previous tab
    if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) && event->key() == Qt::Key_Tab) {
        int currentIndex = tabWidget->currentIndex();
        int prevIndex = (currentIndex - 1 + tabWidget->count()) % tabWidget->count();
        tabWidget->setCurrentIndex(prevIndex);
        event->accept();
        return;
    }

    // Handle Escape to close find bar
    if (event->key() == Qt::Key_Escape && isFindBarVisible) {
        hideFindBar();
        event->accept();
        return;
    }
    
    // Handle Escape to cancel inline rename
    if (event->key() == Qt::Key_Escape && inlineRenameEditor && inlineRenameEditor->isVisible()) {
        cancelRenameEditor();
        event->accept();
        return;
    }

    // Handle Backspace for parent directory (when sidebar has focus)
    if (event->key() == Qt::Key_Backspace && fileTreeView && fileTreeView->hasFocus()) {
        onParentDirectoryClicked();
        event->accept();
        return;
    }

    // Handle F5 for refresh (when sidebar has focus)
    if (event->key() == Qt::Key_F5 && fileTreeView && fileTreeView->hasFocus()) {
        onRefreshClicked();
        event->accept();
        return;
    }
    
    // Handle F2 for rename (when sidebar has focus)
    if (event->key() == Qt::Key_F2 && fileTreeView && fileTreeView->hasFocus()) {
        QModelIndex index = fileTreeView->currentIndex();
        if (index.isValid()) {
            startRenameEditor(index);
        }
        event->accept();
        return;
    }
    
    // Handle Delete for delete (when sidebar has focus)
    if (event->key() == Qt::Key_Delete && fileTreeView && fileTreeView->hasFocus()) {
        if (event->modifiers() == Qt::ShiftModifier) {
            // Shift+Delete = permanent delete
            onDeletePermanently();
        } else {
            // Regular delete = move to trash
            onDeleteToTrash();
        }
        event->accept();
        return;
    }
    
    // Handle Ctrl+D for duplicate (when sidebar has focus)
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_D && 
        fileTreeView && fileTreeView->hasFocus()) {
        onDuplicateFile();
        event->accept();
        return;
    }
    
    // Handle Ctrl+C/X/V for clipboard operations (when sidebar has focus)
    if (fileTreeView && fileTreeView->hasFocus()) {
        if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
            onCopyFile();
            event->accept();
            return;
        }
        if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_X) {
            onCutFile();
            event->accept();
            return;
        }
        if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_V) {
            onPasteFile();
            event->accept();
            return;
        }
    }

    QMainWindow::keyPressEvent(event);
}
