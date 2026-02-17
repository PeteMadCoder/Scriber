#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QColor>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    enum class Theme {
        Light,
        Dark,
        PitchBlack
    };
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr);

    void setTheme(Theme theme);
    void setFontSize(int baseSize);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
        int contentGroup = 2; // Default to group 2 for most patterns
        bool applyBlockFormat = false;
        bool isMultiLine = false; // For elements that span multiple lines
    };
    QVector<HighlightingRule> highlightingRules;
    QVector<HighlightingRule> pythonRules;
    QVector<HighlightingRule> cppRules;
    QVector<HighlightingRule> bashRules;

    // Formats for different Markdown elements
    QTextCharFormat heading1Format;
    QTextCharFormat heading2Format;
    QTextCharFormat heading3Format;
    QTextCharFormat heading4Format;
    QTextCharFormat heading5Format;
    QTextCharFormat heading6Format;
    QTextCharFormat boldFormat;
    QTextCharFormat italicFormat;
    QTextCharFormat strikethroughFormat;
    QTextCharFormat codeFormat; // Inline code
    QTextCharFormat linkFormat;
    QTextCharFormat imageFormat;
    QTextCharFormat listFormat;
    QTextCharFormat taskListFormat;
    QTextCharFormat blockquoteFormat;
    QTextCharFormat tableHeaderFormat;
    QTextCharFormat tableCellFormat;
    QTextCharFormat syntaxFaintFormat; // For syntax characters

    // Block formats
    QTextBlockFormat heading1BlockFormat;
    QTextBlockFormat heading2BlockFormat;
    QTextBlockFormat heading3BlockFormat;
    QTextBlockFormat heading4BlockFormat;
    QTextBlockFormat heading5BlockFormat;
    QTextBlockFormat heading6BlockFormat;
    QTextBlockFormat codeBlockBlockFormat;
    QTextBlockFormat blockquoteBlockFormat;
    QTextBlockFormat horizontalRuleBlockFormat;
    QTextBlockFormat tableBlockFormat;

    int currentBaseFontSize = 12;

    // --- Theme Colors ---
    struct ThemeColors {
        QColor background;
        QColor text;
        QColor heading;
        QColor bold;
        QColor italic;
        QColor strikethrough;
        QColor codeText;
        QColor codeBackground;
        QColor link;
        QColor image;
        QColor list;
        QColor taskList;
        QColor blockquoteText;
        QColor blockquoteBackground;
        QColor tableHeaderText;
        QColor tableCellText;
        QColor tableHeaderBackground;
        QColor tableCellBackground;
        QColor horizontalRule;
        QColor syntaxFaint;
    };

    ThemeColors lightColors;
    ThemeColors darkColors;
    ThemeColors pitchBlackColors;
    ThemeColors currentColors;

    Theme currentTheme;

    void setupThemeColors();
    void updateFormatsForTheme();
    void setupInitialRules();
    
    // State tracking for multi-line elements
    static const int STATE_NORMAL = 0;
    static const int STATE_IN_CODE_BLOCK = 1;
    static const int STATE_IN_TABLE = 2;
    static const int STATE_IN_CODE_BLOCK_PYTHON = 10;
    static const int STATE_IN_CODE_BLOCK_CPP = 11;
    static const int STATE_IN_CODE_BLOCK_BASH = 12;
    
    // Additional formats for code
    QTextCharFormat keywordFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat functionFormat;
    
    void setupPythonRules();
    void setupCppRules();
    void setupBashRules();
    void highlightCodeBlock(const QString &text, int languageState);
};