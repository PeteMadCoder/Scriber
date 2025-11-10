#include "editorwidget.h"
#include "markdownhighlighter.h"
#include "spellchecker.h"
#include <QFont>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QShortcut> 
#include <QMenu> 
#include <QAction> 
#include <QScrollBar>


EditorWidget::EditorWidget(QWidget *parent)
    : QPlainTextEdit(parent), darkTheme(true), currentZoom(0)
{
    QFont font;
    font.setFamily("Segoe UI, Arial, sans-serif");
    font.setPointSize(12);
    setFont(font);
    
    // Set line spacing for better readability
    QTextBlockFormat blockFormat;
    blockFormat.setLineHeight(125, QTextBlockFormat::FixedHeight);
    textCursor().setBlockFormat(blockFormat);

    // Ensure no background color interference
    viewport()->setAutoFillBackground(false);
    
    MarkdownHighlighter* mdHighlighter = new MarkdownHighlighter(document());
    highlighter.reset(mdHighlighter);

    applyTheme();
    
    // Add keyboard shortcuts
    QShortcut *boldShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this);  // USE | INSTEAD OF +
    connect(boldShortcut, &QShortcut::activated, [this]() {
        insertMarkdownPair("**", "**");
    });

    QShortcut *italicShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), this);  // USE | INSTEAD OF +
    connect(italicShortcut, &QShortcut::activated, [this]() {
        insertMarkdownPair("*", "*");
    });

    QShortcut *codeShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_K), this);  // USE | INSTEAD OF +
    connect(codeShortcut, &QShortcut::activated, [this]() {
        insertMarkdownPair("`", "`");
    });

    QShortcut *linkShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);  // USE | INSTEAD OF +
    connect(linkShortcut, &QShortcut::activated, [this]() {
        insertMarkdownPair("[", "](url)");
    });

    // --- Initialize Spell Checker ---
    spellChecker.reset(new SpellChecker(this)); // Parent it to this widget for automatic cleanup

    // Attempt to load a default dictionary
    if (!spellChecker->loadDictionary("en_US")) {
        qWarning() << "EditorWidget: Failed to load default 'en_US' dictionary. Spell checking might not work.";
        // You could try other fallback languages here if needed
    } else {
        qDebug() << "EditorWidget: Successfully loaded spell checker with 'en_US' dictionary.";
    }

    // Create a timer for delayed spell checking to avoid checking on every keystroke
    spellCheckTimer = new QTimer(this);
    spellCheckTimer->setSingleShot(true); // Only fire once per start
    spellCheckTimer->setInterval(500);   // 500ms delay
    connect(spellCheckTimer, &QTimer::timeout, this, &EditorWidget::checkSpelling);

    // Connect to text changes to trigger delayed spell checking
    connect(this, &QPlainTextEdit::textChanged, [this]() {
        if (spellCheckEnabled && spellChecker && spellChecker->isInitialized()) {
            spellCheckTimer->start(); // Restarts the timer if already running
        }
    });

    // Initial spell check if enabled
    if (spellCheckEnabled) {
        // Use a single-shot timer with 0 delay to defer the initial check
        // until after the constructor finishes and the event loop starts.
        QTimer::singleShot(0, this, &EditorWidget::checkSpelling);
    }
}

void EditorWidget::toggleTheme()
{
    darkTheme = !darkTheme;
    applyTheme();
    if (highlighter) {
        highlighter->setTheme(darkTheme ? MarkdownHighlighter::Theme::Dark : MarkdownHighlighter::Theme::Light);
    }
}

