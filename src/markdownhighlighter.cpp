#include "markdownhighlighter.h"
#include "thememanager.h"
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QTextBlock>
#include <QTextCursor>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), currentTheme(Theme::Dark), currentBaseFontSize(12)
{
    updateFormatsForTheme();
    setupInitialRules();
    setupPythonRules();
    setupCppRules();
    setupBashRules();
}

void MarkdownHighlighter::setTheme(Theme theme)
{
    if (currentTheme == theme) {
        return;
    }
    currentTheme = theme;
    updateFormatsForTheme();
    rehighlight();
}

void MarkdownHighlighter::updateFormatsForTheme()
{
    // Get colors from ThemeManager based on current theme
    ThemeManager *themeManager = ThemeManager::instance();
    if (!themeManager) return;
    
    // Map our Theme enum to ThemeManager's Theme enum
    ThemeManager::Theme tmTheme = ThemeManager::Theme::Dark;
    if (currentTheme == Theme::Light) {
        tmTheme = ThemeManager::Theme::Light;
    } else if (currentTheme == Theme::PitchBlack) {
        tmTheme = ThemeManager::Theme::PitchBlack;
    }
    
    // Get colors from ThemeManager
    QColor textColor = themeManager->textColor();
    QColor secondaryColor = themeManager->secondaryColor();
    QColor backgroundColor = themeManager->backgroundColor();
    QColor baseColor = themeManager->baseColor();
    QColor borderColor = themeManager->borderColor();
    
    // For colors not directly available from ThemeManager, derive them
    QColor headingColor = textColor;
    QColor boldColor = textColor;
    QColor italicColor = textColor;
    QColor strikethroughColor = textColor.darker(130);
    QColor codeTextColor = textColor.lighter(110);
    QColor codeBgColor = baseColor.darker(105);
    QColor linkColor = secondaryColor;
    QColor imageColor = textColor.darker(110);
    QColor listColor = textColor;
    QColor blockquoteColor = textColor.darker(120);
    QColor blockquoteBg = baseColor.darker(105);
    QColor tableHeaderBg = baseColor.darker(108);
    QColor tableCellBg = backgroundColor;
    QColor horizontalRuleColor = borderColor;
    QColor syntaxFaintColor = textColor.darker(120);

    // Update character formats
    const float heading1Ratio = 1.8f;
    const float heading2Ratio = 1.6f;
    const float heading3Ratio = 1.4f;
    const float heading4Ratio = 1.2f;

    heading1Format.setForeground(headingColor);
    heading1Format.setFontWeight(QFont::Bold);
    heading1Format.setFontPointSize(qRound(currentBaseFontSize * heading1Ratio));

    heading2Format.setForeground(headingColor);
    heading2Format.setFontWeight(QFont::Bold);
    heading2Format.setFontPointSize(qRound(currentBaseFontSize * heading2Ratio));

    heading3Format.setForeground(headingColor);
    heading3Format.setFontWeight(QFont::Bold);
    heading3Format.setFontPointSize(qRound(currentBaseFontSize * heading3Ratio));

    heading4Format.setForeground(headingColor);
    heading4Format.setFontWeight(QFont::Bold);
    heading4Format.setFontPointSize(qRound(currentBaseFontSize * heading4Ratio));

    heading5Format.setForeground(headingColor);
    heading5Format.setFontWeight(QFont::Bold);
    heading5Format.setFontPointSize(qRound(currentBaseFontSize * 1.1f));

    heading6Format.setForeground(headingColor);
    heading6Format.setFontWeight(QFont::Bold);
    heading6Format.setFontPointSize(currentBaseFontSize);

    boldFormat.setForeground(boldColor);
    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setFontPointSize(currentBaseFontSize);

    italicFormat.setForeground(italicColor);
    italicFormat.setFontItalic(true);
    italicFormat.setFontPointSize(currentBaseFontSize);

    strikethroughFormat.setForeground(strikethroughColor);
    strikethroughFormat.setFontStrikeOut(true);
    strikethroughFormat.setFontPointSize(currentBaseFontSize);

    codeFormat.setForeground(codeTextColor);
    codeFormat.setFontFamilies(QStringList() << "Monospace");
    codeFormat.setBackground(codeBgColor);
    codeFormat.setFontPointSize(currentBaseFontSize);

    linkFormat.setForeground(linkColor);
    linkFormat.setFontUnderline(true);
    linkFormat.setFontPointSize(currentBaseFontSize);

    imageFormat.setForeground(imageColor);
    imageFormat.setFontItalic(true);
    imageFormat.setFontPointSize(currentBaseFontSize);

    listFormat.setForeground(listColor);
    listFormat.setFontPointSize(currentBaseFontSize);

    taskListFormat.setForeground(listColor);
    taskListFormat.setFontWeight(QFont::Bold);
    taskListFormat.setFontPointSize(currentBaseFontSize);

    blockquoteFormat.setForeground(blockquoteColor);
    blockquoteFormat.setFontItalic(true);
    blockquoteFormat.setFontPointSize(currentBaseFontSize);

    tableHeaderFormat.setForeground(textColor);
    tableHeaderFormat.setFontWeight(QFont::Bold);
    tableHeaderFormat.setBackground(tableHeaderBg);
    tableHeaderFormat.setFontPointSize(currentBaseFontSize);

    tableCellFormat.setForeground(textColor);
    tableCellFormat.setBackground(tableCellBg);
    tableCellFormat.setFontPointSize(currentBaseFontSize);

    syntaxFaintFormat.setForeground(syntaxFaintColor);
    syntaxFaintFormat.setFontPointSize(currentBaseFontSize);

    // Code highlighting formats
    keywordFormat.setForeground(codeTextColor);
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setFontPointSize(currentBaseFontSize);

    commentFormat.setForeground(syntaxFaintColor);
    commentFormat.setFontItalic(true);
    commentFormat.setFontPointSize(currentBaseFontSize);

    stringFormat.setForeground(linkColor);
    stringFormat.setFontPointSize(currentBaseFontSize);

    numberFormat.setForeground(imageColor);
    numberFormat.setFontPointSize(currentBaseFontSize);

    functionFormat.setForeground(headingColor);
    functionFormat.setFontPointSize(currentBaseFontSize);

    // Update block formats
    heading1BlockFormat.setBottomMargin(15);
    heading1BlockFormat.setTopMargin(20);
    heading1BlockFormat.setLineHeight(120, QTextBlockFormat::FixedHeight);

    heading2BlockFormat.setBottomMargin(12);
    heading2BlockFormat.setTopMargin(18);
    heading2BlockFormat.setLineHeight(115, QTextBlockFormat::FixedHeight);

    heading3BlockFormat.setBottomMargin(10);
    heading3BlockFormat.setTopMargin(15);
    heading3BlockFormat.setLineHeight(110, QTextBlockFormat::FixedHeight);

    heading4BlockFormat.setBottomMargin(8);
    heading4BlockFormat.setTopMargin(12);
    heading4BlockFormat.setLineHeight(105, QTextBlockFormat::FixedHeight);

    heading5BlockFormat.setBottomMargin(6);
    heading5BlockFormat.setTopMargin(10);
    heading5BlockFormat.setLineHeight(102, QTextBlockFormat::FixedHeight);

    heading6BlockFormat.setBottomMargin(4);
    heading6BlockFormat.setTopMargin(8);
    heading6BlockFormat.setLineHeight(100, QTextBlockFormat::FixedHeight);

    codeBlockBlockFormat.setBackground(codeBgColor);
    codeBlockBlockFormat.setLineHeight(105, QTextBlockFormat::FixedHeight);

    blockquoteBlockFormat.setBackground(blockquoteBg);
    blockquoteBlockFormat.setLeftMargin(15);
    blockquoteBlockFormat.setLineHeight(105, QTextBlockFormat::FixedHeight);

    horizontalRuleBlockFormat.setBackground(QBrush(horizontalRuleColor));
    horizontalRuleBlockFormat.setLineHeight(1, QTextBlockFormat::FixedHeight);

    tableBlockFormat.setLineHeight(105, QTextBlockFormat::FixedHeight);
}

