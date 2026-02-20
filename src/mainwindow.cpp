// mainwindow.cpp
#include "mainwindow.h"
#include "editorwidget.h"
#include "filemanager.h"
#include "findbarwidget.h"
#include "documentoutlinewidget.h"
#include "sidebarfileexplorer.h"
#include "toastnotification.h"
#include "thememanager.h"
#include "themedialog.h"
#include "outlinedelegate.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QStatusBar>
#include <QLabel>
#include <QActionGroup>
#include <QKeyEvent>
#include <QTimer>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <cmark.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , tabWidget(nullptr)
    , fileManager(nullptr)
    , fileExplorer(nullptr)
    , outlineWidget(nullptr)
    , findBarWidget(nullptr)
    , toast(nullptr)
    , outlineDelegate(nullptr)
    , outlineTimer(nullptr)
    , wordCountTimer(nullptr)
{
    fileManager = new FileManager(this);

    resize(1200, 800);

    // Create tab widget FIRST (before anything that might use it)
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);

    // Create find bar
    findBarWidget = new FindBarWidget(this);
    findBarWidget->hide();

    // Create central widget with layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(tabWidget);
    centralLayout->addWidget(findBarWidget);
    setCentralWidget(centralWidget);

    // Create toast notification
    toast = new ToastNotification(this);

    createActions();
    createSidebar();
    createMenus();
    createStatusBar();
    loadSettings();

    // Initialize timers
    outlineTimer = new QTimer(this);
    outlineTimer->setSingleShot(true);
    outlineTimer->setInterval(1000);
    connect(outlineTimer, &QTimer::timeout, this, &MainWindow::updateOutline);

    wordCountTimer = new QTimer(this);
    wordCountTimer->setSingleShot(true);
    wordCountTimer->setInterval(1000);
    connect(wordCountTimer, &QTimer::timeout, this, &MainWindow::updateWordCount);

    // Connect tab signals
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    // Connect find bar signals
    connect(findBarWidget, &FindBarWidget::findBarHidden, [this]() {
        if (tabWidget->currentIndex() >= 0 && tabWidget->currentIndex() < editorTabs.size()) {
            editorTabs[tabWidget->currentIndex()].editor->setFocus();
        }
    });

    // Don't create initial tab here - only create if no files are opened
    // This allows command-line file opening to work without an extra empty tab

    statusBar()->showMessage(tr("Ready"));
    showMaximized();
}

