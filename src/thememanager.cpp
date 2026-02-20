#include "thememanager.h"
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QSettings>
#include <QList>
#include <QWidget>
#include <QComboBox>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <QTreeView>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QScrollBar>
#include <QStandardPaths>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDebug>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), m_currentTheme(Theme::Dark)
{
    loadBuiltInThemes();
    loadThemeFromSettings();
    applyCurrentTheme();
}

QString ThemeManager::themeToId(Theme theme) const
{
    switch (theme) {
        case Theme::Light: return "light";
        case Theme::Dark: return "dark";
        case Theme::PitchBlack: return "pitchblack";
        default: return "dark";
    }
}

void ThemeManager::loadBuiltInThemes()
{
    // Load themes from Qt resource system
    QStringList themeResources = {
        ":/resources/themes/light.json",
        ":/resources/themes/dark.json",
        ":/resources/themes/pitchblack.json"
    };
    
    for (const QString &resourcePath : themeResources) {
        QFile file(resourcePath);
        if (file.exists()) {
            loadThemeFromJson(resourcePath);
        } else {
            qWarning() << "ThemeManager: Theme resource not found:" << resourcePath;
        }
    }
    
    // If no themes loaded, create minimal defaults
    if (m_themes.isEmpty()) {
        qWarning() << "ThemeManager: No theme files found, using minimal defaults";
        
        ThemeColors defaultTheme;
        defaultTheme.background = QColor(245, 245, 245);
        defaultTheme.text = QColor(36, 41, 47);
        defaultTheme.window = QColor(240, 240, 240);
        defaultTheme.windowText = QColor(36, 41, 47);
        defaultTheme.base = QColor(255, 255, 255);
        defaultTheme.alternateBase = QColor(235, 235, 235);
        defaultTheme.highlight = QColor(0, 120, 215);
        defaultTheme.highlightedText = QColor(255, 255, 255);
        defaultTheme.button = QColor(220, 220, 220);
        defaultTheme.buttonText = QColor(36, 41, 47);
        defaultTheme.border = QColor(200, 200, 200);
        defaultTheme.tooltip = QColor(255, 255, 220);
        defaultTheme.tooltipText = QColor(36, 41, 47);
        defaultTheme.secondary = QColor(3, 102, 214);
        defaultTheme.name = "Default";
        
        m_themes["light"] = defaultTheme;
    }
}

bool ThemeManager::loadThemeFromJson(const QString &jsonPath)
{
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ThemeManager: Failed to open theme file:" << jsonPath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull()) {
        qWarning() << "ThemeManager: Invalid JSON in theme file:" << jsonPath;
        return false;
    }

    QJsonObject root = doc.object();
    QString themeId = root["id"].toString();
    
    if (themeId.isEmpty()) {
        qWarning() << "ThemeManager: Theme file missing 'id' field:" << jsonPath;
        return false;
    }

    QJsonObject colors = root["colors"].toObject();
    QJsonObject markdownColors = root["markdownColors"].toObject();

    ThemeColors theme;

    // Load UI colors
    theme.background = QColor(colors["background"].toString("#FFFFFF"));
    theme.text = QColor(colors["text"].toString("#000000"));
    theme.window = QColor(colors["window"].toString("#FFFFFF"));
    theme.windowText = QColor(colors["windowText"].toString("#000000"));
    theme.base = QColor(colors["base"].toString("#FFFFFF"));
    theme.alternateBase = QColor(colors["alternateBase"].toString("#F0F0F0"));
    theme.highlight = QColor(colors["highlight"].toString("#0078D7"));
    theme.highlightedText = QColor(colors["highlightedText"].toString("#FFFFFF"));
    theme.button = QColor(colors["button"].toString("#DCDCDC"));
    theme.buttonText = QColor(colors["buttonText"].toString("#000000"));
    theme.border = QColor(colors["border"].toString("#C8C8C8"));
    theme.tooltip = QColor(colors["tooltip"].toString("#FFFFDC"));
    theme.tooltipText = QColor(colors["tooltipText"].toString("#000000"));
    theme.secondary = QColor(colors["secondary"].toString("#0078D7"));

    // Load Markdown colors
    theme.heading = QColor(markdownColors["heading"].toString(theme.text.name()));
    theme.bold = QColor(markdownColors["bold"].toString(theme.text.name()));
    theme.italic = QColor(markdownColors["italic"].toString(theme.text.name()));
    theme.strikethrough = QColor(markdownColors["strikethrough"].toString("#646464"));
    theme.codeText = QColor(markdownColors["codeText"].toString("#9C27B0"));
    theme.codeBackground = QColor(markdownColors["codeBackground"].toString("#F6F8FA"));
    theme.link = QColor(markdownColors["link"].toString(theme.secondary.name()));
    theme.image = QColor(markdownColors["image"].toString("#6A737D"));
    theme.list = QColor(markdownColors["list"].toString(theme.text.name()));
    theme.taskList = QColor(markdownColors["taskList"].toString(theme.text.name()));
    theme.blockquoteText = QColor(markdownColors["blockquoteText"].toString("#6A737D"));
    theme.blockquoteBackground = QColor(markdownColors["blockquoteBackground"].toString("#F6F8FA"));
    theme.tableHeaderText = QColor(markdownColors["tableHeaderText"].toString(theme.text.name()));
    theme.tableCellText = QColor(markdownColors["tableCellText"].toString(theme.text.name()));
    theme.tableHeaderBackground = QColor(markdownColors["tableHeaderBackground"].toString("#F6F8FA"));
    theme.tableCellBackground = QColor(markdownColors["tableCellBackground"].toString(theme.background.name()));
    theme.horizontalRule = QColor(markdownColors["horizontalRule"].toString("#DCDCDC"));
    theme.syntaxFaint = QColor(markdownColors["syntaxFaint"].toString("#969696"));

    // Load metadata
    theme.name = root["name"].toString(themeId);
    theme.description = root["description"].toString("");

    m_themes[themeId] = theme;
    
    return true;
}

