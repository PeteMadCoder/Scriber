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
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeyEvent>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      editor(nullptr), fileManager(nullptr),
      findBarWidget(nullptr), findLineEdit(nullptr), findStatusLabel(nullptr),
      caseSensitiveCheckBox(nullptr), wholeWordsCheckBox(nullptr),
      findNextButton(nullptr), findPreviousButton(nullptr), closeFindBarButton(nullptr),

      newAct(this), openAct(this), saveAct(this), saveAsAct(this),
      exportHtmlAct(this), exportPdfAct(this), exitAct(this),
      toggleThemeAct(this), aboutAct(this), findAct(this)
{
    editor = new EditorWidget(this);
    fileManager = new FileManager(this);
    setCentralWidget(editor);

    resize(1200, 800);

    createActions();
    createMenus();
    createFindBar();
    createStatusBar();
    loadSettings();

    setCurrentFile(QString());
    statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
    // QPointer handles cleanup automatically
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        editor->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        // Use QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) for default dir?
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Markdown File"), "",
                                                        tr("Markdown Files (*.md);;Text Files (*.txt);;All Files (*)"));
        if (!fileName.isEmpty())
            fileManager->loadFile(fileName, editor);
    }
}

void MainWindow::save()
{
    if (currentFile.isEmpty()) {
        saveAs();
    } else {
        fileManager->saveFile(currentFile, editor);
    }
}

void MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save As"), "",
                                                    tr("Markdown Files (*.md);;Text Files (*.txt);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    // Ensure .md extension if none provided and user selected Markdown filter
    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().isEmpty()) {
         fileName += ".md";
    }

    fileManager->saveFile(fileName, editor);
    setCurrentFile(fileName);
}

void MainWindow::exportToHtml() {
    // Implementation will go here, likely using fileManager
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to HTML"), QString(), tr("HTML Files (*.html)"));
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".html", Qt::CaseInsensitive))
            fileName += ".html";
        fileManager->exportToHtml(fileName, editor);
     }
}

void MainWindow::exportToPdf() {
    // Implementation will go here, likely using fileManager
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PDF"), QString(), tr("PDF Files (*.pdf)"));
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
            fileName += ".pdf";
       fileManager->exportToPdf(fileName, editor);
    }
}

void MainWindow::toggleTheme() {
    // Implementation will go here, likely calling a method on editor or a ThemeManager
    editor->toggleTheme();
}

void MainWindow::about()
{
     QMessageBox::about(this, tr("About Scriber"),
                        tr("<b>Scriber</b><br/>"
                           "A distraction-free Markdown editor.<br/>"
                           "Version %1")
                        .arg(QApplication::applicationVersion()));
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
    findAct.setShortcuts(QKeySequence::Find); // Ctrl+F
    findAct.setStatusTip(tr("Find text in the document"));
    connect(&findAct, &QAction::triggered, this, &MainWindow::find);
    addAction(&findAct);
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

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(&toggleThemeAct);

    // --- Add Spell Check Menu ---
    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));

    QAction *spellCheckToggleAction = new QAction(tr("&Spell Check"), this);
    spellCheckToggleAction->setCheckable(true);
    spellCheckToggleAction->setChecked(editor->isSpellCheckEnabled());
    connect(spellCheckToggleAction, &QAction::toggled, [this](bool checked) {
        editor->setSpellCheckEnabled(checked);
    });
    toolsMenu->addAction(spellCheckToggleAction);

    // --- Add Language Selection Submenu ---
    QMenu *languageMenu = toolsMenu->addMenu(tr("&Language"));

    // Define available languages (add more as needed)
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
        // Add more languages supported by your system's hunspell dictionaries
    };

    QActionGroup *languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true); // Only one language can be selected

    for (const LanguageInfo &langInfo : languages) {
        QAction *langAction = new QAction(langInfo.name, this);
        langAction->setCheckable(true);
        langAction->setData(langInfo.code); // Store the language code
        languageGroup->addAction(langAction);
        languageMenu->addAction(langAction);

        // Set the default language as checked
        if (langInfo.code == "en_US") { // Or whatever your default is
             langAction->setChecked(true);
        }

        connect(langAction, &QAction::triggered, [this, langInfo]() {
            editor->setSpellCheckLanguage(langInfo.code);
        });
    }

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(&findAct);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(&aboutAct);
}


void MainWindow::loadSettings()
{
    QSettings settings;
    // Load window geometry, last file, theme preference, etc.
    // Example:
    // restoreGeometry(settings.value("geometry").toByteArray());
    // QString lastTheme = settings.value("theme", "light").toString();
    // if (lastTheme == "dark") editor->setTheme(false); // Assuming setTheme(false) sets dark
}

