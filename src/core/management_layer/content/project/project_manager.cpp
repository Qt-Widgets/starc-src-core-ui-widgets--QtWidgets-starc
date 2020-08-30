#include "project_manager.h"

#include "project_models_facade.h"
#include "project_plugins_builder.h"

#include <business_layer/model/characters/character_model.h>
#include <business_layer/model/characters/characters_model.h>
#include <business_layer/model/locations/location_model.h>
#include <business_layer/model/locations/locations_model.h>
#include <business_layer/model/project/project_information_model.h>
#include <business_layer/model/structure/structure_model.h>
#include <business_layer/model/structure/structure_model_item.h>
#include <business_layer/model/structure/structure_proxy_model.h>

#include <data_layer/storage/document_change_storage.h>
#include <data_layer/storage/document_data_storage.h>
#include <data_layer/storage/document_storage.h>
#include <data_layer/storage/settings_storage.h>
#include <data_layer/storage/storage_facade.h>

#include <domain/document_object.h>

#include <ui/abstract_navigator.h>
#include <ui/design_system/design_system.h>
#include <ui/project/create_document_dialog.h>
#include <ui/project/project_navigator.h>
#include <ui/project/project_tool_bar.h>
#include <ui/project/project_view.h>
#include <ui/widgets/context_menu/context_menu.h>
#include <ui/widgets/dialog/dialog.h>

#include <QDateTime>
#include <QHBoxLayout>
#include <QSet>
#include <QStandardItemModel>
#include <QUuid>
#include <QVariantAnimation>


namespace ManagementLayer
{

class ProjectManager::Implementation
{
public:
    explicit Implementation(QWidget* _parent);

    /**
     * @brief Действия контекстного меню
     */
    enum class ContextMenuAction {
        AddDocument,
        RemoveDocument,
        EmptyRecycleBin
    };

    /**
     * @brief Обновить модель действий контекстного меню навигатора
     */
    void updateNavigatorContextMenu(const QModelIndex& _index);

    /**
     * @brief Выполнить заданное действие контекстного меню
     */
    void executeContextMenuAction(const QModelIndex& _itemIndex, const QModelIndex& _contextMenuIndex);

    /**
     * @brief Добавить документ в проект
     */
    void addDocument(const QModelIndex& _itemIndex);

    /**
     * @brief Добавить документ в заданный контейнер
     */
    void addDocumentToContainer(Domain::DocumentObjectType _containerType,
        Domain::DocumentObjectType _documentType, const QString& _documentName, const QByteArray& _content = {});

    /**
     * @brief Удалить документ
     */
    void removeDocument(const QModelIndex& _itemIndex);

    /**
     * @brief Очистить корзину
     */
    void emptyRecycleBin(const QModelIndex& _recycleBinIndex);

    //
    // Данные
    //

    QWidget* topLevelWidget = nullptr;

    Ui::ProjectToolBar* toolBar = nullptr;

    Ui::ProjectNavigator* navigator = nullptr;
    QStandardItemModel* navigatorContextMenuModel = nullptr;

    Ui::ProjectView* view = nullptr;

    BusinessLayer::StructureModel* projectStructureModel = nullptr;
    BusinessLayer::StructureProxyModel* projectStructureProxyModel = nullptr;

    DataStorageLayer::DocumentDataStorage documentDataStorage;

