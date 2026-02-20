#include "markdownhighlighter.h"
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QTextBlock>
#include <QTextCursor>
#include <QDebug>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), currentTheme(Theme::Dark)
{
    setupThemeColors();
    currentColors = darkColors;
    currentBaseFontSize = 12;
    updateFormatsForTheme();
    setupInitialRules();
    
    // Initialize language rules
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
    if (theme == Theme::Dark) {
        currentColors = darkColors;
    } else if (theme == Theme::PitchBlack) {
        currentColors = pitchBlackColors;
    } else {
        currentColors = lightColors;
    }
    updateFormatsForTheme();
    rehighlight();
}

void MarkdownHighlighter::setupThemeColors()
{
    // Light Theme Colors
    lightColors.background = QColor(255, 255, 255);
    lightColors.text = QColor(36, 41, 47);
    lightColors.heading = QColor(36, 41, 47);
    lightColors.bold = QColor(36, 41, 47);
    lightColors.italic = QColor(36, 41, 47);
    lightColors.strikethrough = QColor(100, 100, 100);
    lightColors.codeText = QColor(156, 39, 176);
    lightColors.codeBackground = QColor(246, 248, 250);
    lightColors.link = QColor(3, 102, 214);
    lightColors.image = QColor(106, 115, 125);
    lightColors.list = QColor(36, 41, 47);
    lightColors.taskList = QColor(36, 41, 47);
    lightColors.blockquoteText = QColor(106, 115, 125);
    lightColors.blockquoteBackground = QColor(246, 248, 250);
    lightColors.tableHeaderText = QColor(36, 41, 47);
    lightColors.tableCellText = QColor(36, 41, 47);
    lightColors.tableHeaderBackground = QColor(246, 248, 250);
    lightColors.tableCellBackground = QColor(255, 255, 255);
    lightColors.horizontalRule = QColor(220, 220, 220);
    lightColors.syntaxFaint = QColor(150, 150, 150);
    lightColors.secondary = QColor(3, 102, 214);  // Sync with ThemeManager secondary

    // Dark Theme Colors
    darkColors.background = QColor(13, 17, 23);
    darkColors.text = QColor(225, 228, 232);
    darkColors.heading = QColor(225, 228, 232);
    darkColors.bold = QColor(225, 228, 232);
    darkColors.italic = QColor(225, 228, 232);
    darkColors.strikethrough = QColor(180, 180, 180);
    darkColors.codeText = QColor(198, 120, 221);
    darkColors.codeBackground = QColor(22, 27, 34);
    darkColors.link = QColor(88, 166, 255);
    darkColors.image = QColor(139, 148, 158);
    darkColors.list = QColor(225, 228, 232);
    darkColors.taskList = QColor(225, 228, 232);
    darkColors.blockquoteText = QColor(139, 148, 158);
    darkColors.blockquoteBackground = QColor(22, 27, 34);
    darkColors.tableHeaderText = QColor(225, 228, 232);
    darkColors.tableCellText = QColor(225, 228, 232);
    darkColors.tableHeaderBackground = QColor(30, 35, 45);
    darkColors.tableCellBackground = QColor(25, 30, 35);
    darkColors.horizontalRule = QColor(60, 65, 75);
    darkColors.syntaxFaint = QColor(100, 100, 100);
    darkColors.secondary = QColor(88, 166, 255);  // Sync with ThemeManager secondary

    // Pitch Black Theme Colors
    pitchBlackColors.background = QColor(0, 0, 0);
    pitchBlackColors.text = QColor(224, 224, 224);
    pitchBlackColors.heading = QColor(224, 224, 224);
    pitchBlackColors.bold = QColor(224, 224, 224);
    pitchBlackColors.italic = QColor(224, 224, 224);
    pitchBlackColors.strikethrough = QColor(128, 128, 128);
    pitchBlackColors.codeText = QColor(200, 200, 200);
    pitchBlackColors.codeBackground = QColor(20, 20, 20);
    pitchBlackColors.link = QColor(135, 207, 62);  // Mint's green (#87CF3E)
    pitchBlackColors.image = QColor(150, 150, 150);
    pitchBlackColors.list = QColor(224, 224, 224);
    pitchBlackColors.taskList = QColor(224, 224, 224);
    pitchBlackColors.blockquoteText = QColor(180, 180, 180);
    pitchBlackColors.blockquoteBackground = QColor(20, 20, 20);
    pitchBlackColors.tableHeaderText = QColor(224, 224, 224);
    pitchBlackColors.tableCellText = QColor(224, 224, 224);
    pitchBlackColors.tableHeaderBackground = QColor(30, 30, 30);
    pitchBlackColors.tableCellBackground = QColor(0, 0, 0);
    pitchBlackColors.horizontalRule = QColor(80, 80, 80);
    pitchBlackColors.syntaxFaint = QColor(80, 80, 80);
    pitchBlackColors.secondary = QColor(135, 207, 62);  // Mint's green (#87CF3E)
}