void MarkdownHighlighter::setupInitialRules()
{
    highlightingRules.clear();
    HighlightingRule rule;

    // Headings
    rule.pattern = QRegularExpression(QStringLiteral("^(#{1})\\s+(.+)$"));
    rule.format = heading1Format;
    rule.contentGroup = 2;
    rule.applyBlockFormat = true;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^(#{2})\\s+(.+)$"));
    rule.format = heading2Format;
    rule.contentGroup = 2;
    rule.applyBlockFormat = true;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^(#{3})\\s+(.+)$"));
    rule.format = heading3Format;
    rule.contentGroup = 2;
    rule.applyBlockFormat = true;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^(#{4})\\s+(.+)$"));
    rule.format = heading4Format;
    rule.contentGroup = 2;
    rule.applyBlockFormat = true;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^(#{5})\\s+(.+)$"));
    rule.format = heading5Format;
    rule.contentGroup = 2;
    rule.applyBlockFormat = true;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^(#{6})\\s+(.+)$"));
    rule.format = heading6Format;
    rule.contentGroup = 2;
    rule.applyBlockFormat = true;
    highlightingRules.append(rule);

    // Italic (asterisk)
    rule.pattern = QRegularExpression(QStringLiteral("(?<![\\w*])(\\*)([^*]+?)(\\*)(?![\\w*])"));
    rule.format = italicFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // Italic (underscore)
    rule.pattern = QRegularExpression(QStringLiteral("(?<![\\w_])(_[^_\\s][^_]*?_)(?![\\w_])"));
    rule.format = italicFormat;
    rule.contentGroup = 1;
    highlightingRules.append(rule);

    // Bold (asterisk)
    rule.pattern = QRegularExpression(QStringLiteral("(?<![\\w*])(\\*\\*)([^*]+?)(\\*\\*)(?![\\w*])"));
    rule.format = boldFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // Bold (underscore)
    rule.pattern = QRegularExpression(QStringLiteral("(?<![\\w_])(__[^_\\s][^_]*?__)(?![\\w_])"));
    rule.format = boldFormat;
    rule.contentGroup = 1;
    highlightingRules.append(rule);

    // Strikethrough
    rule.pattern = QRegularExpression(QStringLiteral("(~~)([^~]+?)(~~)"));
    rule.format = strikethroughFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // Inline Code
    rule.pattern = QRegularExpression(QStringLiteral("(`)([^`]+?)(`)"));
    rule.format = codeFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // Links
    rule.pattern = QRegularExpression(QStringLiteral("(\\[)([^\\]]+)(\\]\\()([^)]+)(\\))"));
    rule.format = linkFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // Images
    rule.pattern = QRegularExpression(QStringLiteral("(!\\[)([^\\]]+)(\\]\\()([^)]+)(\\))"));
    rule.format = imageFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // Lists
    rule.pattern = QRegularExpression(QStringLiteral("^([\\*\\-\\+])\\s+(.+)$"));
    rule.format = listFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^(\\d+\\.)\\s+(.+)$"));
    rule.format = listFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // Task Lists
    rule.pattern = QRegularExpression(QStringLiteral("^([\\*\\-\\+])\\s+\\[([ xX])\\]\\s+(.+)$"));
    rule.format = taskListFormat;
    rule.contentGroup = 3;
    highlightingRules.append(rule);

    // Blockquotes
    rule.pattern = QRegularExpression(QStringLiteral("^(>)\\s*(.+)$"));
    rule.format = blockquoteFormat;
    rule.contentGroup = 2;
    rule.applyBlockFormat = true;
    highlightingRules.append(rule);
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    // Apply block-level formatting
    if (text.startsWith("# ")) {
        currentBlock().setUserState(STATE_NORMAL);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading1BlockFormat);
    } else if (text.startsWith("## ")) {
        currentBlock().setUserState(STATE_NORMAL);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading2BlockFormat);
    } else if (text.startsWith("### ")) {
        currentBlock().setUserState(STATE_NORMAL);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading3BlockFormat);
    } else if (text.startsWith("#### ")) {
        currentBlock().setUserState(STATE_NORMAL);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading4BlockFormat);
    } else if (text.startsWith("##### ")) {
        currentBlock().setUserState(STATE_NORMAL);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading5BlockFormat);
    } else if (text.startsWith("###### ")) {
        currentBlock().setUserState(STATE_NORMAL);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading6BlockFormat);
    } else if (text.startsWith("> ")) {
        currentBlock().setUserState(STATE_NORMAL);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(blockquoteBlockFormat);
    } else if (text.trimmed().length() >= 3 &&
               (text.trimmed().startsWith("---") ||
                text.trimmed().startsWith("***") ||
                text.trimmed().startsWith("___"))) {
        currentBlock().setUserState(STATE_NORMAL);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(horizontalRuleBlockFormat);
        setFormat(0, text.length(), QTextCharFormat());
    }

    // Apply character-level formatting
    int state = previousBlockState();
    bool isInCodeBlock = (state == STATE_IN_CODE_BLOCK ||
                          state == STATE_IN_CODE_BLOCK_PYTHON ||
                          state == STATE_IN_CODE_BLOCK_CPP ||
                          state == STATE_IN_CODE_BLOCK_BASH);

    // Handle Code Block Start/End
    if (text.startsWith("```")) {
        if (isInCodeBlock) {
            state = STATE_NORMAL;
            isInCodeBlock = false;
        } else {
            QString lang = text.mid(3).trimmed().toLower();
            if (lang == "python" || lang == "py") {
                state = STATE_IN_CODE_BLOCK_PYTHON;
            } else if (lang == "cpp" || lang == "c++" || lang == "c") {
                state = STATE_IN_CODE_BLOCK_CPP;
            } else if (lang == "bash" || lang == "sh") {
                state = STATE_IN_CODE_BLOCK_BASH;
            } else {
                state = STATE_IN_CODE_BLOCK;
            }
            isInCodeBlock = true;
        }

        setFormat(0, text.length(), codeFormat);
        setCurrentBlockState(state);
        return;
    }

    // Apply code block highlighting
    if (isInCodeBlock) {
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(codeBlockBlockFormat);
        setFormat(0, text.length(), codeFormat);
        highlightCodeBlock(text, state);
        setCurrentBlockState(state);
        return;
    }

    setCurrentBlockState(STATE_NORMAL);

    // Apply standard markdown rules
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();

            int contentStart = -1;
            int contentLength = 0;

            if (rule.contentGroup > 0 && rule.contentGroup <= match.lastCapturedIndex()) {
                contentStart = match.capturedStart(rule.contentGroup);
                contentLength = match.capturedLength(rule.contentGroup);
            }

            if (contentLength <= 0) {
                for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
                    if (match.capturedLength(i) > contentLength) {
                        contentStart = match.capturedStart(i);
                        contentLength = match.capturedLength(i);
                    }
                }
            }

            if (contentLength > 0 && contentStart >= 0) {
                QTextCharFormat contentFormat = rule.format;
                QFont contentFont = contentFormat.font();
                qreal currentSize = rule.format.fontPointSize();
                contentFont.setPointSize(currentSize + (currentBaseFontSize - 12));
                contentFormat.setFont(contentFont);
                setFormat(contentStart, contentLength, contentFormat);
            }

            for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
                if (i != rule.contentGroup) {
                    setFormat(match.capturedStart(i),
                            match.capturedLength(i),
                            syntaxFaintFormat);
                }
            }
        }
    }
}

