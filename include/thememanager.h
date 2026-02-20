#pragma once

#include <QObject>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QSettings>
#include <QWidget>

/**
 * @brief Global theme manager for the application
 * 
 * Manages application-wide theming including:
 * - Global color palette
 * - Application stylesheet
 * - Theme persistence
 * - Theme change notifications
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class Theme {
        Light,
        Dark,
        PitchBlack
    };
    Q_ENUM(Theme)

    static ThemeManager* instance();

    void setTheme(Theme theme);
    Theme currentTheme() const { return m_currentTheme; }

    // Theme color accessors for consistent styling across widgets
    QColor backgroundColor() const;
    QColor textColor() const;
    QColor windowColor() const;
    QColor windowTextColor() const;
    QColor highlightColor() const;
    QColor highlightedTextColor() const;
    QColor borderColor() const;
    QColor secondaryColor() const;

    // Apply theme to a specific widget and all its children
    void applyThemeToWidget(QWidget *widget);

    // Get arrow icon for tree views (color-matched to current theme)
    QIcon getArrowIcon(bool expanded) const;

    // Theme import/export
    bool importThemeFromFile(const QString &filePath);
    bool exportThemeToFile(const QString &filePath, Theme theme);
    bool loadThemeFromJson(const QString &jsonPath);
    bool saveThemeToJson(const QString &jsonPath, Theme theme);

signals:
    void themeChanged(Theme theme);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager() = default;

    // Prevent copying
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void loadThemeColors();
    void loadThemeFromSettings();
    void saveThemeToSettings();
    void applyGlobalPalette();
    void applyApplicationStylesheet();
    QString buildStylesheet() const;

    static ThemeManager* s_instance;

    Theme m_currentTheme;

    // Theme colors
    struct ThemeColors {
        QColor background;
        QColor text;
        QColor window;
        QColor windowText;
        QColor base;
        QColor alternateBase;
        QColor highlight;
        QColor highlightedText;
        QColor button;
        QColor buttonText;
        QColor border;
        QColor tooltip;
        QColor tooltipText;
        QColor secondary;  // Secondary/accent color for links, focus, selection
    };

    ThemeColors m_lightColors;
    ThemeColors m_darkColors;
    ThemeColors m_pitchBlackColors;
    ThemeColors m_currentColors;
};