void MarkdownHighlighter::updateFormatsForTheme()
{
    // --- Update QTextCharFormats ---
    const float heading1Ratio = 1.8f;  
    const float heading2Ratio = 1.6f;  
    const float heading3Ratio = 1.4f;  
    const float heading4Ratio = 1.2f;  
    
    heading1Format.setForeground(currentColors.heading);
    heading1Format.setFontWeight(QFont::Bold);
    heading1Format.setFontPointSize(qRound(currentBaseFontSize * heading1Ratio));

    heading2Format.setForeground(currentColors.heading);
    heading2Format.setFontWeight(QFont::Bold);
    heading2Format.setFontPointSize(qRound(currentBaseFontSize * heading2Ratio));

    heading3Format.setForeground(currentColors.heading);
    heading3Format.setFontWeight(QFont::Bold);
    heading3Format.setFontPointSize(qRound(currentBaseFontSize * heading3Ratio));

    heading4Format.setForeground(currentColors.heading);
    heading4Format.setFontWeight(QFont::Bold);
    heading4Format.setFontPointSize(qRound(currentBaseFontSize * heading4Ratio));

    heading5Format.setForeground(currentColors.heading);
    heading5Format.setFontWeight(QFont::Bold);
    heading5Format.setFontPointSize(qRound(currentBaseFontSize * 1.1f));

    heading6Format.setForeground(currentColors.heading);
    heading6Format.setFontWeight(QFont::Bold);
    heading6Format.setFontPointSize(currentBaseFontSize);

    boldFormat.setForeground(currentColors.bold);
    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setFontPointSize(currentBaseFontSize);

    italicFormat.setForeground(currentColors.italic);
    italicFormat.setFontItalic(true);
    italicFormat.setFontPointSize(currentBaseFontSize);

    strikethroughFormat.setForeground(currentColors.strikethrough);
    strikethroughFormat.setFontStrikeOut(true);
    strikethroughFormat.setFontPointSize(currentBaseFontSize);

    codeFormat.setForeground(currentColors.codeText);
    codeFormat.setFontFamilies(QStringList() << "Monospace");
    codeFormat.setBackground(currentColors.codeBackground);
    codeFormat.setFontPointSize(currentBaseFontSize);

    linkFormat.setForeground(currentColors.link);
    linkFormat.setFontUnderline(true);
    linkFormat.setFontPointSize(currentBaseFontSize);

    imageFormat.setForeground(currentColors.image);
    imageFormat.setFontItalic(true);
    imageFormat.setFontPointSize(currentBaseFontSize);

    listFormat.setForeground(currentColors.list);
    listFormat.setFontPointSize(currentBaseFontSize);

    taskListFormat.setForeground(currentColors.taskList);
    taskListFormat.setFontWeight(QFont::Bold);
    taskListFormat.setFontPointSize(currentBaseFontSize);

    blockquoteFormat.setForeground(currentColors.blockquoteText);
    blockquoteFormat.setFontItalic(true);
    blockquoteFormat.setFontPointSize(currentBaseFontSize);

    tableHeaderFormat.setForeground(currentColors.tableHeaderText);
    tableHeaderFormat.setFontWeight(QFont::Bold);
    tableHeaderFormat.setBackground(currentColors.tableHeaderBackground);
    tableHeaderFormat.setFontPointSize(currentBaseFontSize);

    tableCellFormat.setForeground(currentColors.tableCellText);
    tableCellFormat.setBackground(currentColors.tableCellBackground);
    tableCellFormat.setFontPointSize(currentBaseFontSize);

    // Syntax characters
    syntaxFaintFormat.setForeground(currentColors.syntaxFaint);
    syntaxFaintFormat.setFontPointSize(currentBaseFontSize);

    // --- Code Highlighting Formats ---
    keywordFormat.setForeground(currentColors.codeText);
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setFontPointSize(currentBaseFontSize);

    commentFormat.setForeground(currentColors.syntaxFaint);
    commentFormat.setFontItalic(true);
    commentFormat.setFontPointSize(currentBaseFontSize);

    stringFormat.setForeground(currentColors.link);
    stringFormat.setFontPointSize(currentBaseFontSize);
    
    numberFormat.setForeground(currentColors.image);
    numberFormat.setFontPointSize(currentBaseFontSize);

    functionFormat.setForeground(currentColors.heading);
    functionFormat.setFontPointSize(currentBaseFontSize);

    // --- Update Block Formats ---
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

    codeBlockBlockFormat.setBackground(currentColors.codeBackground);
    codeBlockBlockFormat.setLineHeight(105, QTextBlockFormat::FixedHeight);

    blockquoteBlockFormat.setBackground(currentColors.blockquoteBackground);
    blockquoteBlockFormat.setLeftMargin(15);
    blockquoteBlockFormat.setLineHeight(105, QTextBlockFormat::FixedHeight);

    horizontalRuleBlockFormat.setBackground(QBrush(currentColors.horizontalRule));
    horizontalRuleBlockFormat.setLineHeight(1, QTextBlockFormat::FixedHeight);
    horizontalRuleBlockFormat.setLineHeight(1, QTextBlockFormat::FixedHeight);

    tableBlockFormat.setLineHeight(105, QTextBlockFormat::FixedHeight);
}