void MarkdownHighlighter::setFontSize(int baseSize)
{
    if (currentBaseFontSize == baseSize) {
        return;
    }

    currentBaseFontSize = baseSize;
    updateFormatsForTheme();
    rehighlight();
}

void MarkdownHighlighter::setupPythonRules()
{
    pythonRules.clear();
    HighlightingRule rule;

    QStringList keywords;
    keywords << "\\band\\b" << "\\bas\\b" << "\\bassert\\b" << "\\bbreak\\b" << "\\bclass\\b"
             << "\\bcontinue\\b" << "\\bdef\\b" << "\\bdel\\b" << "\\belif\\b" << "\\belse\\b"
             << "\\bexcept\\b" << "\\bexec\\b" << "\\bfinally\\b" << "\\bfor\\b" << "\\bfrom\\b"
             << "\\bglobal\\b" << "\\bif\\b" << "\\bimport\\b" << "\\bin\\b" << "\\bis\\b"
             << "\\blambda\\b" << "\\bnot\\b" << "\\bor\\b" << "\\bpass\\b" << "\\bprint\\b"
             << "\\braise\\b" << "\\breturn\\b" << "\\btry\\b" << "\\bwhile\\b" << "\\bwith\\b"
             << "\\byield\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        pythonRules.append(rule);
    }

    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = stringFormat;
    pythonRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = stringFormat;
    pythonRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = commentFormat;
    pythonRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+\\b"));
    rule.format = numberFormat;
    pythonRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    pythonRules.append(rule);
}

