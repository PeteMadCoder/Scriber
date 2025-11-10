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
}

void MarkdownHighlighter::setTheme(Theme theme)
{
    if (currentTheme == theme) {
        return;
    }
    currentTheme = theme;
    currentColors = (theme == Theme::Dark) ? darkColors : lightColors;
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
    lightColors.syntaxFaint = QColor(150, 150, 150); // Faint but visible

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
    darkColors.syntaxFaint = QColor(100, 100, 100); // Faint but visible
}

void MarkdownHighlighter::updateFormatsForTheme()
{
    // --- Update QTextCharFormats ---
    const float heading1Ratio = 1.8f;  // 80% larger
    const float heading2Ratio = 1.6f;  // 60% larger
    const float heading3Ratio = 1.4f;  // 40% larger
    const float heading4Ratio = 1.2f;  // 20% larger
    
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

    // Syntax characters - FAINT but VISIBLE
    syntaxFaintFormat.setForeground(currentColors.syntaxFaint);
    syntaxFaintFormat.setFontPointSize(currentBaseFontSize);

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

    // --- Italic ---
    rule.pattern = QRegularExpression(QStringLiteral("(\\*)([^_]+?)(\\*)"));
    rule.format = italicFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("(_)([^_]+?)(_)"));
    rule.format = italicFormat;
    rule.contentGroup = 2;
    highlightingRules.append(rule);

    // --- Bold ---
    rule.pattern = QRegularExpression(QStringLiteral("\\*\\*(.*?)\\*\\*"));
    rule.format = boldFormat;
    rule.contentGroup = 1;  // Now the content is in group 1
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("(?<!_)__([^_]+?)__(?!_)"));
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
    rule.contentGroup = 3;  // The actual task text
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
        currentBlock().setUserState(1);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading1BlockFormat);
    } else if (text.startsWith("## ")) {
        currentBlock().setUserState(2);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading2BlockFormat);
    } else if (text.startsWith("### ")) {
        currentBlock().setUserState(3);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading3BlockFormat);
    } else if (text.startsWith("#### ")) {
        currentBlock().setUserState(4);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading4BlockFormat);
    } else if (text.startsWith("##### ")) {
        currentBlock().setUserState(5);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading5BlockFormat);
    } else if (text.startsWith("###### ")) {
        currentBlock().setUserState(6);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(heading6BlockFormat);
    } else if (text.startsWith("> ")) {
        currentBlock().setUserState(7);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(blockquoteBlockFormat);
    } else if (text.trimmed().length() >= 3 && 
               (text.trimmed().startsWith("---") || 
                text.trimmed().startsWith("***") || 
                text.trimmed().startsWith("___"))) {
        currentBlock().setUserState(8);
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(horizontalRuleBlockFormat);
        setFormat(0, text.length(), QTextCharFormat());
    } else {
        currentBlock().setUserState(0);
    }

    // Apply character-level formatting
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            
            // Find the content portion to format
            int contentStart = -1;
            int contentLength = 0;
            
            // Try to use the explicitly specified content group
            if (rule.contentGroup > 0 && rule.contentGroup <= match.lastCapturedIndex()) {
                contentStart = match.capturedStart(rule.contentGroup);
                contentLength = match.capturedLength(rule.contentGroup);
            }
            
            // If content group is invalid, find the longest group
            if (contentLength <= 0) {
                for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
                    if (match.capturedLength(i) > contentLength) {
                        contentStart = match.capturedStart(i);
                        contentLength = match.capturedLength(i);
                    }
                }
            }
            
            // Apply formatting to the content
            if (contentLength > 0 && contentStart >= 0) {
                // CORRECT: Apply the rule format directly without resetting font size
                QTextCharFormat contentFormat = rule.format;
                QFont contentFont = contentFormat.font();
                qreal currentSize = rule.format.fontPointSize();
                contentFont.setPointSize(currentSize + (currentBaseFontSize - 12));
                contentFormat.setFont(contentFont);
                setFormat(contentStart, contentLength, contentFormat);
            }
            
            // Format syntax characters (faint but visible)
            for (int i = 1; i <= match.lastCapturedIndex(); ++i) {
                if (i != rule.contentGroup) {
                    // CORRECT: Apply the syntax format directly
                    setFormat(match.capturedStart(i), 
                            match.capturedLength(i), 
                            syntaxFaintFormat);
                }
            }
        }
    }

    // Special handling for code blocks (fenced)
    int state = previousBlockState();
    bool isInCodeBlock = (state == STATE_IN_CODE_BLOCK);
    
    if (!isInCodeBlock && text.startsWith("```")) {
        isInCodeBlock = true;
        state = STATE_IN_CODE_BLOCK;
    }
    
    if (isInCodeBlock) {
        if (text.startsWith("```") && text.length() == 3) {
            isInCodeBlock = false;
            state = STATE_NORMAL;
        } else {
            QTextCursor cursor(currentBlock());
            cursor.setBlockFormat(codeBlockBlockFormat);
            setFormat(0, text.length(), codeFormat);
        }
    }
    
    // Handle tables
    bool isInTable = (state == STATE_IN_TABLE || 
                     (previousBlockState() == STATE_IN_TABLE && text.trimmed().startsWith("|")));
    
    // Check for table row (lines with |)
    if (text.contains(QRegularExpression("\\|.*\\|"))) {
        state = STATE_IN_TABLE;
        QTextCursor cursor(currentBlock());
        cursor.setBlockFormat(tableBlockFormat);
        
        // Check for header separator (lines with |---|)
        if (QRegularExpression("\\|\\s*:?-+:?\\s*\\|").match(text).hasMatch()) {
            // Format as table header separator (don't apply to content)
        } else {
            // Format table cells
            int pos = 0;
            while ((pos = text.indexOf('|', pos)) != -1) {
                // Skip the pipe character itself
                pos++;
                
                // Find the next pipe or end of line
                int endPos = text.indexOf('|', pos);
                if (endPos == -1) endPos = text.length();
                
                // Skip empty cells or whitespace
                QString cell = text.mid(pos, endPos - pos).trimmed();
                if (!cell.isEmpty() && cell != ":") {
                    // Determine if this is a header cell (if previous line was separator)
                    QTextCharFormat cellFormat = tableCellFormat;
                    if (currentBlock().previous().isValid() && 
                        QRegularExpression("\\|\\s*:?-+:?\\s*\\|").match(currentBlock().previous().text()).hasMatch()) {
                        cellFormat = tableHeaderFormat;
                    }
                    
                    // Format the cell content (skip leading/trailing whitespace)
                    int contentStart = pos + text.mid(pos, endPos - pos).indexOf(cell);
                    int contentLength = cell.length();
                    setFormat(contentStart, contentLength, cellFormat);
                }
                
                pos = endPos;
            }
        }
    } else if (isInTable) {
        state = STATE_NORMAL;
    }
    
    setCurrentBlockState(state);
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