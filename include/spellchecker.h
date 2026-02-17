#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>

class Hunspell;

class SpellChecker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a SpellChecker object.
     * @param parent The parent QObject.
     */
    explicit SpellChecker(QObject *parent = nullptr);

    /**
     * @brief Destroys the SpellChecker object and cleans up Hunspell resources.
     */
    ~SpellChecker();

    /**
     * @brief Loads a dictionary for a specific language.
     * @param language The language code (e.g., "en_US", "fr_FR").
     * @return true if the dictionary was loaded successfully, false otherwise.
     */
    bool loadDictionary(const QString &language);

    /**
     * @brief Checks if a word is misspelled.
     * @param word The word to check.
     * @return true if the word is misspelled, false otherwise.
     * @note Returns false if the spell checker is not initialized or the word is empty.
     */
    bool isWordMisspelled(const QString &word) const;

    /**
     * @brief Gets spelling suggestions for a misspelled word.
     * @param word The misspelled word.
     * @return A list of suggested corrections.
     */
    QStringList getSuggestions(const QString &word) const;

    /**
     * @brief Adds a word to the personal dictionary (runtime only).
     * @param word The word to add.
     */
    void addWord(const QString &word);

    /**
     * @brief Checks if the spell checker is properly initialized.
     * @return true if initialized, false otherwise.
     */
    bool isInitialized() const;

private:
    std::unique_ptr<Hunspell> hunspell; ///< Pointer to the Hunspell instance.
};