void EditorWidget::applyTheme()
{
    QString themeFile = darkTheme ? ":/resources/themes/dark.css" : ":/resources/themes/light.css";
    QFile file(themeFile);
    QString styleSheet;
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        styleSheet = in.readAll();
        file.close();
        setStyleSheet(styleSheet);
        qDebug() << "Applied theme:" << (darkTheme ? "Dark" : "Light");
    } else {
        qDebug() << "Could not open theme file:" << themeFile;
        // Fallback to explicit palette
        QPalette p = qApp->palette();
        if (darkTheme) {
            // DARK THEME FALLBACK - IMPROVED
            p.setColor(QPalette::Base, QColor(30, 30, 30));
            p.setColor(QPalette::Text, QColor(225, 228, 232));  // Very light gray
            p.setColor(QPalette::Window, QColor(50, 50, 50));
            p.setColor(QPalette::WindowText, QColor(240, 240, 240));
        } else {
            // Light theme fallback
            p.setColor(QPalette::Base, Qt::white);
            p.setColor(QPalette::Text, Qt::black);
            p.setColor(QPalette::Window, Qt::white);
            p.setColor(QPalette::WindowText, Qt::black);
            p.setColor(QPalette::Highlight, QColor(0, 120, 215));
            p.setColor(QPalette::HighlightedText, Qt::white);
        }
        setPalette(p);
        setAutoFillBackground(true);
        qDebug() << "Using palette fallback for" << (darkTheme ? "dark" : "light") << "theme";
    }
    
    // Force re-highlighting to ensure new colors are applied
    if (highlighter) {
        highlighter->rehighlight();
    }
}

// Correct implementation of key event handling
void EditorWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Backspace) {
        if (handleBackspace()) {
            return; // Event handled, don't call base class
        }
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (handleEnter()) {
            return; // Event handled, don't call base class
        }
    } else if (event->text() == "*" || event->text() == "_" || event->text() == "`") {
        insertMarkdownPair(event->text(), event->text());
        return; // Event handled, don't call base class
    }

    if (event->modifiers() == Qt::ControlModifier && 
        (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal)) {
        zoomIn();
        return;
    }
    
    // Handle Ctrl+'-' for zoom out
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Minus) {
        zoomOut();
        return;
    }
    
    // Handle Ctrl+0 to reset zoom
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_0) {
        currentZoom = 0;
        updateFont();
        return;
    }
    
    // If we get here, the event wasn't handled by our custom logic
    QPlainTextEdit::keyPressEvent(event);
}

bool EditorWidget::handleBackspace() {
    QTextCursor cursor = textCursor();
    if (cursor.positionInBlock() > 0) {
        QString text = cursor.block().text();
        int pos = cursor.positionInBlock();
        
        // Unindent on backspace at beginning of indented line
        if (pos == 1 && text.startsWith(' ')) {
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            return true; // Event handled
        }
        
        // Remove paired markdown syntax
        if (pos > 1 && text.mid(pos-1, 2) == "**") {
            cursor.movePosition(QTextCursor::Left);
            cursor.deleteChar();
            cursor.deleteChar();
            return true; // Event handled
        }
        
        // Handle other markdown pairs
        if (pos > 1 && (text.mid(pos-1, 2) == "__" || text.mid(pos-1, 2) == "``")) {
            cursor.movePosition(QTextCursor::Left);
            cursor.deleteChar();
            cursor.deleteChar();
            return true; // Event handled
        }
    }
    
    return false; // Event not handled
}

bool EditorWidget::handleEnter() {
    QTextCursor cursor = textCursor();
    QString currentLine = cursor.block().text().trimmed();
    
    // Continue list formatting
    if (currentLine.startsWith("- ") || currentLine.startsWith("* ") || 
        QRegularExpression("^\\d+\\. ").match(currentLine).hasMatch()) {
        cursor.insertBlock();
        
        // Preserve list type
        if (currentLine.startsWith("- ")) {
            cursor.insertText("- ");
        } else if (currentLine.startsWith("* ")) {
            cursor.insertText("* ");
        } else {
            int num = QRegularExpression("^\\d+").match(currentLine).captured().toInt() + 1;
            cursor.insertText(QString::number(num) + ". ");
        }
        return true; // Event handled
    }
    
    // Auto-indent blockquotes
    if (currentLine.startsWith("> ")) {
        cursor.insertBlock();
        cursor.insertText("> ");
        return true; // Event handled
    }
    
    // Auto-indent code blocks
    if (currentLine.startsWith("```")) {
        cursor.insertBlock();
        cursor.insertText("```");
        return true; // Event handled
    }
    
    return false; // Event not handled
}

