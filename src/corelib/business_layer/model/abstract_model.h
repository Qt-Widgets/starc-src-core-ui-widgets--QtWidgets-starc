#pragma once

#include <QAbstractItemModel>

#include <corelib_global.h>


class DiffMatchPatchController;

namespace Domain {
class DocumentObject;
}

namespace BusinessLayer {

class AbstractImageWrapper;
class AbstractModelItem;

struct ChangeCursor {
    AbstractModelItem* item = nullptr;
    int position = -1;
};

/**
 * @brief Абстрактная модель для работы над документами
 */
class CORE_LIBRARY_EXPORT AbstractModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit AbstractModel(const QVector<QString>& _tags, QObject* _parent = nullptr);
    ~AbstractModel() override;

    /**
     * @brief Получить документ модели
     */
    Domain::DocumentObject* document() const;

    /**
     * @brief Задать документ со структурой
     */
    void setDocument(Domain::DocumentObject* _document);

    /**
     * @brief Получить название документа
     */
    virtual QString documentName() const;

    /**
     * @brief Задать название документа
     */
    virtual void setDocumentName(const QString& _name);

    /**
     * @brief Получить цвет документа
     */
    virtual QColor documentColor() const;

    /**
     * @brief Задать цвет документа
     */
    virtual void setDocumentColor(const QColor& _color);

    /**
     * @brief Установить загрузчик изображений
     */
    void setImageWrapper(AbstractImageWrapper* _image);

    /**
     * @brief Очистить все загруженные данные
     */
    void clear();

    /**
     * @brief Пересохранить контент без вычленения изменений
     * @note Используется при импорте, чтобы переписать импортированный контент таким же, но с
     * правильным оформлением
     */
    void reassignContent();

    /**
     * @brief Сохранить изменения прямо сейчас
     */
    void saveChanges();

    /**
     * @brief Отменить последнее изменение текущего пользователя
     */
    ChangeCursor undo();

    /**
     * @brief Отменить заданное изменение
     */
    void undoChange(const QByteArray& _undo, const QByteArray& _redo);

    /**
     * @brief Повторить отменённое действие
     */
    ChangeCursor redo();

    /**
     * @brief Смержить документ с заданным
     */
    bool mergeDocumentChanges(const QByteArray _content, const QVector<QByteArray>& _patches);

    /**
     * @brief Наложить заданные изменения на документ
     */
    void applyDocumentChanges(const QVector<QByteArray>& _patches);

    /**
     * @brief Применяются ли в данный момент изменения
     */
    bool isChangesApplyingInProcess() const;

    /**
     * @brief Реализация базовых вещей для древовидной модели
     */
    /** @{ */
    QModelIndex index(int _row, int _column, const QModelIndex& _parent = {}) const override;
    QModelIndex parent(const QModelIndex& _child) const override;
    int columnCount(const QModelIndex& _parent = {}) const override;
    int rowCount(const QModelIndex& _parent = {}) const override;
    QVariant data(const QModelIndex& _index, int _role) const override;
    /** @} */

signals:
    /**
     * @brief Изменилось название документа модели
     */
    void documentNameChanged(const QString& _name);

    /**
     * @brief Сменился цвет документа
     */
    void documentColorChanged(const QColor& _color);

    /**
     * @brief Модель хочет отменить изменение с заданным индексом изменения в истории
     */
    void undoRequested(int _undoStep);

    /**
     * @brief Данные изменились
     */
    void contentsChanged(const QByteArray& _undo, const QByteArray& _redo);

    /**
     * @brief Сигналы для выполнения действий после изменения модели, чтобы пропустить вперёд
     *        обработку событий модели в нативных Qt-классах, например в прокси модели
     */
    void afterRowsInserted(const QModelIndex& _parent, int _first, int _last);
    void afterRowsRemoved(const QModelIndex& _parent, int _first, int _last);

    /**
     * @brief Сигналы для обёртки групповых действий над элементами модели,
     *        которые не ложатся в рамки стандартных механизмов Qt
     */
    void rowsAboutToBeChanged();
    void rowsChanged();

    /**
     * @brief Запрос на удаление модели
     * @note Используется за пределами доступности модели структуры,
     *       чтобы удаление модели корректно проходило через корзину
     */
    void removeRequested();

protected:
    /**
     * @brief Транзакция, для управления операция сброса данных модели
     */
    void beginResetModelTransaction();
    void endResetModelTransaction();

    /**
     * @brief Настроить хранилище изображений
     */
    virtual void initImageWrapper();

    /**
     * @brief Настроить документ
     */
    virtual void initDocument() = 0;

    /**
     * @brief Очистить документ
     */
    virtual void clearDocument() = 0;

    /**
     * @brief Сформировать xml-содержимое документа
     */
    virtual QByteArray toXml() const = 0;

    /**
     * @brief Применить заданное изменение для модели
     * @return Положение курсора внутри документа в конце изменения
     */
    virtual ChangeCursor applyPatch(const QByteArray& _patch);

    /**
     * @brief Получить обёртку для работы с изображениями
     */
    AbstractImageWrapper* imageWrapper() const;

    /**
     * @brief Получить управляющего процессом применения изменений
     */
    const DiffMatchPatchController& dmpController() const;

    /**
     * @brief Установить данные в документ
     */
    void updateDocumentContent();

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace BusinessLayer
