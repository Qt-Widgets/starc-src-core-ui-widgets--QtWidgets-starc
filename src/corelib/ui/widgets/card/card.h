#pragma once

#include <ui/widgets/resizable_widget/resizable_widget.h>


class CORE_LIBRARY_EXPORT Card : public ResizableWidget
{
    Q_OBJECT

public:
    explicit Card(QWidget* _parent = nullptr);
    ~Card() override;

    /**
     * @brief Перекрываем метод, чтобы не возникло путанницы во время разработки
     */
    void setLayout(QLayout* _layout);

    /**
     * @brief Настроить компоновщик содержимого
     */
    QLayout* contentLayout() const;
    void setContentLayout(QLayout* _layout) const;

protected:
    /**
     * @brief Переопределяем для реализации отрисовки
     */
    void paintEvent(QPaintEvent* _event) override;

    /**
     * @brief Переопределяем для реализации эффекта поднятия виджета при ховере
     */
#if (QT_VERSION > QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEnterEvent* _event) override;
#else
    void enterEvent(QEvent* _event) override;
#endif
    void leaveEvent(QEvent* _event) override;

    /**
     * @brief Переопределяем для настройки отступов лейаута
     */
    void designSystemChangeEvent(DesignSystemChangeEvent* _event) override;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};
