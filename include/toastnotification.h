#pragma once

#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

/**
 * @brief Toast notification widget for displaying temporary messages
 *
 * Displays a fade-in notification at the bottom center of the parent widget
 * that automatically hides after a specified duration.
 */
class ToastNotification : public QLabel
{
    Q_OBJECT

public:
    explicit ToastNotification(QWidget *parent = nullptr);
    
    /// Show a toast message with optional duration
    void showMessage(const QString &message, int durationMs = 3000);
    
    /// Hide the toast immediately
    void hideToast();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void positionToast();
    
    QTimer *hideTimer;
    QPropertyAnimation *fadeInAnimation;
    QGraphicsOpacityEffect *opacityEffect;
};
