#include "documentoutlinewidget.h"
#include "editorwidget.h"
#include "outlinedelegate.h"
#include "thememanager.h"
#include <QTreeWidgetItem>
#include <QKeyEvent>
#include <cmark.h>

DocumentOutlineWidget::DocumentOutlineWidget(QWidget *parent)
    : QTreeWidget(parent)
    , currentEditor(nullptr)
    , updateTimer(nullptr)
{
    setHeaderHidden(true);
    setColumnCount(1);
    
    // Set up custom delegate for theme-aware arrow icons
    outlineDelegate = new OutlineDelegate(this);
    setItemDelegate(outlineDelegate);
    
    // Connect to theme changes to update arrow colors
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        outlineDelegate->setArrowColor(ThemeManager::instance()->textColor());
        viewport()->update();
    });
    
    // Initialize arrow color
    outlineDelegate->setArrowColor(ThemeManager::instance()->textColor());
    
    // Setup update timer
    updateTimer = new QTimer(this);
    updateTimer->setSingleShot(true);
    updateTimer->setInterval(1000);
    connect(updateTimer, &QTimer::timeout, this, &DocumentOutlineWidget::updateOutline);
    
    connect(this, &QTreeWidget::itemClicked, this, &DocumentOutlineWidget::onItemClicked);
}

DocumentOutlineWidget::~DocumentOutlineWidget()
{
}

void DocumentOutlineWidget::setEditor(EditorWidget *editor)
{
    currentEditor = editor;
}

void DocumentOutlineWidget::updateOutline()
{
    if (!currentEditor) return;
    
    clear();
    
    QString markdown = currentEditor->toPlainText();
    QByteArray utf8 = markdown.toUtf8();
    
    cmark_node *doc = cmark_parse_document(utf8.constData(), utf8.size(), CMARK_OPT_DEFAULT);
    if (!doc) return;
    
    cmark_iter *iter = cmark_iter_new(doc);
    cmark_event_type ev_type;
    
    QList<QTreeWidgetItem*> parents;
    parents.append(nullptr);
    
    while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
        cmark_node *cur = cmark_iter_get_node(iter);
        
        if (ev_type == CMARK_EVENT_ENTER && cmark_node_get_type(cur) == CMARK_NODE_HEADING) {
            int level = cmark_node_get_heading_level(cur);
            int startLine = cmark_node_get_start_line(cur);
            
            QString headingText;
            cmark_iter *subIter = cmark_iter_new(cur);
            while (cmark_iter_next(subIter) != CMARK_EVENT_DONE) {
                cmark_node *subNode = cmark_iter_get_node(subIter);
                if (cmark_node_get_type(subNode) == CMARK_NODE_TEXT ||
                    cmark_node_get_type(subNode) == CMARK_NODE_CODE) {
                    const char *text = cmark_node_get_literal(subNode);
                    if (text) headingText += QString::fromUtf8(text);
                }
            }
            cmark_iter_free(subIter);
            
            if (headingText.isEmpty())
                headingText = tr("(Empty Heading)");
            
            QTreeWidgetItem *item = new QTreeWidgetItem();
            item->setText(0, headingText);
            item->setData(0, Qt::UserRole, startLine);
            
            while (parents.size() <= level) parents.append(nullptr);
            while (parents.size() > level + 1) parents.removeLast();
            
            QTreeWidgetItem *parentItem = nullptr;
            if (level > 0 && level < parents.size()) {
                parentItem = parents[level - 1];
            }
            
            if (parentItem) {
                parentItem->addChild(item);
            } else {
                addTopLevelItem(item);
            }
            
            if (parents.size() > level) {
                parents[level] = item;
            } else {
                parents.append(item);
            }
            
            item->setExpanded(true);
        }
    }
    
    cmark_iter_free(iter);
    cmark_node_free(doc);
}

void DocumentOutlineWidget::onItemClicked(QTreeWidgetItem *item, int column)
{
    if (!currentEditor) return;
    
    int line = item->data(0, Qt::UserRole).toInt();
    if (line > 0) {
        QTextCursor cursor = currentEditor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
        currentEditor->setTextCursor(cursor);
        currentEditor->centerCursor();
        currentEditor->setFocus();
    }
}

void DocumentOutlineWidget::keyPressEvent(QKeyEvent *event)
{
    // Prevent escape from being handled here - let parent handle it
    if (event->key() == Qt::Key_Escape) {
        event->ignore();
        return;
    }
    QTreeWidget::keyPressEvent(event);
}