void MarkdownHighlighter::setupInitialRules()
{
    highlightingRules.clear();
    HighlightingRule rule;

    // --- Headings ---
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

    // --- Italic (asterisk) ---
    // Asterisks can be used more freely, but require word boundaries
    rule.pattern = QRegularExpression(QStringLiteral("(?<![\\w*])(\\*)([^*]+?)(\\*)(?![\\w*])"));
    rule.format = italicFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // --- Italic (underscore) ---
    // Underscores for italic must have word boundaries and not be part of identifiers
    // Match single underscores that surround text, with word boundaries
    rule.pattern = QRegularExpression(QStringLiteral("(?<![\\w_])(_[^_\\s][^_]*?_)(?![\\w_])"));
    rule.format = italicFormat;
    rule.contentGroup = 1;
    highlightingRules.append(rule);

    // --- Bold (asterisk) ---
    rule.pattern = QRegularExpression(QStringLiteral("(?<![\\w*])(\\*\\*)([^*]+?)(\\*\\*)(?![\\w*])"));
    rule.format = boldFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // --- Bold (underscore) ---
    // Double underscores with strict word boundaries to avoid matching __attribute__ etc.
    rule.pattern = QRegularExpression(QStringLiteral("(?<![\\w_])(__[^_\\s][^_]*?__)(?![\\w_])"));
    rule.format = boldFormat;
    rule.contentGroup = 1;
    highlightingRules.append(rule);

    // --- Strikethrough ---
    rule.pattern = QRegularExpression(QStringLiteral("(~~)([^~]+?)(~~)"));
    rule.format = strikethroughFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // --- Inline Code ---
    rule.pattern = QRegularExpression(QStringLiteral("(`)([^`]+?)(`)"));
    rule.format = codeFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // --- Links ---
    rule.pattern = QRegularExpression(QStringLiteral("(\\[)([^\\]]+)(\\]\\()([^)]+)(\\))"));
    rule.format = linkFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // --- Images ---
    rule.pattern = QRegularExpression(QStringLiteral("(!\\[)([^\\]]+)(\\]\\()([^)]+)(\\))"));
    rule.format = imageFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // --- Lists ---
    rule.pattern = QRegularExpression(QStringLiteral("^([\\*\\-\\+])\\s+(.+)$"));
    rule.format = listFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^(\\d+\\.)\\s+(.+)$"));
    rule.format = listFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // --- Task Lists ---
    rule.pattern = QRegularExpression(QStringLiteral("^([\\*\\-\\+])\\s+\\[([ xX])\\]\\s+(.+)$"));
    rule.format = taskListFormat;
    rule.contentGroup = 3;  
    highlightingRules.append(rule);

    // --- Blockquotes ---
    rule.pattern = QRegularExpression(QStringLiteral("^(>)\\s*(.+)$"));
    rule.format = blockquoteFormat;
    rule.contentGroup = 2;
    rule.applyBlockFormat = true;
    highlightingRules.append(rule);
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    // First apply block-level formatting
    if (text.startsWith("# ")) {
        currentBlock().setUserState(STATE_NORMAL); // Reset state
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
    } else {
        // Only reset to normal if not inside a code block or table (handled below)
        // currentBlock().setUserState(STATE_NORMAL);
    }

    // Apply character-level formatting (standard markdown)
    // IMPORTANT: Only apply if NOT in a code block (except for the fence itself)
    int state = previousBlockState();
    bool isInCodeBlock = (state == STATE_IN_CODE_BLOCK || 
                          state == STATE_IN_CODE_BLOCK_PYTHON ||
                          state == STATE_IN_CODE_BLOCK_CPP ||
                          state == STATE_IN_CODE_BLOCK_BASH);

    // Handle Code Block Start/End
    if (text.startsWith("```")) {
        if (isInCodeBlock) {
             // Ending a block
             state = STATE_NORMAL;
             isInCodeBlock = false;
        } else {
             // Starting a block
             // Check language
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
        
        // Format the fence line itself
        setFormat(0, text.length(), codeFormat);
        setCurrentBlockState(state);
        return; // Don't apply other rules to the fence
    }
    
    // If we are inside a code block, apply specific rules
    if (isInCodeBlock) {
        // Apply block background
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(codeBlockBlockFormat);
        
        // Apply base code color
        setFormat(0, text.length(), codeFormat);
        
        // Apply language highlighting
        highlightCodeBlock(text, state);
        
        setCurrentBlockState(state);
        return; // Skip standard markdown rules inside code block
    }
    
    // Normal Markdown Rules
    setCurrentBlockState(STATE_NORMAL);

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
    
    // Handle tables (simple state)
    // ... (Table logic kept simple or omitted for brevity if not focus, but preserved if present)
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
             << "\\btime\\b" << "\\b[[\\b" << "\\b]]\\b" << "\\breturn\\b" << "\\bexit\\b";

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
