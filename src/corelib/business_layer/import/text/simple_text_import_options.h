#pragma once

#include <QString>

#include <corelib_global.h>


namespace BusinessLayer {

/**
 * @brief Опции импорта
 */
struct CORE_LIBRARY_EXPORT SimpleTextImportOptions {
    /**
     * @brief Путь файла для импорта
     */
    QString filePath;
};

} // namespace BusinessLayer