void MarkdownHighlighter::setupCppRules()
{
    cppRules.clear();
    HighlightingRule rule;

    QStringList keywords;
    keywords << "\\balignas\\b" << "\\balignof\\b" << "\\band\\b" << "\\band_eq\\b" << "\\basm\\b"
             << "\\bauto\\b" << "\\bbitand\\b" << "\\bbitor\\b" << "\\bbool\\b" << "\\bbreak\\b"
             << "\\bcase\\b" << "\\bcatch\\b" << "\\bchar\\b" << "\\bchar16_t\\b" << "\\bchar32_t\\b"
             << "\\bclass\\b" << "\\bcompl\\b" << "\\bconst\\b" << "\\bconstexpr\\b" << "\\bconst_cast\\b"
             << "\\bcontinue\\b" << "\\bdecltype\\b" << "\\bdefault\\b" << "\\bdelete\\b" << "\\bdo\\b"
             << "\\bdouble\\b" << "\\bdynamic_cast\\b" << "\\belse\\b" << "\\benum\\b" << "\\bexplicit\\b"
             << "\\bexport\\b" << "\\bextern\\b" << "\\bfalse\\b" << "\\bfloat\\b" << "\\bfor\\b"
             << "\\bfriend\\b" << "\\bgoto\\b" << "\\bif\\b" << "\\binline\\b" << "\\bint\\b"
             << "\\blong\\b" << "\\bmutable\\b" << "\\bnamespace\\b" << "\\bnew\\b" << "\\bnoexcept\\b"
             << "\\bnot\\b" << "\\bnot_eq\\b" << "\\bnullptr\\b" << "\\boperator\\b" << "\\bor\\b"
             << "\\bor_eq\\b" << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b" << "\\bregister\\b"
             << "\\breinterpret_cast\\b" << "\\breturn\\b" << "\\bshort\\b" << "\\bsigned\\b" << "\\bsizeof\\b"
             << "\\bstatic\\b" << "\\bstatic_assert\\b" << "\\bstatic_cast\\b" << "\\bstruct\\b" << "\\bswitch\\b"
             << "\\btemplate\\b" << "\\bthis\\b" << "\\bthread_local\\b" << "\\bthrow\\b" << "\\btrue\\b"
             << "\\btry\\b" << "\\btypedef\\b" << "\\btypeid\\b" << "\\btypename\\b" << "\\bunion\\b"
             << "\\bunsigned\\b" << "\\busing\\b" << "\\bvirtual\\b" << "\\bvoid\\b" << "\\bvolatile\\b"
             << "\\bwchar_t\\b" << "\\bwhile\\b" << "\\bxor\\b" << "\\bxor_eq\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        cppRules.append(rule);
    }

    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = stringFormat;
    cppRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = commentFormat;
    cppRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("/\\*.*\\*/"));
    rule.format = commentFormat;
    cppRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^\\s*#[^\n]*"));
    rule.format = functionFormat;
    cppRules.append(rule);
}

