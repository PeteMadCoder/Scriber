// mainwindow.cpp
#include "mainwindow.h"
#include "editorwidget.h"
#include "filemanager.h"
#include "markdownhighlighter.h"
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
#include <cmark.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      tabWidget(nullptr), fileManager(nullptr),
      findBarWidget(nullptr), findLineEdit(nullptr), findStatusLabel(nullptr),
      caseSensitiveCheckBox(nullptr), wholeWordsCheckBox(nullptr),
      findNextButton(nullptr), findPreviousButton(nullptr), closeFindBarButton(nullptr),
      sidebarTabs(nullptr), outlineTree(nullptr), outlineTimer(nullptr),

      newAct(this), openAct(this), saveAct(this), saveAsAct(this),
      exportHtmlAct(this), exportPdfAct(this), exitAct(this),
      toggleThemeAct(this), aboutAct(this), findAct(this), closeTabAct(this)
{
    // Create the tab widget
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);
    
    setCentralWidget(tabWidget);

    fileManager = new FileManager(this);

    resize(1200, 800);

    createActions();
    createSidebar();
    createMenus();
    createFindBar();
    createStatusBar();
    loadSettings();

    // Connect tab signals
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    // Initialize timers
    outlineTimer = new QTimer(this);
    outlineTimer->setSingleShot(true);
    outlineTimer->setInterval(1000);
    connect(outlineTimer, &QTimer::timeout, this, &MainWindow::updateOutline);

    // Create initial empty tab
    newFile();

    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
    // QPointer and Qt parent-child hierarchy handle cleanup automatically
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check all tabs for unsaved changes
    for (int i = 0; i < editorTabs.size(); ++i) {
        const EditorTab &tab = editorTabs[i];
        if (tab.editor && tab.editor->document()->isModified()) {
            tabWidget->setCurrentIndex(i);
            if (!maybeSaveCurrentTab()) {
                event->ignore();
                return;
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

void MainWindow::toggleTheme() {
    // Apply theme toggle to all editors
    for (int i = 0; i < editorTabs.size(); ++i) {
        if (editorTabs[i].editor) {
            editorTabs[i].editor->toggleTheme();
        }
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
    updateWindowTitle();
    updateWordCount();
    updateOutline();
}

void MainWindow::closeTab(int index)
{
    if (index < 0) return;
    
    EditorTab &tab = editorTabs[index];
    
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

    // Remove the tab
    editorTabs.removeAt(index);
    QWidget *widget = tabWidget->widget(index);
    tabWidget->removeTab(index);
    delete widget;

    // Update window title if we still have tabs
    if (tabWidget->count() == 0) {
        newFile(); // Create a new empty tab if all were closed
    } else {
        updateWindowTitle();
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
        setWindowTitle(tr("Scriber"));
        return;
    }

    const EditorTab &tab = editorTabs[currentIndex];
    QString fileName = tab.filePath.isEmpty() ? tr("Untitled") : tab.filePath;

    setWindowTitle(QString("%1 - %2").arg(fileName, QApplication::applicationName()));
}

void MainWindow::createStatusBar() {
    // Sidebar toggle button
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
    fileTreeView = new QTreeView(sidebarTabs);
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

    connect(fileTreeView, &QTreeView::doubleClicked, [this](const QModelIndex &index) {
        QString filePath = fileSystemModel->filePath(index);
        QFileInfo fileInfo(filePath);
        if (fileInfo.isFile()) {
            openFileInNewTab(filePath);
        }
    });

    sidebarTabs->addTab(fileTreeView, tr("Files"));

    // --- Tab 2: Outline ---
    outlineTree = new QTreeWidget(sidebarTabs);
    outlineTree->setHeaderHidden(true);
    outlineTree->setColumnCount(1);
    connect(outlineTree, &QTreeWidget::itemClicked, this, &MainWindow::onOutlineItemClicked);

    sidebarTabs->addTab(outlineTree, tr("Outline"));

    sidebarDock->setWidget(sidebarTabs);
    addDockWidget(Qt::LeftDockWidgetArea, sidebarDock);

    toggleSidebarAct = sidebarDock->toggleViewAction();
    toggleSidebarAct->setText(tr("&Sidebar"));
    toggleSidebarAct->setStatusTip(tr("Show or hide the sidebar"));
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

void MainWindow::createFindBar()
{
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

    toggleThemeAct.setText(tr("&Toggle Theme"));
    toggleThemeAct.setStatusTip(tr("Switch between light and dark themes"));
    connect(&toggleThemeAct, &QAction::triggered, this, &MainWindow::toggleTheme);

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
    viewMenu->addAction(&toggleThemeAct);
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
    
    QMainWindow::keyPressEvent(event);
}