void MainWindow::saveSettings()
{
    QSettings settings;
    // Save window geometry, current file, theme preference, etc.
    // Example:
    // settings.setValue("geometry", saveGeometry());
    // settings.setValue("theme", editor->isDarkTheme() ? "dark" : "light"); // Need isDarkTheme() method
}

bool MainWindow::maybeSave()
{
    // Check if the document in editor is modified
    if (editor->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Scriber"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return fileManager->saveFile(currentFile, editor); // This needs to handle Save As if currentFile is empty
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    editor->document()->setModified(false);
    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.md";
    setWindowFilePath(shownName); // Sets the window title with the file path
    setWindowTitle(QString("%1[*] - %2").arg(shownName, QApplication::applicationName())); // [*] shows modified state
}

void MainWindow::createStatusBar() {
    wordCountLabel = new QLabel(tr("Words: 0"));
    charCountLabel = new QLabel(tr("Chars: 0"));
    
    statusBar()->addPermanentWidget(wordCountLabel);
    statusBar()->addPermanentWidget(charCountLabel);
    
    // Connect to editor's textChanged signal
    connect(editor, &QPlainTextEdit::textChanged, 
            this, &MainWindow::updateWordCount);
}

void MainWindow::updateWordCount() {
    QString text = editor->toPlainText();
    int wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    int charCount = text.length();
    
    wordCountLabel->setText(tr("Words: %1").arg(wordCount));
    charCountLabel->setText(tr("Chars: %1").arg(charCount));
}

void MainWindow::openFile(const QString &path)
{
    QString fileName = path;
    
    // If no path was provided, show file dialog
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), 
                                              QString(), 
                                              tr("Markdown Files (*.md *.markdown);;All Files (*)"));
    }
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            editor->setPlainText(in.readAll());
            setCurrentFile(fileName);
            statusBar()->showMessage(tr("File loaded: %1").arg(fileName), 2000);
        } else {
            QMessageBox::warning(this, tr("Scriber"),
                                 tr("Cannot read file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        }
    }
}

void MainWindow::createFindBar()
{
    // Create the container widget for the find bar
    findBarWidget = new QWidget(this);
    findBarWidget->setObjectName("findBar"); // Optional: for styling via CSS
    findBarWidget->setVisible(false); // Start hidden
    isFindBarVisible = false;

    // Create the UI elements
    QLabel *findLabel = new QLabel(tr("Find:"), findBarWidget);
    findLineEdit = new QLineEdit(findBarWidget);
    findStatusLabel = new QLabel(findBarWidget); // For status messages
    findStatusLabel->setMinimumWidth(150); // Give it some space
    findStatusLabel->setStyleSheet("QLabel { color : gray; }"); // Style the status label
    findStatusLabel->setText(""); // Initially empty

    caseSensitiveCheckBox = new QCheckBox(tr("Case sensitive"), findBarWidget);
    wholeWordsCheckBox = new QCheckBox(tr("Whole words"), findBarWidget);

    findNextButton = new QPushButton(tr("Next"), findBarWidget);
    findPreviousButton = new QPushButton(tr("Previous"), findBarWidget);
    closeFindBarButton = new QPushButton(tr("Close"), findBarWidget);
    // Make the close button look like a small 'x'
    closeFindBarButton->setFixedSize(24, 24);
    closeFindBarButton->setText("X");
    closeFindBarButton->setToolTip(tr("Close find bar (Escape)"));

    // --- Layout ---
    QHBoxLayout *optionsLayout = new QHBoxLayout;
    optionsLayout->addWidget(caseSensitiveCheckBox);
    optionsLayout->addWidget(wholeWordsCheckBox);
    optionsLayout->addStretch(); // Push buttons to the right

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(findStatusLabel);
    buttonsLayout->addStretch(); // Push status label to the left
    buttonsLayout->addWidget(findNextButton);
    buttonsLayout->addWidget(findPreviousButton);
    buttonsLayout->addWidget(closeFindBarButton);

    QVBoxLayout *findBarLayout = new QVBoxLayout(findBarWidget);
    findBarLayout->setContentsMargins(5, 5, 5, 5); // Reduce margins
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(findLabel);
    topLayout->addWidget(findLineEdit);
    topLayout->addLayout(optionsLayout);
    findBarLayout->addLayout(topLayout);
    findBarLayout->addLayout(buttonsLayout);

    // --- Add to Main Window Layout ---
    // QMainWindow typically uses a central widget. To add a bar at the bottom,
    // we need to manage the layout of the central widget or use QMainWindow's
    // addToolBarBreak/addToolBar(Qt::BottomToolBarArea, ...) conceptually.
    // A simpler and common approach is to put the central widget and the find bar
    // inside a new container widget with a QVBoxLayout.

    // 1. Create a new central container
    QWidget *centralContainer = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0); // No margins
    mainLayout->setSpacing(0); // No spacing

    // 2. Reparent the existing editor to this container (it's already created)
    //    and add the find bar widget
    mainLayout->addWidget(editor);       // Add editor first (takes most space)
    mainLayout->addWidget(findBarWidget); // Add find bar at the bottom

    // 3. Set this new container as the central widget
    setCentralWidget(centralContainer);

    // --- Connect Signals and Slots ---
    connect(findLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onFindNext);
    connect(findLineEdit, &QLineEdit::textEdited, this, &MainWindow::onFindTextEdited); // Optional live search
    connect(findNextButton, &QPushButton::clicked, this, &MainWindow::onFindNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &MainWindow::onFindPrevious);
    connect(closeFindBarButton, &QPushButton::clicked, this, &MainWindow::hideFindBar);
    // Optional: Connect checkbox changes to re-trigger search if text is present
    // connect(caseSensitiveCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onFindOptionsChanged);
    // connect(wholeWordsCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onFindOptionsChanged);
}


