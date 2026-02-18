#include "themedialog.h"
#include "thememanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

ThemeDialog::ThemeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Select Theme"));
    setFixedSize(350, 250);
    setupUI();
}

void ThemeDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Description label
    auto *descLabel = new QLabel(tr("Choose a theme for the application:\n\n"
                                    "• Light - Bright theme for well-lit environments\n"
                                    "• Dark - Comfortable theme for general use\n"
                                    "• Pitch Black - High contrast, OLED-friendly theme"));
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);

    // Theme selection group
    auto *themeGroup = new QGroupBox(tr("Theme"));
    auto *themeLayout = new QVBoxLayout(themeGroup);
    themeLayout->setSpacing(8);

    themeButtonGroup = new QButtonGroup(this);

    ThemeManager *manager = ThemeManager::instance();
    ThemeManager::Theme currentTheme = manager->currentTheme();

    // Light theme option
    auto *lightRadio = new QRadioButton(tr("Light"));
    lightRadio->setChecked(currentTheme == ThemeManager::Theme::Light);
    themeButtonGroup->addButton(lightRadio, static_cast<int>(ThemeManager::Theme::Light));
    themeLayout->addWidget(lightRadio);

    // Dark theme option
    auto *darkRadio = new QRadioButton(tr("Dark"));
    darkRadio->setChecked(currentTheme == ThemeManager::Theme::Dark);
    themeButtonGroup->addButton(darkRadio, static_cast<int>(ThemeManager::Theme::Dark));
    themeLayout->addWidget(darkRadio);

    // Pitch Black theme option
    auto *pitchBlackRadio = new QRadioButton(tr("Pitch Black"));
    pitchBlackRadio->setChecked(currentTheme == ThemeManager::Theme::PitchBlack);
    themeButtonGroup->addButton(pitchBlackRadio, static_cast<int>(ThemeManager::Theme::PitchBlack));
    themeLayout->addWidget(pitchBlackRadio);

    mainLayout->addWidget(themeGroup);

    // Buttons
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &ThemeDialog::onApplyClicked);
    buttonLayout->addWidget(applyButton);

    cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connect radio buttons to preview theme
    connect(themeButtonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, &ThemeDialog::onThemeSelected);
}

void ThemeDialog::onThemeSelected()
{
    // Preview the selected theme
    QAbstractButton *button = themeButtonGroup->checkedButton();
    if (!button) return;
    
    int themeId = themeButtonGroup->id(button);
    ThemeManager::Theme theme = static_cast<ThemeManager::Theme>(themeId);
    ThemeManager::instance()->setTheme(theme);
}

void ThemeDialog::onApplyClicked()
{
    accept();
}
