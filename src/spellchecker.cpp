// spellchecker.cpp
#include "spellchecker.h"
#include <hunspell/hunspell.hxx>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

SpellChecker::SpellChecker(QObject *parent)
    : QObject(parent), hunspell(nullptr) // , codec(nullptr) // <-- REMOVE INITIALIZATION
{
    // Initialization happens in loadDictionary
}

SpellChecker::~SpellChecker()
{
    // std::unique_ptr<Hunspell> will automatically call the destructor
}

bool SpellChecker::loadDictionary(const QString &language)
{
    // --- Find Dictionary Files ---
    QString dictPath;
    QStringList dictSearchPaths;

    dictSearchPaths << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    dictSearchPaths << "/usr/share/hunspell"
                    << "/usr/local/share/hunspell";

    for (const QString &searchPath : dictSearchPaths) {
        QString candidatePath = QDir(searchPath).absoluteFilePath("hunspell");
        if (QDir(candidatePath).exists()) {
            dictPath = candidatePath;
            break;
        }
    }

    if (dictPath.isEmpty()) {
        qWarning() << "SpellChecker: Could not find hunspell dictionary directory.";
        return false;
    }

    QString affPath = QDir(dictPath).absoluteFilePath(language + ".aff");
    QString dicPath = QDir(dictPath).absoluteFilePath(language + ".dic");

    if (!QFile::exists(affPath) || !QFile::exists(dicPath)) {
        qWarning() << "SpellChecker: Dictionary files not found for language:" << language
                   << "Aff Path:" << affPath << "Dic Path:" << dicPath;
        return false;
    }

    try {
        // Create a new Hunspell instance.
        hunspell.reset(new Hunspell(affPath.toLocal8Bit().constData(),
                                    dicPath.toLocal8Bit().constData()));

        // --- Get Encoding (Informational, but we'll assume UTF-8 for simplicity) ---
        // const char* encoding_cstr = hunspell->get_dic_encoding();
        // QString encoding = QString::fromLatin1(encoding_cstr);
        // qDebug() << "SpellChecker: Dictionary" << language << "uses encoding:" << encoding;
        // Most modern dictionaries are UTF-8. Hunspell internally handles conversion
        // reasonably well if we pass UTF-8 encoded strings from QString.

        qDebug() << "SpellChecker: Successfully loaded dictionary for" << language;
        return true;

    } catch (const std::exception& e) {
        qWarning() << "SpellChecker: Exception caught while initializing Hunspell for" << language << ":" << e.what();
        hunspell.reset();
        return false;
    } catch (...) {
        qWarning() << "SpellChecker: Unknown exception caught while initializing Hunspell for" << language;
        hunspell.reset();
        return false;
    }
}

bool SpellChecker::isWordMisspelled(const QString &word) const
{
    if (!hunspell || word.isEmpty()) {
        return false;
    }

    // Convert QString (UTF-16) to UTF-8 for Hunspell
    QByteArray utf8Word = word.toUtf8(); // <-- USE toUtf8()

    try {
        // Hunspell::spell returns 0 if the word is NOT found (misspelled)
        int result = hunspell->spell(utf8Word.constData()); // <-- PASS UTF-8 DATA
        return (result == 0);
    } catch (const std::exception& e) {
        qWarning() << "SpellChecker: Exception in isWordMisspelled for word:" << word << e.what();
        return false;
    } catch (...) {
        qWarning() << "SpellChecker: Unknown exception in isWordMisspelled for word:" << word;
        return false;
    }
}

QStringList SpellChecker::getSuggestions(const QString &word) const
{
    QStringList suggestions;
    if (!hunspell) {
        return suggestions;
    }

    // Convert QString (UTF-16) to UTF-8 for Hunspell
    QByteArray utf8Word = word.toUtf8(); // <-- USE toUtf8()

    try {
        char **sug;
        int sugCount = hunspell->suggest(&sug, utf8Word.constData()); // <-- PASS UTF-8 DATA

        for (int i = 0; i < sugCount; ++i) {
            // Convert Hunspell's C-string (assumed UTF-8) back to QString
            QString suggestion = QString::fromUtf8(sug[i]); // <-- USE fromUtf8()
            suggestions << suggestion;
        }

        hunspell->free_list(&sug, sugCount);

    } catch (const std::exception& e) {
        qWarning() << "SpellChecker: Exception in getSuggestions for word:" << word << e.what();
    } catch (...) {
        qWarning() << "SpellChecker: Unknown exception in getSuggestions for word:" << word;
    }

    return suggestions;
}

void SpellChecker::addWord(const QString &word)
{
    if (!hunspell) {
        return;
    }

    // Convert QString (UTF-16) to UTF-8 for Hunspell
    QByteArray utf8Word = word.toUtf8(); // <-- USE toUtf8()

    try {
        hunspell->add(utf8Word.constData()); // <-- PASS UTF-8 DATA
        qDebug() << "SpellChecker: Added word to dictionary:" << word;
    } catch (const std::exception& e) {
        qWarning() << "SpellChecker: Exception in addWord for word:" << word << e.what();
    } catch (...) {
        qWarning() << "SpellChecker: Unknown exception in addWord for word:" << word;
    }
}

bool SpellChecker::isInitialized() const
{
    return static_cast<bool>(hunspell);
}