void EditorWidget::insertMarkdownPair(const QString &opening, const QString &closing) {
    QTextCursor cursor = textCursor();
    cursor.insertText(opening + closing);
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
}

void EditorWidget::focusInEvent(QFocusEvent *event) {
    // Call the base class implementation first
    QPlainTextEdit::focusInEvent(event);
}

void EditorWidget::contextMenuEvent(QContextMenuEvent *event) {
    QMenu *menu = createStandardContextMenu();

    // --- Spell Check Context Menu Integration ---
    if (spellCheckEnabled && spellChecker && spellChecker->isInitialized()) {
        QTextCursor cursor = cursorForPosition(event->pos());
        cursor.select(QTextCursor::WordUnderCursor);
        QString selectedWord = cursor.selectedText();

        if (!selectedWord.isEmpty() &&
            QRegularExpression(QStringLiteral("^\\w+$")).match(selectedWord).hasMatch() &&
            spellChecker->isWordMisspelled(selectedWord)) {

            QList<QAction*> spellActions;
            QStringList suggestions = spellChecker->getSuggestions(selectedWord);

            if (!suggestions.isEmpty()) {
                for (const QString &suggestion : suggestions) {
                    QAction *suggestionAction = new QAction(suggestion, menu);
                    connect(suggestionAction, &QAction::triggered, [this, suggestion, cursor]() {
                        QTextCursor editCursor = cursor;
                        editCursor.beginEditBlock();
                        editCursor.insertText(suggestion);
                        editCursor.endEditBlock();
                        QTimer::singleShot(0, this, &EditorWidget::checkSpelling);
                    });
                    spellActions.append(suggestionAction);
                }
            } else {
                QAction *noSuggestionsAction = new QAction(tr("No suggestions"), menu);
                noSuggestionsAction->setEnabled(false);
                spellActions.append(noSuggestionsAction);
            }

            QAction *addWordAction = new QAction(tr("Add to Dictionary"), menu);
            connect(addWordAction, &QAction::triggered, [this, selectedWord]() {
                spellChecker->addWord(selectedWord);
                QTimer::singleShot(0, this, &EditorWidget::checkSpelling);
            });
            spellActions.append(addWordAction);

            // --- Insert Actions and Separators ---
            // 1. Determine the insertion point: before the first original action
            QAction *originalFirstAction = nullptr;
            if (!menu->actions().isEmpty()) {
                originalFirstAction = menu->actions().first();
            }
            // If menu is empty, originalFirstAction remains nullptr, and actions/separators will be appended.

            // 2. Insert Bottom Separator (before original actions)
            QAction *bottomSeparator = new QAction(menu);
            bottomSeparator->setSeparator(true);
            menu->insertAction(originalFirstAction, bottomSeparator);

            // 3. Insert Spell Actions (above the bottom separator)
            // Iterate in reverse order to maintain correct order when inserting at the same point
            for (int i = spellActions.size() - 1; i >= 0; --i) {
                menu->insertAction(originalFirstAction, spellActions[i]);
            }

            // 4. Insert Top Separator (above all spell actions)
            QAction *topSeparator = new QAction(menu);
            topSeparator->setSeparator(true);
            menu->insertAction(originalFirstAction, topSeparator); // Now inserts above everything spell-related
        }
    } else {
        // Add Markdown formatting actions
        menu->addSeparator();
        
        QAction *boldAction = menu->addAction(tr("Bold (Ctrl+B)"));
        boldAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));  // USE | INSTEAD OF +
        connect(boldAction, &QAction::triggered, [this]() {
            insertMarkdownPair("**", "**");
        });
        
        QAction *italicAction = menu->addAction(tr("Italic (Ctrl+I)"));
        italicAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));  // USE | INSTEAD OF +
        connect(italicAction, &QAction::triggered, [this]() {
            insertMarkdownPair("*", "*");
        });
        
        QAction *codeAction = menu->addAction(tr("Inline Code (Ctrl+K)"));
        codeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_K));  // USE | INSTEAD OF +
        connect(codeAction, &QAction::triggered, [this]() {
            insertMarkdownPair("`", "`");
        });
        
        menu->addSeparator();
        
        QAction *linkAction = menu->addAction(tr("Insert Link (Ctrl+L)"));
        linkAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));  // USE | INSTEAD OF +
        connect(linkAction, &QAction::triggered, [this]() {
            insertMarkdownPair("[", "](url)");
        });
        
        QAction *imageAction = menu->addAction(tr("Insert Image"));
        connect(imageAction, &QAction::triggered, [this]() {
            insertMarkdownPair("![", "](image.jpg)");
        });
        
        menu->addSeparator();
        
        QAction *blockquoteAction = menu->addAction(tr("Blockquote (> )"));
        connect(blockquoteAction, &QAction::triggered, [this]() {
            QTextCursor cursor = textCursor();
            if (cursor.hasSelection()) {
                // Apply blockquote to each selected line
                QTextBlock block = document()->findBlock(cursor.selectionStart());
                QTextBlock endBlock = document()->findBlock(cursor.selectionEnd()).next();
                
                while (block != endBlock) {
                    cursor.setPosition(block.position());
                    cursor.insertText("> ");
                    block = block.next();
                }
                
                // Adjust selection to include the added "> "
                cursor.setPosition(document()->findBlock(cursor.selectionStart()).position());
                cursor.setPosition(document()->findBlock(cursor.selectionEnd()).position() + cursor.selectionEnd() - cursor.selectionStart() + 2, QTextCursor::KeepAnchor);
                setTextCursor(cursor);
            } else {
                insertMarkdownPair("> ", "");
            }
        });
        
        QAction *unorderedListAction = menu->addAction(tr("Unordered List (- )"));
        connect(unorderedListAction, &QAction::triggered, [this]() {
            insertMarkdownPair("- ", "");
        });
        
        QAction *orderedListAction = menu->addAction(tr("Ordered List (1. )"));
        connect(orderedListAction, &QAction::triggered, [this]() {
            insertMarkdownPair("1. ", "");
        });
    }
        
    menu->exec(event->globalPos());
    delete menu;
}