MainWindow::~MainWindow()
{
    for (const EditorTab &tab : editorTabs) {
        if (tab.editor) {
            disconnect(tab.editor->document(), nullptr, this, nullptr);
            disconnect(tab.editor, nullptr, this, nullptr);
        }
    }
    editorTabs.clear();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    for (int i = editorTabs.size() - 1; i >= 0; --i) {
        const EditorTab &tab = editorTabs[i];
        // Only warn if document is modified AND has content (not empty)
        bool hasContent = tab.editor && !tab.editor->toPlainText().trimmed().isEmpty();
        if (tab.editor && tab.editor->document()->isModified() && hasContent) {
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
    EditorWidget *newEditor = new EditorWidget(tabWidget);
    setupEditorConnections(newEditor);

    EditorTab tab;
    tab.editor = newEditor;
    tab.filePath = QString();
    tab.isModified = false;
    editorTabs.append(tab);

    int tabIndex = tabWidget->addTab(newEditor, tr("Untitled"));
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
    for (int i = 0; i < editorTabs.size(); ++i) {
        if (editorTabs[i].filePath == fileName) {
            tabWidget->setCurrentIndex(i);
            // Update sidebar even if file is already open
            if (fileExplorer) {
                QFileInfo fi(fileName);
                fileExplorer->setRootPath(fi.absolutePath());
                if (sidebarDock && !sidebarDock->isVisible()) {
                    sidebarDock->show();
                }
            }
            return;
        }
    }

    EditorWidget *newEditor = new EditorWidget(tabWidget);

    if (!fileManager->loadFile(fileName, newEditor)) {
        delete newEditor;
        return;
    }

    setupEditorConnections(newEditor);

    EditorTab tab;
    tab.editor = newEditor;
    tab.filePath = fileName;
    tab.isModified = false;
    editorTabs.append(tab);

    int tabIndex = tabWidget->addTab(newEditor, QFileInfo(fileName).fileName());
    tabWidget->setCurrentIndex(tabIndex);

    updateWindowTitle();

    // Update sidebar to show the file's directory and ensure it's visible
    if (fileExplorer) {
        QFileInfo fi(fileName);
        fileExplorer->setRootPath(fi.absolutePath());

        // Make sure sidebar is visible when opening a file
        if (sidebarDock && !sidebarDock->isVisible()) {
            sidebarDock->show();
        }
    }
}

void MainWindow::setupEditorConnections(EditorWidget *editor)
{
    connect(editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::documentWasModified);

    connect(editor, &QPlainTextEdit::textChanged, [this]() {
        outlineTimer->start();
        wordCountTimer->start();
    });

    // Connect find bar to editor
    connect(findBarWidget, &FindBarWidget::findNextRequested, [this, editor]() {
        findBarWidget->setEditor(editor);
    });
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
        // Theme is already applied via ThemeManager::setTheme during preview
        // Just need to save the theme to settings (done automatically in setTheme)
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

void MainWindow::find()
{
    if (!isFindBarVisible()) {
        findBarWidget->showFindBar();
    }
}

bool MainWindow::isFindBarVisible() const
{
    return findBarWidget && findBarWidget->isFindBarVisible();
}

void MainWindow::onTabChanged(int index)
{
    updateActionsState();
    if (index >= 0 && index < editorTabs.size()) {
        updateWindowTitle();
        updateWordCount();
        updateOutline();
        
        // Update find bar editor reference
        findBarWidget->setEditor(editorTabs[index].editor);
    }
}

void MainWindow::closeTab(int index)
{
    if (index < 0 || index >= editorTabs.size()) return;

    EditorTab tab = editorTabs[index];

    // Only warn if document is modified AND has content (not empty)
    bool hasContent = tab.editor && !tab.editor->toPlainText().trimmed().isEmpty();
    if (tab.editor && tab.editor->document()->isModified() && hasContent) {
        tabWidget->setCurrentIndex(index);
        QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Scriber"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch (ret) {
        case QMessageBox::Save:
            if (!save()) return;
            break;
        case QMessageBox::Cancel:
            return;
        default:
            break;
        }
    }

    tabWidget->blockSignals(true);
    QWidget *widget = tabWidget->widget(index);
    tabWidget->removeTab(index);
    editorTabs.removeAt(index);
    tabWidget->blockSignals(false);

    delete widget;

    updateActionsState();

    if (tabWidget->count() == 0) {
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

    // Only warn if document is modified AND has content (not empty)
    bool hasContent = tab.editor && !tab.editor->toPlainText().trimmed().isEmpty();
    if (!tab.editor || !tab.editor->document()->isModified() || !hasContent)
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

    // Only warn if document is modified AND has content (not empty)
    bool hasContent = tab.editor && !tab.editor->toPlainText().trimmed().isEmpty();
    if (!tab.editor || !tab.editor->document()->isModified() || !hasContent)
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

    if (fileExplorer && !fileName.isEmpty()) {
        QFileInfo fi(fileName);
        fileExplorer->setRootPath(fi.absolutePath());
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

    // File Explorer tab
    fileExplorer = new SidebarFileExplorer(sidebarTabs);
    connect(fileExplorer, &SidebarFileExplorer::fileActivated, this, &MainWindow::openFileInNewTab);
    connect(fileExplorer, &SidebarFileExplorer::directoryChanged, [this](const QString &path) {
        // Handle directory change if needed
    });
    sidebarTabs->addTab(fileExplorer, tr("Files"));

    // Outline tab
    outlineWidget = new DocumentOutlineWidget(sidebarTabs);
    
    // Set up custom delegate for theme-aware arrow icons
    outlineDelegate = new OutlineDelegate(outlineWidget);
    outlineWidget->setItemDelegate(outlineDelegate);
    
    // Connect to theme changes
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        outlineDelegate->setArrowColor(ThemeManager::instance()->textColor());
        outlineWidget->viewport()->update();
    });
    outlineDelegate->setArrowColor(ThemeManager::instance()->textColor());
    
    sidebarTabs->addTab(outlineWidget, tr("Outline"));

    sidebarDock->setWidget(sidebarTabs);
    addDockWidget(Qt::LeftDockWidgetArea, sidebarDock);
    sidebarDock->show(); // Show sidebar by default

    toggleSidebarAct = sidebarDock->toggleViewAction();
    toggleSidebarAct->setText(tr("&Sidebar"));
    toggleSidebarAct->setStatusTip(tr("Show or hide the sidebar"));
}

void MainWindow::updateOutline()
{
    if (!outlineWidget) return;

    int currentIndex = tabWidget->currentIndex();
    if (currentIndex < 0 || currentIndex >= editorTabs.size()) return;

    EditorWidget *currentEditor = editorTabs[currentIndex].editor;
    if (!currentEditor) return;

    outlineWidget->setEditor(currentEditor);
    outlineWidget->updateOutline();
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
    if (event->key() == Qt::Key_Escape && isFindBarVisible()) {
        findBarWidget->hideFindBar();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}