void MarkdownHighlighter::setupBashRules()
{
    bashRules.clear();
    HighlightingRule rule;

    QStringList keywords;
    keywords << "\\bif\\b" << "\\bthen\\b" << "\\belse\\b" << "\\belif\\b" << "\\bfi\\b"
             << "\\bcase\\b" << "\\besac\\b" << "\\bfor\\b" << "\\bwhile\\b" << "\\buntil\\b"
             << "\\bdo\\b" << "\\bdone\\b" << "\\bin\\b" << "\\bfunction\\b" << "\\bselect\\b"
             << "\\btime\\b" << "\\b\\[\\[\\b" << "\\b\\]\\]\\b" << "\\breturn\\b" << "\\bexit\\b";

    for (const QString &pattern : keywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        bashRules.append(rule);
    }

    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = stringFormat;
    bashRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = stringFormat;
    bashRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
    rule.format = commentFormat;
    bashRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("\\$[A-Za-z0-9_]+"));
    rule.format = numberFormat;
    bashRules.append(rule);
}

void MarkdownHighlighter::highlightCodeBlock(const QString &text, int languageState)
{
    QVector<HighlightingRule> *rules = nullptr;
    if (languageState == STATE_IN_CODE_BLOCK_PYTHON) {
        rules = &pythonRules;
    } else if (languageState == STATE_IN_CODE_BLOCK_CPP) {
        rules = &cppRules;
    } else if (languageState == STATE_IN_CODE_BLOCK_BASH) {
        rules = &bashRules;
    }

    if (!rules) return;

    for (const HighlightingRule &rule : *rules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
