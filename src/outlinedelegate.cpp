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
        
        // Calculate arrow position
        int x = option.rect.left() + 2;
        int y = option.rect.center().y();
        int arrowSize = 5;
        
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(m_arrowColor);
        painter->setBrush(m_arrowColor);
        
        if (expanded) {
            // Down-pointing arrow
            QPolygon arrow;
            arrow << QPoint(x, y - 2)
                  << QPoint(x + arrowSize * 2, y - 2)
                  << QPoint(x + arrowSize, y + 3);
            painter->drawPolygon(arrow);
        } else {
            // Right-pointing arrow
            QPolygon arrow;
            arrow << QPoint(x, y - 3)
                  << QPoint(x, y + 3)
                  << QPoint(x + arrowSize + 2, y);
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
