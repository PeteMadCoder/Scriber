#pragma once
#include <QTextEdit>
#include <QTextBlockUserData>
#include <QScopedPointer>

class MarkdownBlockData : public QTextBlockUserData {
public:
    QString rawMarkdown;
    bool isRendered = false;
};

class MarkdownHighlighter; // Forward declaration
#include <QKeyEvent>
#include <QFocusEvent>
#include <QScopedPointer>
#include <QWheelEvent>
#include <QTextEdit>
#include <QPointer>
#include <QTimer>

class MarkdownHighlighter; // Forward declaration
class SpellChecker;

class EditorWidget : public QTextEdit
{
    Q_OBJECT

public:
    enum class Theme {
        Light,
        Dark,
        PitchBlack
    };

    explicit EditorWidget(QWidget *parent = nullptr);
    ~EditorWidget();

    void toggleTheme(); // Cycle through themes
    void zoomIn(int steps = 1);
    void zoomOut(int steps = 1);
    int getCurrentZoom() const { return currentZoom; }
    void setSpellCheckerEnabled(bool enabled);

    // Spell checker methods
    void setSpellCheckEnabled(bool enabled);
    void setSpellCheckLanguage(const QString &language);
    bool isSpellCheckEnabled() const;
    QString getRawMarkdown() const;
    void renderAllBlocks();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void checkSpelling();
    void onCursorPositionChanged();

private:
    QScopedPointer<MarkdownHighlighter> highlighter; // Manage the highlighter's lifetime
    Theme currentTheme; // Track current theme state
    int currentZoom;
    int activeBlockNumber = -1; // Track the block currently being edited

    void applyTheme(); // Apply the current theme (palette, stylesheet)

    void renderBlock(QTextBlock block);
    int revealBlock(QTextBlock block); // Returns number of blocks merged
    QString renderMarkdownToHtml(const QString& markdown);

    void insertMarkdownPair(const QString &opening, const QString &closing);
    bool handleBackspace();
    bool handleEnter();
    void autoIndent();

    void updateFont();

    // Spell checker components
    QScopedPointer<SpellChecker> spellChecker; // Use QScopedPointer for automatic cleanup
    bool spellCheckEnabled = true;
    QTimer *spellCheckTimer; // Timer for delayed checking
    void highlightMisspelledWords();
    QTextCursor findWordUnderCursor();
};
