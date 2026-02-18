#include "outlinedelegate.h"
#include <QPainter>
#include <QStyleOption>
#include <QTreeView>

OutlineDelegate::OutlineDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_arrowColor(Qt::black)
{
}

void OutlineDelegate::setArrowColor(const QColor &color)
{
    m_arrowColor = color;
}

void OutlineDelegate::drawBranchIndicator(QPainter *painter, const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    const QTreeView *view = qobject_cast<const QTreeView*>(option.widget);
    if (!view) return;

    // Check if item has children
    if (index.model()->hasChildren(index)) {
        bool expanded = view->isExpanded(index);

        // Get the indentation to position the arrow correctly
        int indent = view->indentation();
        int level = 0;
        QModelIndex parent = index.parent();
        while (parent.isValid()) {
            level++;
            parent = parent.parent();
        }
        
        // Position: start of item rect minus indentation for branch area, plus small offset
        int x = option.rect.left() - (indent * level) - indent + 6;
        int y = option.rect.center().y();
        int halfArrowSize = 4;

        // Ensure x is at a reasonable position
        if (x < 2) x = 6;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(m_arrowColor);
        painter->setBrush(m_arrowColor);

        if (expanded) {
            // Down-pointing arrow (expanded state)
            QPolygon arrow;
            arrow << QPoint(x, y - halfArrowSize)
                  << QPoint(x + halfArrowSize * 2, y - halfArrowSize)
                  << QPoint(x + halfArrowSize, y + halfArrowSize);
            painter->drawPolygon(arrow);
        } else {
            // Right-pointing arrow (collapsed state)
            QPolygon arrow;
            arrow << QPoint(x, y - halfArrowSize)
                  << QPoint(x, y + halfArrowSize)
                  << QPoint(x + halfArrowSize * 2, y);
            painter->drawPolygon(arrow);
        }

        painter->restore();
    }
}

void OutlineDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    // Draw the standard item (text, background, etc.)
    QStyledItemDelegate::paint(painter, option, index);

    // Draw custom branch indicator
    drawBranchIndicator(painter, option, index);
}