    ProjectModelsFacade modelsFacade;
    ProjectPluginsBuilder pluginsBuilder;
};

ProjectManager::Implementation::Implementation(QWidget* _parent)
    : topLevelWidget(_parent),
      toolBar(new Ui::ProjectToolBar(_parent)),
      navigator(new Ui::ProjectNavigator(_parent)),
      navigatorContextMenuModel(new QStandardItemModel(navigator)),
      view(new Ui::ProjectView(_parent)),
      projectStructureModel(new BusinessLayer::StructureModel(navigator)),
      projectStructureProxyModel(new BusinessLayer::StructureProxyModel(projectStructureModel)),
      modelsFacade(&documentDataStorage)
{
    toolBar->hide();
    navigator->hide();
    view->hide();

    navigator->setModel(projectStructureProxyModel);
    navigator->setContextMenuModel(navigatorContextMenuModel);
}

void ProjectManager::Implementation::updateNavigatorContextMenu(const QModelIndex& _index)
{
    navigatorContextMenuModel->clear();

    const auto currentItemIndex = projectStructureProxyModel->mapToSource(_index);
    const auto currentItem = projectStructureModel->itemForIndex(currentItemIndex);

    if (currentItem->type() == Domain::DocumentObjectType::RecycleBin) {
        if (currentItem->hasChildren()) {
            auto emptyRecicleBin = new QStandardItem(tr("Empty recycle bin"));
            emptyRecicleBin->setData(u8"\U000f05e8", Qt::DecorationRole);
            emptyRecicleBin->setData(static_cast<int>(ContextMenuAction::EmptyRecycleBin), Qt::UserRole);
            navigatorContextMenuModel->appendRow(emptyRecicleBin);
        }
        return;
    }


    auto addDocument = new QStandardItem(tr("Add document"));
    addDocument->setData(u8"\U000f0415", Qt::DecorationRole);
    addDocument->setData(static_cast<int>(ContextMenuAction::AddDocument), Qt::UserRole);
    navigatorContextMenuModel->appendRow(addDocument);

    const QSet<Domain::DocumentObjectType> cantBeRemovedItems
            = { Domain::DocumentObjectType::Project,
                Domain::DocumentObjectType::Characters,
                Domain::DocumentObjectType::Locations,
                Domain::DocumentObjectType::ScreenplayTitlePage,
                Domain::DocumentObjectType::ScreenplaySynopsis,
                Domain::DocumentObjectType::ScreenplayTreatment,
                Domain::DocumentObjectType::ScreenplayText,
                Domain::DocumentObjectType::ScreenplayStatistics };
    if (_index.isValid() && !cantBeRemovedItems.contains(currentItem->type())) {
        auto removeDocument = new QStandardItem(tr("Remove document"));
        removeDocument->setData(u8"\U000f01b4", Qt::DecorationRole);
        removeDocument->setData(static_cast<int>(ContextMenuAction::RemoveDocument), Qt::UserRole);
        navigatorContextMenuModel->appendRow(removeDocument);
    }
}

void ProjectManager::Implementation::executeContextMenuAction(const QModelIndex& _itemIndex,
    const QModelIndex& _contextMenuIndex)
{
    if (!_contextMenuIndex.isValid()) {
        return;
    }

    const auto actionData = _contextMenuIndex.data(Qt::UserRole);
    if (actionData.isNull()) {
        return;
    }

    const auto action = static_cast<ContextMenuAction>(actionData.toInt());
    switch (action) {
        case ContextMenuAction::AddDocument: {
            addDocument(_itemIndex);
            break;
        }

        case ContextMenuAction::RemoveDocument: {
            removeDocument(_itemIndex);
            break;
        }

        case ContextMenuAction::EmptyRecycleBin: {
            emptyRecycleBin(_itemIndex);
            break;
        }
    }
}

void ProjectManager::Implementation::addDocument(const QModelIndex& _itemIndex)
{
    //
    // TODO: вложение в выделенный в дереве элемент
    //

    auto dialog = new Ui::CreateDocumentDialog(topLevelWidget);

    connect(dialog, &Ui::CreateDocumentDialog::createPressed, navigator,
            [this, dialog] (Domain::DocumentObjectType _type, const QString& _name)
    {
        projectStructureModel->addDocument(_type, _name);
        dialog->hideDialog();
    });
    connect(dialog, &Ui::CreateDocumentDialog::disappeared, dialog, &Ui::CreateDocumentDialog::deleteLater);

    dialog->showDialog();
}

void ProjectManager::Implementation::addDocumentToContainer(Domain::DocumentObjectType _containerType,
    Domain::DocumentObjectType _documentType, const QString& _documentName, const QByteArray& _content)
{
    for (int itemRow = 0; itemRow < projectStructureModel->rowCount(); ++itemRow) {
        const auto itemIndex = projectStructureModel->index(itemRow, 0);
        const auto item = projectStructureModel->itemForIndex(itemIndex);
        if (item->type() == _containerType) {
            projectStructureModel->addDocument(_documentType, _documentName, itemIndex, _content);
            break;
        }
    }
}

void ProjectManager::Implementation::removeDocument(const QModelIndex& _itemIndex)
{
    const auto mappedItemIndex = projectStructureProxyModel->mapToSource(_itemIndex);
    auto item = projectStructureModel->itemForIndex(mappedItemIndex);
    if (item == nullptr) {
        return;
    }

    auto itemTopLevelParent = item->parent();
    if (itemTopLevelParent == nullptr) {
        return;
    }
    while (itemTopLevelParent->parent()
           && itemTopLevelParent->parent()->type() != Domain::DocumentObjectType::Undefined) {
        itemTopLevelParent = itemTopLevelParent->parent();
    }

    //
    // Если документ ещё не в корзине, то переносим его в корзину
    //
    if (itemTopLevelParent->type() != Domain::DocumentObjectType::RecycleBin) {
        projectStructureModel->moveItemToRecycleBin(item);
        return;
    }

    //
    // Если документ в корзине
    //
    // ... то спросим действительно ли пользователь хочет его удалить
    //
    const int kCancelButtonId = 0;
    const int kRemoveButtonId = 1;
    auto dialog = new Dialog(topLevelWidget);
    dialog->showDialog({},
                       tr("Do you really want to permanently remove document?"),
                       {{ kCancelButtonId, tr("No"), Dialog::RejectButton },
                        { kRemoveButtonId, tr("Yes, remove"), Dialog::NormalButton }});
    QObject::connect(dialog, &Dialog::finished,
                     [this, item, kCancelButtonId, dialog] (const Dialog::ButtonInfo& _buttonInfo)
    {
        dialog->hideDialog();

        //
        // Пользователь передумал удалять
        //
        if (_buttonInfo.id == kCancelButtonId) {
            return;
        }

        //
        // Если таки хочет, то удаляем документ
        // NOTE: порядок удаления важен
        //
        auto document = DataStorageLayer::StorageFacade::documentStorage()->document(item->uuid());
        modelsFacade.removeModelFor(document);
        DataStorageLayer::StorageFacade::documentStorage()->removeDocument(document);
        projectStructureModel->removeItem(item);
    });
    QObject::connect(dialog, &Dialog::disappeared, dialog, &Dialog::deleteLater);
}

void ProjectManager::Implementation::emptyRecycleBin(const QModelIndex& _recycleBinIndex)
{
    const auto mappedRecycleBinIndex = projectStructureProxyModel->mapToSource(_recycleBinIndex);
    auto recycleBin = projectStructureModel->itemForIndex(mappedRecycleBinIndex);
    if (recycleBin == nullptr) {
        return;
    }

    //
    // Спросим действительно ли пользователь хочет очистить корзину
    //
    const int kCancelButtonId = 0;
    const int kEmptyButtonId = 1;
    auto dialog = new Dialog(topLevelWidget);
    dialog->showDialog({},
                       tr("Do you really want to permanently remove all documents from the recycle bin?"),
                       {{ kCancelButtonId, tr("No"), Dialog::RejectButton },
                        { kEmptyButtonId, tr("Yes, remove"), Dialog::NormalButton }});
    QObject::connect(dialog, &Dialog::finished,
                     [this, recycleBin, kCancelButtonId, dialog] (const Dialog::ButtonInfo& _buttonInfo)
    {
        dialog->hideDialog();

        //
        // Пользователь передумал очищать корзину
        //
        if (_buttonInfo.id == kCancelButtonId) {
            return;
        }

        //
        // Если таки хочет, то удаляем все вложенные документы
        // NOTE: порядок удаления важен
        //
        while (recycleBin->hasChildren()) {
            auto itemToRemove = recycleBin->childAt(0);
            auto documentToRemove = DataStorageLayer::StorageFacade::documentStorage()->document(itemToRemove->uuid());
            if (documentToRemove != nullptr) {
                modelsFacade.removeModelFor(documentToRemove);
                DataStorageLayer::StorageFacade::documentStorage()->removeDocument(documentToRemove);
            }
            projectStructureModel->removeItem(itemToRemove);
        }
    });
    QObject::connect(dialog, &Dialog::disappeared, dialog, &Dialog::deleteLater);
}


// ****


ProjectManager::ProjectManager(QObject* _parent, QWidget* _parentWidget)
    : QObject(_parent),
      d(new Implementation(_parentWidget))
{
    connect(d->toolBar, &Ui::ProjectToolBar::menuPressed, this, &ProjectManager::menuRequested);
    connect(d->toolBar, &Ui::ProjectToolBar::viewPressed, this, [this] (const QString& _mimeType) {
        showView(d->navigator->currentIndex(), _mimeType);
    });

    //
    // Отображаем необходимый редактор при выборе документа в списке
    //
    connect(d->navigator, &Ui::ProjectNavigator::itemSelected, this,
            [this] (const QModelIndex& _index)
    {
        if (!_index.isValid()) {
            d->view->showDefaultPage();
            return;
        }

        //
        // Определим выделенный элемент и скорректируем интерфейс
        //
        const auto mappedIndex = d->projectStructureProxyModel->mapToSource(_index);
        const auto item = d->projectStructureModel->itemForIndex(mappedIndex);
        const auto documentMimeType = Domain::mimeTypeFor(item->type());
        //
        // ... настроим иконки представлений
        //
        d->toolBar->clearViews();
        const auto views = d->pluginsBuilder.editorsInfoFor(documentMimeType);
        for (auto view : views) {
            const auto tooltip = d->pluginsBuilder.editorDescription(view.mimeType);
            const bool isActive = view.mimeType == views.first().mimeType;
            d->toolBar->addView(view.mimeType, view.icon, tooltip, isActive);
        }

        //
        // Откроем документ на редактирование в первом из представлений
        //
        if (views.isEmpty()) {
            d->view->showDefaultPage();
            return;
        }
        showView(_index, views.first().mimeType);
    });
    //
    // Отображаем навигатор выбранного элемента
    //
    connect(d->navigator, &Ui::ProjectNavigator::itemDoubleClicked, this,
            [this] (const QModelIndex& _index)
    {
        const auto mappedIndex = d->projectStructureProxyModel->mapToSource(_index);
        if (!d->projectStructureModel->data(mappedIndex,
                static_cast<int>(BusinessLayer::StructureModelDataRole::IsNavigatorAvailable)).toBool()) {
            return;
        }

        showNavigator(_index);
    });
    connect(d->navigator, &Ui::ProjectNavigator::contextMenuUpdateRequested, this,
            [this] (const QModelIndex& _index) { d->updateNavigatorContextMenu(_index); });
    connect(d->navigator, &Ui::ProjectNavigator::contextMenuItemClicked, this,
            [this] (const QModelIndex& _contextMenuIndex)
    {
        d->executeContextMenuAction(d->navigator->currentIndex(), _contextMenuIndex);
    });
    connect(d->navigator, &Ui::ProjectNavigator::addDocumentClicked, this, [this] { d->addDocument({}); });

    //
    // Соединения с моделью структуры проекта
    //
    connect(d->projectStructureModel, &BusinessLayer::StructureModel::documentAdded,
            [this] (const QUuid& _uuid, Domain::DocumentObjectType _type, const QString& _name,
                    const QByteArray& _content)
    {
        auto document = DataStorageLayer::StorageFacade::documentStorage()->storeDocument(_uuid, _type);
        if (!_content.isNull()) {
            document->setContent(_content);
        }

        auto documentModel = d->modelsFacade.modelFor(document);
        documentModel->setDocumentName(_name);

        switch (_type) {
            case Domain::DocumentObjectType::Character: {
                auto charactersDocument = DataStorageLayer::StorageFacade::documentStorage()->document(
                                              Domain::DocumentObjectType::Characters);
                auto charactersModel = static_cast<BusinessLayer::CharactersModel*>(d->modelsFacade.modelFor(charactersDocument));
                auto characterModel = static_cast<BusinessLayer::CharacterModel*>(documentModel);
                charactersModel->addCharacterModel(characterModel);

                break;
            }

            case Domain::DocumentObjectType::Location: {
                auto locationsDocument = DataStorageLayer::StorageFacade::documentStorage()->document(
                                             Domain::DocumentObjectType::Locations);
                auto locationsModel = static_cast<BusinessLayer::LocationsModel*>(d->modelsFacade.modelFor(locationsDocument));
                auto locationModel = static_cast<BusinessLayer::LocationModel*>(documentModel);
                locationsModel->addLocationModel(locationModel);

                break;
            }

            default: break;
        }
    });
    connect(d->projectStructureModel, &BusinessLayer::StructureModel::contentsChanged, this,
            [this] (const QByteArray& _undo, const QByteArray& _redo)
    {
        handleModelChange(d->projectStructureModel, _undo, _redo);
    });

    //
    // Соединения представления
    //
    connect(d->view, &Ui::ProjectView::createNewItemPressed, this, [this] {
        d->addDocument({});
    });

    //
    // Соединения со строителем моделей
    //
    connect(&d->modelsFacade, &ProjectModelsFacade::modelNameChanged, this,
            [this] (BusinessLayer::AbstractModel* _model, const QString& _name)
    {
        auto item = d->projectStructureModel->itemForUuid(_model->document()->uuid());
        d->projectStructureModel->setItemName(item, _name);
    });
    connect(&d->modelsFacade, &ProjectModelsFacade::modelContentChanged, this, &ProjectManager::handleModelChange);
    connect(&d->modelsFacade, &ProjectModelsFacade::projectNameChanged, this, &ProjectManager::projectNameChanged);
    connect(&d->modelsFacade, &ProjectModelsFacade::projectLoglineChanged, this, &ProjectManager::projectLoglineChanged);
    connect(&d->modelsFacade, &ProjectModelsFacade::projectCoverChanged, this, &ProjectManager::projectCoverChanged);
    connect(&d->modelsFacade, &ProjectModelsFacade::createCharacterRequested, this,
            [this] (const QString& _name, const QByteArray& _content)
    {
        d->addDocumentToContainer(Domain::DocumentObjectType::Characters,
                                  Domain::DocumentObjectType::Character,
                                  _name, _content);
    });
    connect(&d->modelsFacade, &ProjectModelsFacade::createLocationRequested, this,
            [this] (const QString& _name, const QByteArray& _content)
    {
        d->addDocumentToContainer(Domain::DocumentObjectType::Locations,
                                  Domain::DocumentObjectType::Location,
                                  _name, _content);
    });
    //
    auto setDocumentVisible = [this] (BusinessLayer::AbstractModel* _screenplayModel,
                                      Domain::DocumentObjectType _type, bool _visible) {
        auto screenplayItem = d->projectStructureModel->itemForUuid(_screenplayModel->document()->uuid());
        for (int childIndex = 0; childIndex < screenplayItem->childCount(); ++childIndex) {
            auto childItem = screenplayItem->childAt(childIndex);
            if (childItem->type() == _type) {
                d->projectStructureModel->setItemVisible(childItem, _visible);
                break;
            }
        }
    };
    connect(&d->modelsFacade, &ProjectModelsFacade::screenplayTitlePageVisibilityChanged, this,
            [setDocumentVisible] (BusinessLayer::AbstractModel* _screenplayModel, bool _visible)
    {
        setDocumentVisible(_screenplayModel, Domain::DocumentObjectType::ScreenplayTitlePage, _visible);
    });
    connect(&d->modelsFacade, &ProjectModelsFacade::screenplaySynopsisVisibilityChanged, this,
            [setDocumentVisible] (BusinessLayer::AbstractModel* _screenplayModel, bool _visible)
    {
        setDocumentVisible(_screenplayModel, Domain::DocumentObjectType::ScreenplaySynopsis, _visible);
    });
    connect(&d->modelsFacade, &ProjectModelsFacade::screenplayTreatmentVisibilityChanged, this,
            [setDocumentVisible] (BusinessLayer::AbstractModel* _screenplayModel, bool _visible)
    {
        setDocumentVisible(_screenplayModel, Domain::DocumentObjectType::ScreenplayTreatment, _visible);
    });
    connect(&d->modelsFacade, &ProjectModelsFacade::screenplayTextVisibilityChanged, this,
            [setDocumentVisible] (BusinessLayer::AbstractModel* _screenplayModel, bool _visible)
    {
        setDocumentVisible(_screenplayModel, Domain::DocumentObjectType::ScreenplayText, _visible);
    });
    connect(&d->modelsFacade, &ProjectModelsFacade::screenplayStatisticsVisibilityChanged, this,
            [setDocumentVisible] (BusinessLayer::AbstractModel* _screenplayModel, bool _visible)
    {
        setDocumentVisible(_screenplayModel, Domain::DocumentObjectType::ScreenplayStatistics, _visible);
    });
}

ProjectManager::~ProjectManager() = default;

QWidget* ProjectManager::toolBar() const
{
    return d->toolBar;
}

QWidget* ProjectManager::navigator() const
{
    return d->navigator;
}

QWidget* ProjectManager::view() const
{
    return d->view;
}

void ProjectManager::reconfigureAll()
{
    d->pluginsBuilder.reconfigureAll();
}

void ProjectManager::reconfigureScreenplayEditor()
{
    d->pluginsBuilder.reconfigureScreenplayEditor();
}

void ProjectManager::reconfigureScreenplayNavigator()
{
    d->pluginsBuilder.reconfigureScreenplayNavigator();
}

void ProjectManager::loadCurrentProject(const QString& _name, const QString& _path)
{
    //
    // Загружаем структуру
    //
    d->projectStructureModel->setProjectName(_name);
    d->projectStructureModel->setDocument(
        DataStorageLayer::StorageFacade::documentStorage()->document(Domain::DocumentObjectType::Structure));

    //
    // Загружаем информацию о проекте
    //
    auto projectInformationModel
            = static_cast<BusinessLayer::ProjectInformationModel*>(
                  d->modelsFacade.modelFor(
                      DataStorageLayer::StorageFacade::documentStorage()->document(
                          Domain::DocumentObjectType::Project)));
    if (projectInformationModel->name().isEmpty()) {
        projectInformationModel->setName(_name);
    } else {
        emit projectNameChanged(projectInformationModel->name());
        emit projectLoglineChanged(projectInformationModel->logline());
        emit projectCoverChanged(projectInformationModel->cover());
    }


    //
    // Синхронизировать структуру с облаком
    //

    //
    // Открыть структуру
    //

    //
    // Загрузить состояние дерева
    //
    d->navigator->restoreState(DataStorageLayer::StorageFacade::settingsStorage()->value(
                                   DataStorageLayer::projectStructureKey(_path),
                                   DataStorageLayer::SettingsStorage::SettingsPlace::Application));

    //
    // Восстановить последнее состояние дерева, если возможно
    //

    //
    // Синхронизировать выбранный документ
    //

    //
    // Синхрониировать все остальные изменения
    //
}

void ProjectManager::closeCurrentProject(const QString& _path)
{
    //
    // Сохранить состояние дерева
    //
    DataStorageLayer::StorageFacade::settingsStorage()->setValue(
                DataStorageLayer::projectStructureKey(_path),
                d->navigator->saveState(),
                DataStorageLayer::SettingsStorage::SettingsPlace::Application);

    //
    // Очищаем структуру
    //
    d->projectStructureModel->clear();

    //
    // Сбрасываем все плагины
    //
    d->pluginsBuilder.reset();

    //
    // Очищаем все загруженные модели документов
    //
    d->modelsFacade.clear();

    //
    // Сбрасываем загруженные изображения
    //
    d->documentDataStorage.clear();
}

void ProjectManager::saveChanges()
{
    //
    // Сохраняем структуру
    //
    const auto structure = d->projectStructureModel->document();
    DataStorageLayer::StorageFacade::documentStorage()->updateDocument(structure);

    //
    // Сохраняем остальные документы
    //
    for (auto model : d->modelsFacade.models()) {
        DataStorageLayer::StorageFacade::documentStorage()->updateDocument(model->document());
    }

    //
    // Сохраняем изображения
    //
    d->documentDataStorage.saveChanges();

    //
    // Сохраняем все изменения документов
    //
    DataStorageLayer::StorageFacade::documentChangeStorage()->store();
}

void ProjectManager::addCharacter(const QString& _name, const QString& _content)
{
    auto document = DataStorageLayer::StorageFacade::documentStorage()->document(Domain::DocumentObjectType::Characters);
    auto model = d->modelsFacade.modelFor(document);
    auto charactersModel = qobject_cast<BusinessLayer::CharactersModel*>(model);
    if (charactersModel == nullptr) {
        return;
    }

    charactersModel->createCharacter(_name, _content.toUtf8());
}

void ProjectManager::addLocation(const QString& _name, const QString& _content)
{
    auto document = DataStorageLayer::StorageFacade::documentStorage()->document(Domain::DocumentObjectType::Locations);
    auto model = d->modelsFacade.modelFor(document);
    auto charactersModel = qobject_cast<BusinessLayer::LocationsModel*>(model);
    if (charactersModel == nullptr) {
        return;
    }

    charactersModel->createLocation(_name, _content.toUtf8());
}

void ProjectManager::addScreenplay(const QString& _name, const QString& _titlePage,
    const QString& _synopsis, const QString& _treatment, const QString& _text)
{
    //
    // ATTENTION: Копипаста из StructureModel, быть внимательным при обновлении
    //

    using namespace Domain;

    auto createItem = [] (DocumentObjectType _type, const QString& _name) {
        auto uuid = QUuid::createUuid();
        const auto visible = true;
        return new BusinessLayer::StructureModelItem(uuid, _type, _name, {}, visible);
    };

    auto rootItem = d->projectStructureModel->itemForIndex({});
    auto screenplayItem = createItem(DocumentObjectType::Screenplay, _name);
    d->projectStructureModel->appendItem(screenplayItem, rootItem);

    d->projectStructureModel->appendItem(createItem(DocumentObjectType::ScreenplayTitlePage, tr("Title page")), screenplayItem, _titlePage.toUtf8());
    d->projectStructureModel->appendItem(createItem(DocumentObjectType::ScreenplaySynopsis, tr("Synopsis")), screenplayItem, _synopsis.toUtf8());
    d->projectStructureModel->appendItem(createItem(DocumentObjectType::ScreenplayTreatment, tr("Treatment")), screenplayItem, _treatment.toUtf8());
    d->projectStructureModel->appendItem(createItem(DocumentObjectType::ScreenplayText, tr("Text")), screenplayItem, _text.toUtf8());
    d->projectStructureModel->appendItem(createItem(DocumentObjectType::ScreenplayStatistics, tr("Statistics")), screenplayItem, {});
}

void ProjectManager::handleModelChange(BusinessLayer::AbstractModel* _model,
    const QByteArray& _undo, const QByteArray& _redo)
{
    DataStorageLayer::StorageFacade::documentChangeStorage()->appendDocumentChange(
        _model->document()->uuid(), QUuid::createUuid(), _undo, _redo,
        DataStorageLayer::StorageFacade::settingsStorage()->userName(),
        DataStorageLayer::StorageFacade::settingsStorage()->userEmail());

    emit contentsChanged();
}

void ProjectManager::showView(const QModelIndex& _itemIndex, const QString& _viewMimeType)
{
    const auto mappedItemIndex = d->projectStructureProxyModel->mapToSource(_itemIndex);
    const auto item = d->projectStructureModel->itemForIndex(mappedItemIndex);

    //
    // Определим модель
    //
    auto document = DataStorageLayer::StorageFacade::documentStorage()->document(item->uuid());
    auto model = d->modelsFacade.modelFor(document);
    if (model == nullptr) {
        d->view->showDefaultPage();
        return;
    }

    //
    // Определим представление и отобразим
    //
    auto view = d->pluginsBuilder.activateView(_viewMimeType, model);
    if (view == nullptr) {
        d->view->showDefaultPage();
        return;
    }
    d->view->setCurrentWidget(view);

    //
    // Настроим возможность перехода в навигатор
    //
    const auto navigatorMimeType = d->pluginsBuilder.navigatorMimeTypeFor(_viewMimeType);
    d->projectStructureModel->setNavigatorAvailableFor(mappedItemIndex, !navigatorMimeType.isEmpty());

    //
    // Если в данный момент отображён кастомный навигатов, откроем навигатор соответствующий редактору
    //
    if (!d->navigator->isProjectNavigatorShown()) {
        showNavigator(_itemIndex, _viewMimeType);
    }
}

void ProjectManager::showNavigator(const QModelIndex& _itemIndex, const QString& _viewMimeType)
{
    const auto mappedItemIndex = d->projectStructureProxyModel->mapToSource(_itemIndex);
    if (!mappedItemIndex.isValid()) {
        d->navigator->showProjectNavigator();
        return;
    }

    const auto item = d->projectStructureModel->itemForIndex(mappedItemIndex);

    //
    // Определим модель
    //
    auto document = DataStorageLayer::StorageFacade::documentStorage()->document(item->uuid());
    auto model = d->modelsFacade.modelFor(document);
    if (model == nullptr) {
        d->navigator->showProjectNavigator();
        return;
    }

    //
    // Определим представление и отобразим
    //
    const auto viewMimeType = !_viewMimeType.isEmpty()
                              ? _viewMimeType
                              : d->toolBar->currentViewMimeType();
    const auto navigatorMimeType = d->pluginsBuilder.navigatorMimeTypeFor(viewMimeType);
    auto view = d->pluginsBuilder.activateView(navigatorMimeType, model);
    if (view == nullptr) {
        d->navigator->showProjectNavigator();
        return;
    }

    //
    // Связываем редактор с навигатоом
    //
    d->pluginsBuilder.bind(viewMimeType, navigatorMimeType);

    //
    // Задаём заголовок навигатора и настраиваем возможность перехода к навигатору проекта
    //
    auto navigatorView = qobject_cast<Ui::AbstractNavigator*>(view);
    //
    // FIXME: Надо как-то более универсально сделать этот момент, чтобы не приходилось задавать
    //        название родителя для случаев, когда нужно имя самого документа, а не его родителя
    //
    navigatorView->setTitle(item->parent()->name());
    connect(navigatorView, &Ui::AbstractNavigator::backPressed,
            d->navigator, &Ui::ProjectNavigator::showProjectNavigator,
            Qt::UniqueConnection);
    d->navigator->setCurrentWidget(view);
}

} // namespace ManagementLayer
