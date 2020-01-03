#pragma once

#include <ui/widgets/app_bar/app_bar.h>


namespace Ui
{

/**
 * @brief Панель инструментов проекта
 */
class ProjectToolBar : public AppBar
{
    Q_OBJECT
public:
    explicit ProjectToolBar(QWidget* _parent = nullptr);

    /**
     * @brief Очистить представления
     */
    void clearViews();

    /**
     * @brief Добавить представление
     */
    void addView(const QString& _mimeType, const QString& _icon, bool _isActive = false);

signals:
    /**
     * @brief Пользователь хочет открыть меню
     */
    void menuPressed();

    /**
     * @brief Пользователь хочет открыть выбранное представление
     */
    void viewPressed(const QString& _mimeType);

protected:
    /**
     * @brief Обновляем виджет при изменении дизайн системы
     */
    void designSystemChangeEvent(DesignSystemChangeEvent* _event) override;
};

} // namespace Ui
