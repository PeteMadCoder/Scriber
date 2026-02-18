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

        // Calculate the level of this item
        int level = 0;
        QModelIndex parent = index.parent();
        while (parent.isValid()) {
            level++;
            parent = parent.parent();
        }
        
        // The branch indicator area is to the left of the item rect
        // Position based on level: each level shifts by indentation amount
        int indent = view->indentation();
        int x = 4 + (level * indent);  // Start at 4px from left, plus indentation for each level
        int y = option.rect.center().y();
        int halfArrowSize = 4;

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