void ThemeManager::loadThemeFromSettings()
{
    QSettings settings;
    QString themeId = settings.value("theme/id", "dark").toString();
    
    // Validate theme exists
    if (!m_themes.contains(themeId)) {
        themeId = "dark";
        if (!m_themes.contains(themeId)) {
            themeId = m_themes.keys().first();
        }
    }
    
    // Map theme ID to enum
    if (themeId == "light") {
        m_currentTheme = Theme::Light;
    } else if (themeId == "pitchblack") {
        m_currentTheme = Theme::PitchBlack;
    } else {
        m_currentTheme = Theme::Dark;
    }
    
    m_currentThemeId = themeId;
}

void ThemeManager::saveThemeToSettings()
{
    QSettings settings;
    settings.setValue("theme/id", m_currentThemeId);
    settings.sync();
}

void ThemeManager::setTheme(Theme theme)
{
    QString newThemeId = themeToId(theme);
    
    if (m_currentThemeId == newThemeId) {
        return;
    }

    m_currentTheme = theme;
    m_currentThemeId = newThemeId;
    
    applyCurrentTheme();
    saveThemeToSettings();
    emit themeChanged(theme);
}

void ThemeManager::applyCurrentTheme()
{
    if (!m_themes.contains(m_currentThemeId)) {
        qWarning() << "ThemeManager: Theme not found:" << m_currentThemeId;
        return;
    }
    
    m_currentColors = m_themes[m_currentThemeId];
    applyGlobalPalette();
    applyApplicationStylesheet();
}

QStringList ThemeManager::availableThemeIds() const
{
    return m_themes.keys();
}

QString ThemeManager::getThemeName(const QString &themeId) const
{
    if (m_themes.contains(themeId)) {
        return m_themes[themeId].name;
    }
    return themeId;
}

void ThemeManager::applyGlobalPalette()
{
    QPalette palette;

    // Window
    palette.setColor(QPalette::Window, m_currentColors.window);
    palette.setColor(QPalette::WindowText, m_currentColors.windowText);

    // Base (text entry areas)
    palette.setColor(QPalette::Base, m_currentColors.base);
    palette.setColor(QPalette::AlternateBase, m_currentColors.alternateBase);

    // Text
    palette.setColor(QPalette::Text, m_currentColors.text);

    // Button
    palette.setColor(QPalette::Button, m_currentColors.button);
    palette.setColor(QPalette::ButtonText, m_currentColors.buttonText);

    // Highlight
    palette.setColor(QPalette::Highlight, m_currentColors.highlight);
    palette.setColor(QPalette::HighlightedText, m_currentColors.highlightedText);

    // Tooltip
    palette.setColor(QPalette::ToolTipBase, m_currentColors.tooltip);
    palette.setColor(QPalette::ToolTipText, m_currentColors.tooltipText);

    // Disabled colors
    palette.setColor(QPalette::Disabled, QPalette::WindowText,
                     m_currentColors.windowText.darker(150));
    palette.setColor(QPalette::Disabled, QPalette::Text,
                     m_currentColors.text.darker(150));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                     m_currentColors.buttonText.darker(150));

    QApplication::setPalette(palette);
}

