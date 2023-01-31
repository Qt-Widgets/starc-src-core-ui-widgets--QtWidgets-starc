#pragma once

#include <QColor>
#include <QMap>

#include <corelib_global.h>

class QAbstractItemModel;

namespace BusinessLayer {

/**
 * @brief Данные графика
 */
struct CORE_LIBRARY_EXPORT PlotData {
    /**
     * @brief Название
     */
    QString name;

    /**
     * @brief Цвет линии
     */
    QColor color;

    /**
     * @brief Цвет заливки
     */
    QColor brushColor;

    /**
     * @brief Координаты
     */
    /** @{ */
    QVector<qreal> x;
    QVector<qreal> y;
    /** @} */
};

/**
 * @brief Дополнительная информация о графике
 */
struct CORE_LIBRARY_EXPORT Plot {
    /**
     * @brief Дополнительная информация о графике
     * @note Используется в качестве подсказки в QCustomPlotExtended
     */
    QMap<qreal, QStringList> info;

    /**
     * @brief Данные о графике
     */
    QVector<PlotData> data;
};

/**
 * @brief Базовый класс для графика
 */
class CORE_LIBRARY_EXPORT AbstractPlot
{
public:
    virtual ~AbstractPlot() = default;

    /**
     * @brief Сформировать график из заданной модели
     */
    virtual void build(QAbstractItemModel* _model) const = 0;

    /**
     * @brief Получить данные графика
     */
    virtual Plot plot() const = 0;

    /**
     * @brief Сохранить график в файл
     */
    virtual void saveToFile(const QString& _fileName) const = 0;
};

} // namespace BusinessLayer
