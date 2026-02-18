#include "themedialog.h"
#include "thememanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QScrollArea>

ThemeDialog::ThemeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Select Theme"));
    setMinimumSize(450, 350);
    setupUI();
}

void ThemeDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Description label with proper word wrapping
    auto *descLabel = new QLabel(tr(
        "Choose a theme for the application. The theme affects all interface "
        "elements including menus, toolbars, dialogs, and the editor.\n\n"
        "• Light – Bright theme for well-lit environments\n"
        "• Dark – Comfortable theme for general use\n"
        "• Pitch Black – High contrast, OLED-friendly theme"
    ));
    descLabel->setWordWrap(true);
    descLabel->setOpenExternalLinks(false);
    descLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    mainLayout->addWidget(descLabel);

    // Scroll area for theme options (allows adding more themes in the future)
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    auto *scrollContent = new QWidget();
    auto *themeLayout = new QVBoxLayout(scrollContent);
    themeLayout->setSpacing(10);
    themeLayout->setContentsMargins(0, 0, 0, 0);

    themeButtonGroup = new QButtonGroup(this);

    ThemeManager *manager = ThemeManager::instance();
    ThemeManager::Theme currentTheme = manager->currentTheme();

    // Light theme option
    auto *lightRadio = new QRadioButton(tr("Light"));
    lightRadio->setChecked(currentTheme == ThemeManager::Theme::Light);
    themeButtonGroup->addButton(lightRadio, static_cast<int>(ThemeManager::Theme::Light));
    
    auto *lightDesc = new QLabel(tr(
        "A bright, clean theme with white backgrounds and dark text. "
        "Best suited for well-lit environments and users who prefer traditional light interfaces."
    ));
    lightDesc->setWordWrap(true);
    lightDesc->setContentsMargins(24, 0, 0, 8);
    
    themeLayout->addWidget(lightRadio);
    themeLayout->addWidget(lightDesc);

    // Dark theme option
    auto *darkRadio = new QRadioButton(tr("Dark"));
    darkRadio->setChecked(currentTheme == ThemeManager::Theme::Dark);
    themeButtonGroup->addButton(darkRadio, static_cast<int>(ThemeManager::Theme::Dark));
    
    auto *darkDesc = new QLabel(tr(
        "A comfortable dark gray theme that reduces eye strain. "
        "Ideal for extended writing sessions and low-light environments."
    ));
    darkDesc->setWordWrap(true);
    darkDesc->setContentsMargins(24, 0, 0, 8);
    
    themeLayout->addWidget(darkRadio);
    themeLayout->addWidget(darkDesc);

    // Pitch Black theme option
    auto *pitchBlackRadio = new QRadioButton(tr("Pitch Black"));
    pitchBlackRadio->setChecked(currentTheme == ThemeManager::Theme::PitchBlack);
    themeButtonGroup->addButton(pitchBlackRadio, static_cast<int>(ThemeManager::Theme::PitchBlack));
    
    auto *pitchBlackDesc = new QLabel(tr(
        "A pure black theme with high contrast text. "
        "Perfect for OLED displays and users who prefer maximum contrast with minimal light emission."
    ));
    pitchBlackDesc->setWordWrap(true);
    pitchBlackDesc->setContentsMargins(24, 0, 0, 0);
    
    themeLayout->addWidget(pitchBlackRadio);
    themeLayout->addWidget(pitchBlackDesc);

    themeLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    // Buttons
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    applyButton = new QPushButton(tr("Apply"));
    applyButton->setMinimumWidth(80);
    connect(applyButton, &QPushButton::clicked, this, &ThemeDialog::onApplyClicked);
    
    cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setMinimumWidth(80);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton);
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
