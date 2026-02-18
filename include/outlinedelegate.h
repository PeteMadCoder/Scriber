#pragma once

#include <QStyledItemDelegate>
#include <QColor>

/**
 * @brief Custom delegate for outline tree with theme-aware arrow icons
 */
class OutlineDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit OutlineDelegate(QObject *parent = nullptr);
    
    void setArrowColor(const QColor &color);
    
protected:
    void drawBranchIndicator(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

private:
    QColor m_arrowColor;
};
