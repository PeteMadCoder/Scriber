#pragma once

#include <QTreeWidget>
#include <QTimer>

class EditorWidget;
class OutlineDelegate;
class ThemeManager;

/**
 * @brief Document outline widget showing heading structure
 *
 * Displays a tree view of document headings (H1-H6) parsed from Markdown
 * using cmark. Allows navigation by clicking on headings.
 */
class DocumentOutlineWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit DocumentOutlineWidget(QWidget *parent = nullptr);
    ~DocumentOutlineWidget();
    
    /// Set the editor to parse for outline
    void setEditor(EditorWidget *editor);
    
    /// Update the outline tree from current editor content
    void updateOutline();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);

private:
    void parseHeadings(const QString &markdown);
    
    EditorWidget *currentEditor;
    OutlineDelegate *outlineDelegate;
    QTimer *updateTimer;
};