void EditorWidget::updateFont() {
    int newSize = 12 + currentZoom;
    newSize = qBound(8, newSize, 48); // Limit reasonable zoom range
    
    // Update the widget's font for UI elements
    QFont widgetFont = font();
    widgetFont.setPointSize(newSize);
    setFont(widgetFont);

    QFont docFont = document()->defaultFont();
    docFont.setPointSize(newSize);
    document()->setDefaultFont(docFont);
    
    // Now update the highlighter with the new base size
    if (highlighter) {
        highlighter->setFontSize(newSize);
    }
    
    // Update line spacing
    QTextBlockFormat blockFormat;
    blockFormat.setLineHeight(100 + currentZoom * 2, QTextBlockFormat::FixedHeight);
    textCursor().setBlockFormat(blockFormat);
    
    // CRITICAL: Force a complete refresh of the text layout
    document()->markContentsDirty(0, document()->characterCount());
    
    // Force a full repaint
    viewport()->update();
}

void EditorWidget::zoomIn(int steps) {
    currentZoom += steps;
    updateFont();
}

void EditorWidget::zoomOut(int steps) {
    currentZoom -= steps;
    updateFont();
}

void EditorWidget::wheelEvent(QWheelEvent *e) {
    // Handle Ctrl+Wheel for zoom - platform aware
    if ((e->modifiers() & Qt::ControlModifier) || 
        (e->modifiers() & Qt::MetaModifier)) {  // Meta is Command on macOS

        // Calculate zoom steps based on platform-specific wheel delta
        int delta = e->angleDelta().y();
        int steps = qAbs(delta) / 120;  // Standard wheel step is 120
        if (steps == 0) steps = 1;  // Ensure at least one step
        
        if (delta > 0) {
            zoomIn(steps);
        } else {
            zoomOut(steps);
        }
        
        e->accept();  // Mark as handled
        return;
    }
    
    // Pass other wheel events to base class
    QPlainTextEdit::wheelEvent(e);
}