QString ThemeManager::buildStylesheet() const
{
    const QColor &bg = m_currentColors.background;
    const QColor &text = m_currentColors.text;
    const QColor &window = m_currentColors.window;
    const QColor &windowText = m_currentColors.windowText;
    const QColor &base = m_currentColors.base;
    const QColor &highlight = m_currentColors.highlight;
    const QColor &highlightedText = m_currentColors.highlightedText;
    const QColor &button = m_currentColors.button;
    const QColor &buttonText = m_currentColors.buttonText;
    const QColor &border = m_currentColors.border;
    const QColor &secondary = m_currentColors.secondary;

    return QString(R"(
        /* Global Styles */
        QWidget {
            background-color: %1;
            color: %2;
            font-family: "Segoe UI", Arial, sans-serif;
            font-size: 12px;
        }

        /* Main Window */
        QMainWindow {
            background-color: %1;
        }

        /* Menu Bar */
        QMenuBar {
            background-color: %3;
            color: %4;
            padding: 2px;
            border-bottom: 1px solid %9;
        }
        QMenuBar::item:selected {
            background-color: %12;
            color: %7;
        }
        QMenuBar::item:pressed {
            background-color: %12;
            color: %7;
        }

        /* Menus */
        QMenu {
            background-color: %3;
            border: 1px solid %9;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 20px 6px 20px;
        }
        QMenu::item:selected {
            background-color: %12;
            color: %7;
        }
        QMenu::separator {
            height: 1px;
            background: %9;
            margin: 4px 10px 4px 10px;
        }

        /* Tool Bar */
        QToolBar {
            background-color: %3;
            border-bottom: 1px solid %9;
            padding: 2px;
            spacing: 4px;
        }
        QToolBar::separator {
            background-color: %9;
            width: 1px;
            margin: 4px;
        }

        /* Status Bar */
        QStatusBar {
            background-color: %3;
            border-top: 1px solid %9;
            padding: 2px;
        }

        /* Dock Widgets */
        QDockWidget {
            titlebar-close-icon: none;
            titlebar-normal-icon: none;
        }
        QDockWidget::title {
            background-color: %3;
            padding: 4px;
            border-bottom: 1px solid %9;
        }
        QDockWidget::close-button,
        QDockWidget::float-button {
            border: none;
            padding: 4px;
        }
        QDockWidget::close-button:hover,
        QDockWidget::float-button:hover {
            background-color: %12;
            color: %7;
        }

        /* Tab Widgets */
        QTabWidget::pane {
            border: 1px solid %9;
            background-color: %1;
        }
        QTabBar::tab {
            background-color: %3;
            color: %4;
            padding: 6px 12px;
            border: 1px solid %9;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: %1;
            border-top: 2px solid %12;
        }
        QTabBar::tab:hover:!selected {
            background-color: %5;
        }

        /* Tree Views (File Explorer, Outline) */
        QTreeView {
            background-color: %5;
            border: none;
            outline: none;
            color: %2;
            alternate-background-color: %5;
            show-decoration-selected: 1;
        }
        QTreeView::item {
            padding: 4px;
            color: %2;
            border: none;
        }
        QTreeView::item:hover {
            background-color: %12;
        }
        QTreeView::item:selected {
            background-color: %12;
            color: %7;
        }
        QTreeView::branch {
            background-color: %5;
            color: %2;
            border: none;
        }
        QTreeView::branch:has-children:!has-siblings {
            border: none;
            image: none;
        }
        QTreeView::branch:has-children:has-siblings {
            border: none;
            image: none;
        }
        QTreeView::branch:closed:has-children::indicator {
            border-image: none;
            image: none;
        }
        QTreeView::branch:open:has-children::indicator {
            border-image: none;
            image: none;
        }
        QTreeView::branch:!has-children {
            border: none;
            image: none;
        }

        /* Scroll Bars */
        QScrollBar:vertical {
            background-color: %3;
            width: 12px;
            border-radius: 6px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background-color: %9;
            border-radius: 5px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %12;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0;
        }
        QScrollBar:horizontal {
            background-color: %3;
            height: 12px;
            border-radius: 6px;
            margin: 0;
        }
        QScrollBar::handle:horizontal {
            background-color: %9;
            border-radius: 5px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: %12;
        }
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0;
        }

        /* Buttons */
        QPushButton {
            background-color: %6;
            color: %8;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: %12;
            color: %7;
            border-color: %12;
        }
        QPushButton:pressed {
            background-color: %12;
            border: 1px solid %12;
        }
        QPushButton:flat {
            border: none;
            background-color: transparent;
        }
        QPushButton:flat:hover {
            background-color: %12;
        }

        /* Line Edits */
        QLineEdit {
            background-color: %5;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 4px 8px;
            selection-background-color: %12;
            selection-color: %7;
        }
        QLineEdit:focus {
            border: 1px solid %12;
        }

        /* Check Boxes */
        QCheckBox {
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid %9;
            border-radius: 4px;
            background-color: %5;
        }
        QCheckBox::indicator:hover {
            border-color: %12;
        }
        QCheckBox::indicator:checked {
            background-color: %12;
            border-color: %12;
        }

        /* Radio Buttons */
        QRadioButton {
            spacing: 6px;
        }
        QRadioButton::indicator {
            width: 10px;
            height: 10px;
            border: 1px solid %9;
            border-radius: 5px;
            background-color: %5;
        }
        QRadioButton::indicator:hover {
            border-color: %12;
        }
        QRadioButton::indicator:checked {
            border: 2px solid %12;
            background-color: %12;
        }

        /* Labels */
        QLabel {
            background-color: transparent;
        }

        /* Group Boxes */
        QGroupBox {
            border: 1px solid %9;
            border-radius: 4px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }

        /* Combo Boxes */
        QComboBox {
            background-color: %5;
            border: 1px solid %9;
            border-radius: 4px;
            padding: 4px 8px;
        }
        QComboBox:hover {
            border: 1px solid %12;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox QAbstractItemView {
            background-color: %5;
            border: 1px solid %9;
            selection-background-color: %12;
            selection-color: %7;
        }

        /* Dialogs */
        QDialog {
            background-color: %1;
            border: 2px solid %2;
            border-radius: 8px;
        }
        QMessageBox {
            background-color: %1;
            border: 2px solid %2;
            border-radius: 8px;
        }
        QFileDialog {
            background-color: %1;
            border: 2px solid %2;
            border-radius: 8px;
        }

        /* Tool Tips */
        QToolTip {
            background-color: %10;
            color: %11;
            border: 1px solid %9;
            border-radius: 3px;
            padding: 4px 8px;
        }

        /* Find Bar */
        QWidget#findBarWidget {
            background-color: %3;
            border-top: 1px solid %9;
            padding: 4px;
        }

    )")
        .arg(bg.name())                    // %1 - background
        .arg(text.name())                  // %2 - text
        .arg(window.name())                // %3 - window
        .arg(windowText.name())            // %4 - windowText
        .arg(base.name())                  // %5 - base
        .arg(button.name())                // %6 - button
        .arg(highlightedText.name())       // %7 - highlightedText
        .arg(buttonText.name())            // %8 - buttonText
        .arg(border.name())                // %9 - border
        .arg(m_currentColors.tooltip.name())       // %10 - tooltip
        .arg(m_currentColors.tooltipText.name())   // %11 - tooltipText
        .arg(secondary.name());            // %12 - secondary (accent color)
}