void MainWindow::find()
{
    // Toggle visibility of the find bar
    isFindBarVisible = !isFindBarVisible;
    findBarWidget->setVisible(isFindBarVisible);

    if (isFindBarVisible) {
        // If shown, populate with selected text and give it focus
        QString selectedText = editor->textCursor().selectedText();
        if (!selectedText.isEmpty() && selectedText.size() < 100) {
            findLineEdit->setText(selectedText);
        }
        findLineEdit->selectAll();
        findLineEdit->setFocus();
        findStatusLabel->setText(""); // Clear any previous status
    } else {
        // If hidden, optionally clear the highlight or selection
        // This depends on how you want the "find" highlight to behave.
        // For now, we'll just hide the bar.
    }
}

void MainWindow::hideFindBar()
{
     isFindBarVisible = false;
     findBarWidget->setVisible(false);
     // Optional: Clear search term or status
     // findLineEdit->clear();
     // findStatusLabel->setText("");
     editor->setFocus(); // Return focus to the editor
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Handle Escape key to close the find bar if it's visible
    if (event->key() == Qt::Key_Escape && isFindBarVisible) {
        hideFindBar();
        event->accept(); // Mark as handled
        return;
    }
    // Pass other key events to the base class
    QMainWindow::keyPressEvent(event);
}

// --- Find Logic Slots ---

void MainWindow::onFindTextEdited()
{
     // Optional: Perform live search as the user types
     // This can be useful but might be resource intensive for large documents.
     // You can debounce it using a QTimer if needed.
     /*
     QString text = findLineEdit->text();
     if (!text.isEmpty()) {
         // Perform a find next implicitly
         onFindNext();
     } else {
         // Clear any current highlights if text is cleared
         findStatusLabel->setText("");
     }
     */
     // For now, we'll leave this empty or just clear the status.
     findStatusLabel->setText("");
}

void MainWindow::onFindNext()
{
    QString text = findLineEdit->text();
    if (text.isEmpty()) {
        findStatusLabel->setText(tr("Enter text to find"));
        return;
    }

    QTextDocument::FindFlags flags;
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (wholeWordsCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }

    bool found = editor->find(text, flags);

    if (!found) {
        // Wrap around to the beginning
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        editor->setTextCursor(cursor);

        found = editor->find(text, flags);
        if (!found) {
            findStatusLabel->setText(tr("'%1' not found").arg(text));
            // Optional: Change label color for error
            findStatusLabel->setStyleSheet("QLabel { color : red; }");
        } else {
             findStatusLabel->setText(""); // Clear status on successful wrap
             findStatusLabel->setStyleSheet("QLabel { color : gray; }");
        }
    } else {
        // Found, clear status or show position info
        findStatusLabel->setText(""); // Clear status on successful find
        findStatusLabel->setStyleSheet("QLabel { color : gray; }");
    }
    // If found, the editor's cursor is automatically moved and the text is selected.
}

void MainWindow::onFindPrevious()
{
    QString text = findLineEdit->text();
    if (text.isEmpty()) {
        findStatusLabel->setText(tr("Enter text to find"));
        return;
    }

    QTextDocument::FindFlags flags;
    flags |= QTextDocument::FindBackward; // Add backward flag
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (wholeWordsCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }

    bool found = editor->find(text, flags);

    if (!found) {
        // Wrap around to the end
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::End);
        editor->setTextCursor(cursor);

        found = editor->find(text, flags);
        if (!found) {
            findStatusLabel->setText(tr("'%1' not found").arg(text));
            findStatusLabel->setStyleSheet("QLabel { color : red; }");
        } else {
             findStatusLabel->setText(""); // Clear status on successful wrap
             findStatusLabel->setStyleSheet("QLabel { color : gray; }");
        }
    } else {
        // Found, clear status
        findStatusLabel->setText(""); // Clear status on successful find
        findStatusLabel->setStyleSheet("QLabel { color : gray; }");
    }
    // If found, the editor's cursor is automatically moved and the text is selected.
}