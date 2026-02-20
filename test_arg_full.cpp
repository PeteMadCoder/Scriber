#include <QString>
#include <QDebug>
#include <iostream>

int main() {
    QString bg = "#FFFFFF";
    QString text = "#000000";
    QString window = "#FFFFFF";
    QString windowText = "#000000";
    QString base = "#FFFFFF";
    QString button = "#DCDCDC";
    QString highlight = "#0078D7";
    QString highlightedText = "#FFFFFF";
    QString buttonText = "#000000";
    QString border = "#C8C8C8";
    QString tooltip = "#FFFFDC";
    QString tooltipText = "#000000";
    QString secondary = "#0078D7";

    QString result = QString(R"(
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
            background-color: %13;
            color: %8;
        }
        QMenuBar::item:pressed {
            background-color: %13;
            color: %8;
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
            background-color: %13;
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
            background-color: %13;
            color: %8;
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
            border-top: 2px solid %13;
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
            background-color: %13;
        }
        QTreeView::item:selected {
            background-color: %13;
            color: %8;
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
            background-color: %10;
            border-radius: 5px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: %13;
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
            background-color: %13;
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
            background-color: %13;
            color: %8;
            border-color: %13;
        }
        QPushButton:pressed {
            background-color: %13;
            border: 1px solid %13;
        }
        QPushButton:flat {
            border: none;
            background-color: transparent;
        }
        QPushButton:flat:hover {
            background-color: %13;
        }

        /* Line Edits */
        QLineEdit {
            background-color: %5;
            border: 1px solid %10;
            border-radius: 4px;
            padding: 4px 8px;
            selection-background-color: %13;
            selection-color: %8;
        }
        QLineEdit:focus {
            border: 1px solid %13;
        }

        /* Check Boxes */
        QCheckBox {
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid %10;
            border-radius: 4px;
            background-color: %5;
        }
        QCheckBox::indicator:hover {
            border-color: %13;
        }
        QCheckBox::indicator:checked {
            background-color: %13;
            border-color: %13;
        }

        /* Radio Buttons */
        QRadioButton {
            spacing: 6px;
        }
        QRadioButton::indicator {
            width: 10px;
            height: 10px;
            border: 1px solid %10;
            border-radius: 5px;
            background-color: %5;
        }
        QRadioButton::indicator:hover {
            border-color: %13;
        }
        QRadioButton::indicator:checked {
            border: 2px solid %13;
            background-color: %13;
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
            border: 1px solid %13;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox QAbstractItemView {
            background-color: %5;
            border: 1px solid %10;
            selection-background-color: %13;
            selection-color: %8;
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

    )")
        .arg(bg)                    // %1
        .arg(text)                  // %2
        .arg(window)                // %3
        .arg(windowText)            // %4
        .arg(base)                  // %5
        .arg(button)                // %6
        .arg(highlight)             // %7
        .arg(highlightedText)       // %8
        .arg(buttonText)            // %9
        .arg(border)                // %10
        .arg(tooltip)               // %11
        .arg(tooltipText)           // %12
        .arg(secondary);            // %13

    qDebug() << "Success";
    return 0;
}
