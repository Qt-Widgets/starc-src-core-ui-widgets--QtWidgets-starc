#include "purchase_dialog.h"

#include "purchase_dialog_option.h"

#include <domain/starcloud_api.h>
#include <ui/design_system/design_system.h>
#include <ui/widgets/button/button.h>

#include <QApplication>
#include <QHBoxLayout>


namespace Ui {

class PurchaseDialog::Implementation
{
public:
    explicit Implementation(QWidget* _parent);

    /**
     * @brief Обновить состояние опций оплат
     */
    void updateOptionsState(PurchaseDialogOption* _checkedOption);


    Widget* content = nullptr;
    QVector<PurchaseDialogOption*> options;

    QHBoxLayout* buttonsLayout = nullptr;
    Button* cancelButton = nullptr;
    Button* purchaseButton = nullptr;
};

PurchaseDialog::Implementation::Implementation(QWidget* _parent)
    : content(new Widget(_parent))
    , buttonsLayout(new QHBoxLayout)
    , cancelButton(new Button(_parent))
    , purchaseButton(new Button(_parent))
{
    buttonsLayout->setContentsMargins({});
    buttonsLayout->setSpacing(0);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(purchaseButton);
}

void PurchaseDialog::Implementation::updateOptionsState(PurchaseDialogOption* _checkedOption)
{
    for (auto option : std::as_const(options)) {
        if (option == _checkedOption) {
            continue;
        }

        option->setChecked(false);
    }
}


// ****


PurchaseDialog::PurchaseDialog(QWidget* _parent)
    : AbstractDialog(_parent)
    , d(new Implementation(this))
{
    setAcceptButton(d->purchaseButton);
    setRejectButton(d->cancelButton);

    contentsLayout()->addWidget(d->content, 0, 0);
    contentsLayout()->addLayout(d->buttonsLayout, 1, 0);

    connect(d->cancelButton, &Button::clicked, this, &PurchaseDialog::canceled);
    connect(d->purchaseButton, &Button::clicked, this, [this] {
        for (auto option : std::as_const(d->options)) {
            if (option->isChecked()) {
                emit purchasePressed(option->paymentOption());
                return;
            }
        }
    });
}

PurchaseDialog::~PurchaseDialog() = default;

void PurchaseDialog::setPaymentOptions(const QVector<Domain::PaymentOption>& _paymentOptions)
{
    QGridLayout* contentLayout = nullptr;
    if (d->content->layout() == nullptr) {
        contentLayout = new QGridLayout(d->content);
    } else {
        contentLayout = qobject_cast<QGridLayout*>(d->content->layout());
        while (!contentLayout->isEmpty()) {
            auto item = contentLayout->takeAt(0);
            if (item->widget() != nullptr) {
                item->widget()->deleteLater();
            }
            delete item;
        }
    }
    d->options.clear();

    int row = 0;
    int column = 0;
    for (const auto& option : _paymentOptions) {
        auto optionWidget = new PurchaseDialogOption(option);
        optionWidget->setBackgroundColor(Ui::DesignSystem::color().background());
        optionWidget->setTextColor(Ui::DesignSystem::color().onBackground());
        if (option.duration == Domain::PaymentDuration::Lifetime) {
            optionWidget->setContentsMargins(
                Ui::DesignSystem::layout().px12(), Ui::DesignSystem::layout().px4(),
                Ui::DesignSystem::layout().px12(), Ui::DesignSystem::layout().px12());
            contentLayout->addWidget(optionWidget, row++, 0, 1, 2);
            column = 0;
        } else {
            if (column == 2) {
                ++row;
                column = 0;
            }
            optionWidget->setContentsMargins(
                column == 0 ? Ui::DesignSystem::layout().px12() : Ui::DesignSystem::layout().px4(),
                Ui::DesignSystem::layout().px4(),
                column == 0 ? Ui::DesignSystem::layout().px4() : Ui::DesignSystem::layout().px12(),
                Ui::DesignSystem::layout().px4());
            contentLayout->addWidget(optionWidget, row, column++);
        }

        connect(optionWidget, &PurchaseDialogOption::checkedChanged, this,
                [this, optionWidget] { d->updateOptionsState(optionWidget); });

        d->options.append(optionWidget);
    }
}

void PurchaseDialog::selectOption(const Domain::PaymentOption& _option)
{
    for (auto option : std::as_const(d->options)) {
        if (option->paymentOption().subscriptionType == _option.subscriptionType) {
            option->setChecked(true);
            break;
        }
    }
}

QWidget* PurchaseDialog::focusedWidgetAfterShow() const
{
    return d->content;
}

QWidget* PurchaseDialog::lastFocusableWidget() const
{
    return d->purchaseButton;
}

void PurchaseDialog::updateTranslations()
{
    setTitle(tr("Choose what suits you"));
    d->cancelButton->setText(tr("Cancel"));
    d->purchaseButton->setText(tr("Purchase"));
}

void PurchaseDialog::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    AbstractDialog::designSystemChangeEvent(_event);

    setContentFixedWidth(Ui::DesignSystem::dialog().maximumWidth());

    d->content->setBackgroundColor(Ui::DesignSystem::color().background());

    for (auto button : {
             d->cancelButton,
             d->purchaseButton,
         }) {
        button->setBackgroundColor(Ui::DesignSystem::color().accent());
        button->setTextColor(Ui::DesignSystem::color().accent());
    }

    contentsLayout()->setSpacing(static_cast<int>(Ui::DesignSystem::layout().px8()));
    d->buttonsLayout->setContentsMargins(
        QMarginsF(Ui::DesignSystem::layout().px12(), Ui::DesignSystem::layout().px12(),
                  Ui::DesignSystem::layout().px16(), Ui::DesignSystem::layout().px16())
            .toMargins());
}

} // namespace Ui
