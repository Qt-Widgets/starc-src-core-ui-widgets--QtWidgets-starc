#pragma once

#include <corelib_global.h>

#include <QString>


namespace BusinessLayer
{

/**
 * @brief Опции импорта
 */
struct CORE_LIBRARY_EXPORT ImportOptions
{
    /**
     * @brief Путь файла для импорта
     */
    QString filePath;

    /**
     * @brief Нужно ли импортировать персонажей
     */
    bool importCharacters = true;

    /**
     * @brief Нужно ли импортировать локации
     */
    bool importLocations = true;

    /**
     * @brief Нужно ли импортировать сценарий
     */
    bool importScreenplay = true;

    /**
     * @brief Сохранять номера сцен импортируемого сценария
     */
    bool keepSceneNumbers = true;
};

} // namespace BusinessLayer
