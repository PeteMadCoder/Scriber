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
    loadThemeColors();
    m_currentColors = m_darkColors;
    applyGlobalPalette();
    applyApplicationStylesheet();
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) {
        return;
    }

    m_currentTheme = theme;

    if (theme == Theme::Dark) {
        m_currentColors = m_darkColors;
    } else if (theme == Theme::PitchBlack) {
        m_currentColors = m_pitchBlackColors;
    } else {
        m_currentColors = m_lightColors;
    }

    applyGlobalPalette();
    applyApplicationStylesheet();

    // Notify all listeners
    emit themeChanged(theme);

    // Re-apply to all top-level widgets
    const auto topWidgets = QApplication::topLevelWidgets();
    for (QWidget *widget : topWidgets) {
        applyThemeToWidget(widget);
    }
}

void ThemeManager::loadThemeColors()
{
    // Light Theme Colors
    m_lightColors.background = QColor(245, 245, 245);
    m_lightColors.text = QColor(36, 41, 47);
    m_lightColors.window = QColor(240, 240, 240);
    m_lightColors.windowText = QColor(36, 41, 47);
    m_lightColors.base = QColor(255, 255, 255);
    m_lightColors.alternateBase = QColor(235, 235, 235);
    m_lightColors.highlight = QColor(0, 120, 215);
    m_lightColors.highlightedText = QColor(255, 255, 255);
    m_lightColors.button = QColor(220, 220, 220);
    m_lightColors.buttonText = QColor(36, 41, 47);
    m_lightColors.border = QColor(200, 200, 200);
    m_lightColors.tooltip = QColor(255, 255, 220);
    m_lightColors.tooltipText = QColor(36, 41, 47);

    // Dark Theme Colors
    m_darkColors.background = QColor(13, 17, 23);
    m_darkColors.text = QColor(225, 228, 232);
    m_darkColors.window = QColor(30, 35, 45);
    m_darkColors.windowText = QColor(225, 228, 232);
    m_darkColors.base = QColor(25, 30, 40);
    m_darkColors.alternateBase = QColor(35, 40, 50);
    m_darkColors.highlight = QColor(88, 166, 255);
    m_darkColors.highlightedText = QColor(255, 255, 255);
    m_darkColors.button = QColor(45, 50, 60);
    m_darkColors.buttonText = QColor(225, 228, 232);
    m_darkColors.border = QColor(60, 65, 75);
    m_darkColors.tooltip = QColor(40, 45, 55);
    m_darkColors.tooltipText = QColor(225, 228, 232);

    // Pitch Black Theme Colors
    m_pitchBlackColors.background = QColor(0, 0, 0);
    m_pitchBlackColors.text = QColor(224, 224, 224);
    m_pitchBlackColors.window = QColor(10, 10, 10);
    m_pitchBlackColors.windowText = QColor(224, 224, 224);
    m_pitchBlackColors.base = QColor(15, 15, 15);
    m_pitchBlackColors.alternateBase = QColor(25, 25, 25);
    m_pitchBlackColors.highlight = QColor(100, 180, 255);
    m_pitchBlackColors.highlightedText = QColor(255, 255, 255);
    m_pitchBlackColors.button = QColor(30, 30, 30);
    m_pitchBlackColors.buttonText = QColor(224, 224, 224);
    m_pitchBlackColors.border = QColor(80, 80, 80);
    m_pitchBlackColors.tooltip = QColor(20, 20, 20);
    m_pitchBlackColors.tooltipText = QColor(224, 224, 224);
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
            border-bottom: 1px solid %10;
        }
        QMenuBar::item:selected {
            background-color: %7;
        }
        QMenuBar::item:pressed {
            background-color: %7;
        }

        /* Menus */
        QMenu {
            background-color: %3;
            border: 1px solid %10;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 20px 6px 20px;
        }
        QMenu::item:selected {
            background-color: %7;
            color: %8;
        }
        QMenu::separator {
            height: 1px;
            background: %10;
            margin: 4px 10px 4px 10px;
        }

        /* Tool Bar */
        QToolBar {
            background-color: %3;
            border-bottom: 1px solid %10;
            padding: 2px;
            spacing: 4px;
        }
        QToolBar::separator {
            background-color: %10;
            width: 1px;
            margin: 4px;
        }

        /* Status Bar */
        QStatusBar {
            background-color: %3;
            border-top: 1px solid %10;
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
            border-bottom: 1px solid %10;
        }
        QDockWidget::close-button,
        QDockWidget::float-button {
            border: none;
            padding: 4px;
        }
        QDockWidget::close-button:hover,
        QDockWidget::float-button:hover {
            background-color: %7;
        }

        /* Tab Widgets */
        QTabWidget::pane {
            border: 1px solid %10;
            background-color: %1;
        }
        QTabBar::tab {
            background-color: %3;
            color: %4;
            padding: 6px 12px;
            border: 1px solid %10;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: %1;
            border-top: 2px solid %7;
        }
        QTabBar::tab:hover:!selected {
            background-color: %5;
        }

        /* Tree Views (File Explorer, Outline) */
        QTreeView {
            background-color: %5;
            border: none;
            outline: none;
        }
        QTreeView::item {
            padding: 4px;
        }
        QTreeView::item:hover {
            background-color: %7;
        }
        QTreeView::item:selected {
            background-color: %7;
            color: %8;
        }
        QTreeView::branch {
            background-color: %5;
        }

        /* Scroll Bars */
        QScrollBar:vertical {
            background-color: %3;
            width: 12px;
            border-radius: 6px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background-color: %10;
            border-radius: 5px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %7;
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
            background-color: %10;
            border-radius: 5px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: %7;
        }
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0;
        }

        /* Buttons */
        QPushButton {
            background-color: %6;
            color: %9;
            border: 1px solid %10;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: %7;
            color: %8;
        }
        QPushButton:pressed {
            background-color: %7;
            border: 1px solid %7;
        }
        QPushButton:flat {
            border: none;
            background-color: transparent;
        }
        QPushButton:flat:hover {
            background-color: %7;
        }

        /* Line Edits */
        QLineEdit {
            background-color: %5;
            border: 1px solid %10;
            border-radius: 4px;
            padding: 4px 8px;
            selection-background-color: %7;
            selection-color: %8;
        }
        QLineEdit:focus {
            border: 1px solid %7;
        }

        /* Check Boxes */
        QCheckBox {
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid %10;
            border-radius: 3px;
            background-color: %5;
        }
        QCheckBox::indicator:checked {
            background-color: %7;
            border-color: %7;
        }

        /* Labels */
        QLabel {
            background-color: transparent;
        }

        /* Group Boxes */
        QGroupBox {
            border: 1px solid %10;
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
            border: 1px solid %10;
            border-radius: 4px;
            padding: 4px 8px;
        }
        QComboBox:hover {
            border: 1px solid %7;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox QAbstractItemView {
            background-color: %5;
            border: 1px solid %10;
            selection-background-color: %7;
            selection-color: %8;
        }

        /* Dialogs */
        QDialog {
            background-color: %1;
        }
        QMessageBox {
            background-color: %1;
        }
        QFileDialog {
            background-color: %1;
        }

        /* Tool Tips */
        QToolTip {
            background-color: %11;
            color: %12;
            border: 1px solid %10;
            border-radius: 3px;
            padding: 4px 8px;
        }

        /* Find Bar */
        QWidget#findBarWidget {
            background-color: %3;
            border-top: 1px solid %10;
            padding: 4px;
        }

    )").arg(
        bg.name(),           // %1 - background
        text.name(),         // %2 - text
        window.name(),       // %3 - window
        windowText.name(),   // %4 - windowText
        base.name(),         // %5 - base
        button.name(),       // %6 - button
        highlight.name(),    // %7 - highlight
        highlightedText.name(), // %8 - highlightedText
        buttonText.name(),   // %9 - buttonText
        border.name(),       // %10 - border
        m_currentColors.tooltip.name(),    // %11 - tooltip
        m_currentColors.tooltipText.name() // %12 - tooltipText
    );
}

void ThemeManager::applyApplicationStylesheet()
{
    qApp->setStyleSheet(buildStylesheet());
}

void ThemeManager::applyThemeToWidget(QWidget *widget)
{
    if (!widget) return;

    // Apply palette
    widget->setPalette(QApplication::palette());
    widget->setAutoFillBackground(true);

    // Force style update
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);

    // Update for all child widgets
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
