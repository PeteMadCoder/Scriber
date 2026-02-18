#include "toastnotification.h"
#include <QResizeEvent>

ToastNotification::ToastNotification(QWidget *parent)
    : QLabel(parent)
{
    setStyleSheet(
        "QLabel { "
        "  background-color: #333333; "
        "  color: #ffffff; "
        "  padding: 8px 16px; "
        "  border-radius: 4px; "
        "  font-weight: bold;"
        "}"
    );
    hide();
    setAttribute(Qt::WA_TransparentForMouseEvents);
    
    // Setup opacity effect for fade animation
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);
    
    // Setup fade in animation
    fadeInAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeInAnimation->setDuration(300);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    
    // Setup hide timer
    hideTimer = new QTimer(this);
    hideTimer->setSingleShot(true);
    connect(hideTimer, &QTimer::timeout, this, &ToastNotification::hideToast);
}

void ToastNotification::showMessage(const QString &message, int durationMs)
{
    setText(message);
    adjustSize();
    positionToast();
    
    show();
    raise();
    
    // Reset and start fade in animation
    fadeInAnimation->stop();
    opacityEffect->setOpacity(0.0);
    fadeInAnimation->start();
    
    // Auto-hide after duration
    hideTimer->setInterval(durationMs);
    hideTimer->start();
}

void ToastNotification::hideToast()
{
    hide();
}

void ToastNotification::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    positionToast();
}

void ToastNotification::positionToast()
{
    if (!parentWidget()) return;
    
    int x = (parentWidget()->width() - width()) / 2;
    int y = parentWidget()->height() - height() - 50;
    move(x, y);
}