void ThemeManager::applyApplicationStylesheet()
{
    qApp->setStyleSheet(buildStylesheet());
}

void ThemeManager::applyThemeToWidget(QWidget *widget)
{
    if (!widget) return;

    widget->setPalette(QApplication::palette());
    widget->setAutoFillBackground(true);

    widget->style()->unpolish(widget);
    widget->style()->polish(widget);

    const auto children = widget->findChildren<QWidget*>();
    for (QWidget *child : children) {
        child->setPalette(QApplication::palette());
        child->style()->unpolish(child);
        child->style()->polish(child);
    }

    widget->update();
}

QColor ThemeManager::backgroundColor() const
{
    return m_currentColors.background;
}

QColor ThemeManager::textColor() const
{
    return m_currentColors.text;
}

QColor ThemeManager::windowColor() const
{
    return m_currentColors.window;
}

QColor ThemeManager::windowTextColor() const
{
    return m_currentColors.windowText;
}

QColor ThemeManager::highlightColor() const
{
    return m_currentColors.highlight;
}

QColor ThemeManager::highlightedTextColor() const
{
    return m_currentColors.highlightedText;
}

QColor ThemeManager::borderColor() const
{
    return m_currentColors.border;
}

QColor ThemeManager::secondaryColor() const
{
    return m_currentColors.secondary;
}

