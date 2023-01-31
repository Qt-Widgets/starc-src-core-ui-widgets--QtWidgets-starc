#pragma once

#include <interfaces/ui/i_document_view.h>
#include <ui/widgets/widget/widget.h>


namespace Ui {


/**
 * @brief Представление информации о пьесе
 */
class StageplayInformationView : public Widget, public IDocumentView
{
    Q_OBJECT

public:
    explicit StageplayInformationView(QWidget* _parent = nullptr);
    ~StageplayInformationView() override;

    /**
     * @brief Реализация интерфейса IDocumentView
     */
    /** @{ */
    QWidget* asQWidget() override;
    void setEditingMode(ManagementLayer::DocumentEditingMode _mode) override;
    /** @} */

    void setName(const QString& _name);
    Q_SIGNAL void nameChanged(const QString& _name);

    void setTagline(const QString& _tagline);
    Q_SIGNAL void taglineChanged(const QString& _logline);

    void setLogline(const QString& _logline);
    Q_SIGNAL void loglineChanged(const QString& _logline);

    void setTitlePageVisible(bool _visible);
    Q_SIGNAL void titlePageVisibleChanged(bool _visible);

    void setSynopsisVisible(bool _visible);
    Q_SIGNAL void synopsisVisibleChanged(bool _visible);

    void setStageplayTextVisible(bool _visible);
    Q_SIGNAL void stageplayTextVisibleChanged(bool _visible);

    void setStageplayStatisticsVisible(bool _visible);
    Q_SIGNAL void stageplayStatisticsVisibleChanged(bool _visible);

protected:
    /**
     * @brief Обновить переводы
     */
    void updateTranslations() override;

    /**
     * @brief Обновляем виджет при изменении дизайн системы
     */
    void designSystemChangeEvent(DesignSystemChangeEvent* _event) override;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace Ui
