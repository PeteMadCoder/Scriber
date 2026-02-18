#pragma once

#include <QDialog>
#include <QButtonGroup>
#include <QPushButton>
#include "thememanager.h"

/**
 * @brief Dialog for selecting application theme
 */
class ThemeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ThemeDialog(QWidget *parent = nullptr);

private slots:
    void onThemeSelected();
    void onApplyClicked();

private:
    void setupUI();

    QButtonGroup *themeButtonGroup;
    QPushButton *applyButton;
    QPushButton *cancelButton;
};
