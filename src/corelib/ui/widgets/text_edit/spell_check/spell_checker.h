#pragma once

#include <corelib_global.h>

#include <QHash>
#include <QScopedPointer>

class Hunspell;
class QStringList;
class QTextCodec;


/**
 * @brief Класс проверяющего орфографию
 */
class CORE_LIBRARY_EXPORT SpellChecker
{
public:
    /**
     * @brief Синглтон
     */
    static SpellChecker& instance();

public:
    ~SpellChecker();

    /**
     * @brief Установить язык для проверки орфографии
     */
    void setSpellingLanguage(const QString& _languageCode);

    /**
     * @brief Проверить орфографию слова
     * @param Слово для проверки
     * @return Корректность орфографии в слове
     */
    bool spellCheckWord(const QString& _word) const;

    /**
     * @brief Получить список близких слов (вариантов исправления ошибки)
     * @param Некоректное слово, для которого ищется список
     * @return Список близких слов
     */
    QStringList suggestionsForWord(const QString& _word) const;

    /**
     * @brief Игнорировать слово
     * @param Слово, проверку которого необходимо игнорировать
     */
    void ignoreWord(const QString& _word) const;

    /**
     * @brief Добавить слово в словарь
     * @param Слово, которое необходимо добавить в словарь
     */
    void addWordToDictionary(const QString& _word) const;

    /**
     * @brief Получить путь к файлу с пользовательским словарём
     */
    QString userDictionaryPath() const;

private:
    SpellChecker();

    class Implementation;
    QScopedPointer<Implementation> d;
};