void EditorWidget::setSpellCheckEnabled(bool enabled)
{
    if (spellCheckEnabled == enabled) {
        return; // No change
    }

    spellCheckEnabled = enabled;

    if (enabled) {
        // If enabling, perform an immediate check
        checkSpelling();
    } else {
        // If disabling, clear existing highlights
        QTextCursor cursor(document());
        cursor.select(QTextCursor::Document);
        QTextCharFormat format;
        format.setUnderlineStyle(QTextCharFormat::NoUnderline);
        cursor.mergeCharFormat(format);
    }
}

void EditorWidget::setSpellCheckLanguage(const QString &language)
{
    if (spellChecker && spellChecker->loadDictionary(language)) {
        qDebug() << "EditorWidget: Switched spell check language to" << language;
        if (spellCheckEnabled) {
            checkSpelling(); // Re-check with the new language
        }
    } else {
        qWarning() << "EditorWidget: Failed to load dictionary for language" << language;
    }
}

bool EditorWidget::isSpellCheckEnabled() const
{
    return spellCheckEnabled;
}

void EditorWidget::checkSpelling()
{
    if (!spellCheckEnabled || !spellChecker || !spellChecker->isInitialized()) {
        return;
    }

    highlightMisspelledWords();
}

void EditorWidget::highlightMisspelledWords()
{
    // --- Clear Previous Highlights ---
    QTextCursor clearCursor(document());
    clearCursor.select(QTextCursor::Document);
    QTextCharFormat clearFormat;
    clearFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
    clearCursor.mergeCharFormat(clearFormat);

    // --- Define Highlight Format for Misspelled Words ---
    QTextCharFormat misspelledFormat;
    misspelledFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    misspelledFormat.setUnderlineColor(QColor(Qt::red)); // Standard red wavy underline

    // --- Find and Highlight Misspelled Words ---
    // Use a regular expression to find word boundaries.
    // \b ensures we match whole words.
    // \w+ matches one or more word characters (letters, digits, underscore).
    // This is a simple approach; a more sophisticated one might ignore
    // Markdown syntax or numbers.
    QRegularExpression wordRegex(QStringLiteral("\\b(\\w+)\\b"));
    QString documentText = document()->toPlainText();

    QRegularExpressionMatchIterator matchIterator = wordRegex.globalMatch(documentText);

    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString word = match.captured(1); // Captured group 1 is the word itself

        // Check if the word is misspelled using our Hunspell wrapper
        if (spellChecker->isWordMisspelled(word)) {
            // qDebug() << "Found misspelled word:" << word; // Uncomment for debugging

            // Create a cursor to select and format this specific word
            QTextCursor wordCursor(document());
            int startPos = match.capturedStart(1);
            int length = match.capturedLength(1);

            wordCursor.setPosition(startPos);
            wordCursor.setPosition(startPos + length, QTextCursor::KeepAnchor);

            // Apply the misspelled format
            wordCursor.mergeCharFormat(misspelledFormat);
        }
    }
    // qDebug() << "Spell checking completed."; // Uncomment for debugging
}
