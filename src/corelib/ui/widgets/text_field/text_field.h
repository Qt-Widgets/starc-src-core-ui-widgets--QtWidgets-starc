#pragma once

#include <ui/widgets/text_edit/base/base_text_edit.h>

class QValidator;


/**
 * @brief Виджет текстового поля
 */
class CORE_LIBRARY_EXPORT TextField : public BaseTextEdit
{
    Q_OBJECT

public:
    explicit TextField(QWidget* _parent = nullptr);
    ~TextField() override;

    /**
     * @brief Цвет фона виджета
     */
    void setBackgroundColor(const QColor& _color);

    /**
     * @brief Цвет текста виджета
     */
    void setTextColor(const QColor& _color);
    QColor textColor() const;

    /**
     * @brief Установить тект поясняющей метки
     */
    void setLabel(const QString& _text);

    /**
     * @brief Установить текст впомогательной подсказки
     */
    QString helper() const;
    void setHelper(const QString& _text);

    /**
     * @brief Текст ошибки
     */
    QString error() const;
    void setError(const QString& _text);
    void clearError();

    /**
     * @brief Перекрываем собственной реализацией
     */
    void setPlaceholderText(const QString& _placeholder);
    QString placeholderText() const;

    /**
     * @brief Задать суффикс для поля ввода
     */
    void setSuffix(const QString& _suffix);

    /**
     * @brief Получить текст поля ввода
     */
    QString text() const;

    /**
     * @brief Перекрываем собственной реализацией
     */
    void setText(const QString& _text);

    /**
     * @brief Выделенный текст
     */
    QString selectedText() const;

    /**
     * @brief Задать иконку действия в редакторе
     */
    void setTrailingIcon(const QString& _icon);

    /**
     * @brief Задать цвет иконки действия (по-умолчанию используется цвет текста)
     */
    void setTrailingIconColor(const QColor& _color);
    QColor trailingIconColor() const;

    /**
     * @brief Задать всплывающую подсказку для иконки действия
     */
    void setTrailingIconToolTip(const QString& _toolTip);

    /**
     * @brief Включить/отключить режим ввода пароля
     */
    void setPasswordModeEnabled(bool _enable);
    bool isPasswordModeEnabled() const;

    /**
     * @brief Установить видимость декоративной полосы под текстом
     */
    void setUnderlineDecorationVisible(bool _visible);

    /**
     * @brief Установить видимость заголовка
     */
    void setLabelVisible(bool _visible);

    /**
     * @brief Установить использование отступов по-умолчанию
     */
    bool isDefaultMarginsEnabled() const;
    void setDefaultMarginsEnabled(bool _enable);
    QMarginsF customMargins() const;
    void setCustomMargins(const QMarginsF& _margins);

    /**
     * @brief Установить необходимость добавления новых строк при нажатии Enter'а
     */
    void setEnterMakesNewLine(bool _make);

    /**
     * @brief Перекрываем реализацию очистки своей, чтобы не ломался документ
     */
    void clear();

    /**
     * @brief Вычисляем идеальный размер в зависимости от контента
     */
    /** @{ */
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    int heightForWidth(int _width) const override;
    /** @} */

    /**
     * @brief Переопределяем, чтобы отлавливать событие контекстного меню вспомогательного действия
     */
    ContextMenu* createContextMenu(const QPoint& _position, QWidget* _parent = nullptr) override;

signals:
    /**
     * @brief Был нажат энтер
     * @note Сигнал срабатывает только в случае, когда @b setEnterMakesNewLine задан в @b true
     */
    void enterPressed();

    /**
     * @brief Была нажата иконка вспомогательного действия
     */
    void trailingIconPressed();

    /**
     * @brief Запрос на отображение контекстного меню вспомогательного действия
     */
    void trailingIconContextMenuRequested();

protected:
    /**
     * @brief Сконфигурировать виджет при изменении какого-либо из параметров внешнего вида
     */
    virtual void reconfigure();

    /**
     * @brief Перенастраиваем виджет при обновлении дизайн системы
     */
    bool event(QEvent* _event) override;

    /**
     * @brief Собственная реализация рисования
     */
    void paintEvent(QPaintEvent* _event) override;

    /**
     * @brief Корректируем размер полосы декорации, при необходимости
     */
    void resizeEvent(QResizeEvent* _event) override;

    /**
     * @brief Переопределяем для запуска анимании декорирования
     */
    /** @{ */
    void focusInEvent(QFocusEvent* _event) override;
    void focusOutEvent(QFocusEvent* _event) override;
    /** @} */

    /**
     * @brief Переопределяем для обработки нажатия на иконке вспомогательного действия
     */
    void mousePressEvent(QMouseEvent* _event) override;
    void mouseReleaseEvent(QMouseEvent* _event) override;

    /**
     * @brief Корректируем курсор при наведении на иконку
     */
    void mouseMoveEvent(QMouseEvent* _event) override;

    /**
     * @brief Переопределяем для ручной обработки некоторых клавиш
     */
    void keyPressEvent(QKeyEvent* _event) override;

    /**
     * @brief Ловим LayoutDirectionChanged
     */
    void changeEvent(QEvent* _event) override;

    /**
     * @brief Разрешаем вставлять только плоский текст
     */
    void insertFromMimeData(const QMimeData* _source) override;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};
