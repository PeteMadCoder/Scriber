#pragma once
#include <QPlainTextEdit>
#include <QScopedPointer>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QScopedPointer>
#include <QWheelEvent>
#include <QTextEdit>
#include <QPointer>
#include <QTimer>

class MarkdownHighlighter; // Forward declaration
class SpellChecker;

class EditorWidget : public QPlainTextEdit
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

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void checkSpelling();

private:
    QScopedPointer<MarkdownHighlighter> highlighter; // Manage the highlighter's lifetime
    Theme currentTheme; // Track current theme state
    int currentZoom;

    void applyTheme(); // Apply the current theme (palette, stylesheet)

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