QColor ThemeManager::baseColor() const
{
    return m_currentColors.base;
}

QIcon ThemeManager::getArrowIcon(bool expanded) const
{
    const int size = 16;
    const QColor arrowColor = m_currentColors.text;

    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(arrowColor);
    painter.setBrush(arrowColor);

    if (expanded) {
        QPolygon arrow;
        arrow << QPoint(4, 6) << QPoint(12, 6) << QPoint(8, 11);
        painter.drawPolygon(arrow);
    } else {
        QPolygon arrow;
        arrow << QPoint(5, 4) << QPoint(5, 12) << QPoint(10, 8);
        painter.drawPolygon(arrow);
    }

    painter.end();
    return QIcon(pixmap);
}

bool ThemeManager::importThemeFromFile(const QString &filePath)
{
    return loadThemeFromJson(filePath);
}

bool ThemeManager::exportThemeToFile(const QString &filePath, Theme theme)
{
    return saveThemeToJson(filePath, theme);
}

bool ThemeManager::saveThemeToJson(const QString &jsonPath, Theme theme)
{
    QString themeId = themeToId(theme);
    
    if (!m_themes.contains(themeId)) {
        qWarning() << "ThemeManager: Theme not found for export:" << themeId;
        return false;
    }
    
    const ThemeColors &colors = m_themes[themeId];

    QJsonObject root;
    root["name"] = colors.name;
    root["id"] = themeId;
    root["version"] = "1.0";
    if (!colors.description.isEmpty()) {
        root["description"] = colors.description;
    }

    QJsonObject colorsObj;
    colorsObj["background"] = colors.background.name();
    colorsObj["text"] = colors.text.name();
    colorsObj["window"] = colors.window.name();
    colorsObj["windowText"] = colors.windowText.name();
    colorsObj["base"] = colors.base.name();
    colorsObj["alternateBase"] = colors.alternateBase.name();
    colorsObj["highlight"] = colors.highlight.name();
    colorsObj["highlightedText"] = colors.highlightedText.name();
    colorsObj["button"] = colors.button.name();
    colorsObj["buttonText"] = colors.buttonText.name();
    colorsObj["border"] = colors.border.name();
    colorsObj["tooltip"] = colors.tooltip.name();
    colorsObj["tooltipText"] = colors.tooltipText.name();
    colorsObj["secondary"] = colors.secondary.name();
    root["colors"] = colorsObj;

    QJsonObject markdownColors;
    markdownColors["heading"] = colors.heading.name();
    markdownColors["bold"] = colors.bold.name();
    markdownColors["italic"] = colors.italic.name();
    markdownColors["strikethrough"] = colors.strikethrough.name();
    markdownColors["codeText"] = colors.codeText.name();
    markdownColors["codeBackground"] = colors.codeBackground.name();
    markdownColors["link"] = colors.link.name();
    markdownColors["image"] = colors.image.name();
    markdownColors["list"] = colors.list.name();
    markdownColors["taskList"] = colors.taskList.name();
    markdownColors["blockquoteText"] = colors.blockquoteText.name();
    markdownColors["blockquoteBackground"] = colors.blockquoteBackground.name();
    markdownColors["tableHeaderText"] = colors.tableHeaderText.name();
    markdownColors["tableCellText"] = colors.tableCellText.name();
    markdownColors["tableHeaderBackground"] = colors.tableHeaderBackground.name();
    markdownColors["tableCellBackground"] = colors.tableCellBackground.name();
    markdownColors["horizontalRule"] = colors.horizontalRule.name();
    markdownColors["syntaxFaint"] = colors.syntaxFaint.name();
    root["markdownColors"] = markdownColors;

    QJsonDocument doc(root);

    QFile file(jsonPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "ThemeManager: Failed to save theme file:" << jsonPath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    return true;
}
