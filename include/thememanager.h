#pragma once

#include <QObject>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QSettings>
#include <QWidget>
#include <QMap>
#include <QString>

/**
 * @brief Global theme manager for the application
 *
 * Manages application-wide theming by loading themes from JSON files.
 * All theme data is loaded from resources/themes/ directory.
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

    // Get available theme IDs
    QStringList availableThemeIds() const;
    
    // Get theme name by ID
    QString getThemeName(const QString &themeId) const;

    // Theme color accessors for consistent styling across widgets
    QColor backgroundColor() const;
    QColor textColor() const;
    QColor windowColor() const;
    QColor windowTextColor() const;
    QColor highlightColor() const;
    QColor highlightedTextColor() const;
    QColor borderColor() const;
    QColor secondaryColor() const;
    QColor baseColor() const;

    // Apply theme to a specific widget and all its children
    void applyThemeToWidget(QWidget *widget);

    // Get arrow icon for tree views (color-matched to current theme)
    QIcon getArrowIcon(bool expanded) const;

    // Theme import/export
    bool importThemeFromFile(const QString &filePath);
    bool exportThemeToFile(const QString &filePath, Theme theme);

signals:
    void themeChanged(Theme theme);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager() = default;

    // Prevent copying
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    // Theme loading
    void loadBuiltInThemes();
    bool loadThemeFromJson(const QString &jsonPath);
    bool saveThemeToJson(const QString &jsonPath, Theme theme);
    void loadThemeFromSettings();
    void saveThemeToSettings();
    
    // Theme application
    void applyCurrentTheme();
    void applyGlobalPalette();
    void applyApplicationStylesheet();
    QString buildStylesheet() const;

    static ThemeManager* s_instance;

    Theme m_currentTheme;
    QString m_currentThemeId;
    
    // Theme colors structure
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
        QColor secondary;
        
        // Markdown-specific colors
        QColor heading;
        QColor bold;
        QColor italic;
        QColor strikethrough;
        QColor codeText;
        QColor codeBackground;
        QColor link;
        QColor image;
        QColor list;
        QColor taskList;
        QColor blockquoteText;
        QColor blockquoteBackground;
        QColor tableHeaderText;
        QColor tableCellText;
        QColor tableHeaderBackground;
        QColor tableCellBackground;
        QColor horizontalRule;
        QColor syntaxFaint;
        
        QString name;
        QString description;
    };
    
    // Map of theme ID to theme colors
    QMap<QString, ThemeColors> m_themes;
    ThemeColors m_currentColors;
    
    // Map Theme enum to theme ID
    QString themeToId(Theme theme) const;
};
