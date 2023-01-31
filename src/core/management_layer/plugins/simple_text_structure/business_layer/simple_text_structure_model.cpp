#include "simple_text_structure_model.h"

#include <business_layer/model/simple_text/simple_text_model.h>
#include <business_layer/model/text/text_model_text_item.h>
#include <business_layer/templates/simple_text_template.h>

#include <QApplication>


namespace BusinessLayer {

class SimpleTextStructureModel::Implementation
{
public:
    SimpleTextModel* textModel = nullptr;
};


// ****


SimpleTextStructureModel::SimpleTextStructureModel(QObject* _parent)
    : QSortFilterProxyModel(_parent)
    , d(new Implementation)
{
}

SimpleTextStructureModel::~SimpleTextStructureModel() = default;

void SimpleTextStructureModel::setSourceModel(QAbstractItemModel* _sourceModel)
{
    d->textModel = qobject_cast<SimpleTextModel*>(_sourceModel);
    QSortFilterProxyModel::setSourceModel(_sourceModel);
}

bool SimpleTextStructureModel::filterAcceptsRow(int _sourceRow,
                                                const QModelIndex& _sourceParent) const
{
    if (d->textModel == nullptr) {
        return false;
    }

    const auto itemIndex = d->textModel->index(_sourceRow, 0, _sourceParent);
    const auto item = d->textModel->itemForIndex(itemIndex);

    //
    // Показываем главы
    //
    if (item->type() == TextModelItemType::Group) {
        return true;
    }
    //
    // Остальное не показываем
    //
    return false;
}

} // namespace BusinessLayer
