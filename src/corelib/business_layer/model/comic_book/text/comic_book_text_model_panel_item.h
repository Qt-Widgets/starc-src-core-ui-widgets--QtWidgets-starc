#pragma once

#include <business_layer/model/text/text_model_group_item.h>


namespace BusinessLayer {

class ComicBookTextModel;

/**
 * @brief Класс элементов панелей модели комикса
 */
class CORE_LIBRARY_EXPORT ComicBookTextModelPanelItem : public TextModelGroupItem
{
public:
    /**
     * @brief Номер панели
     */
    struct PanelNumber {
        int fromPanel = 0;
        int panelCount = 0;
        QString text;
    };

    /**
     * @brief Роли данных из модели
     */
    enum {
        PanelDialoguesWordsSizeRole = TextModelGroupItem::GroupUserRole + 1,
    };

public:
    explicit ComicBookTextModelPanelItem(const ComicBookTextModel* _model);
    ~ComicBookTextModelPanelItem() override;

    /**
     * @brief Номер панели
     */
    std::optional<PanelNumber> panelNumber() const;
    void setPanelNumber(int& _fromNumber, const QStringList& _singlePanelIntros,
                        const QStringList& _multiplePanelIntros);

    /**
     * @brief Получить количество слов в репликах
     */
    int dialoguesWordsCount() const;

    /**
     * @brief Определяем интерфейс получения данных панели
     */
    QVariant data(int _role) const override;

protected:
    /**
     * @brief Считываем дополнительный контент
     */
    QStringRef readCustomContent(QXmlStreamReader& _contentReader) override;

    /**
     * @brief Сформировать xml-блок с кастомными данными элемента
     */
    QByteArray customContent() const override;

    /**
     * @brief Обновляем текст панели при изменении кого-то из детей
     */
    void handleChange() override;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace BusinessLayer
