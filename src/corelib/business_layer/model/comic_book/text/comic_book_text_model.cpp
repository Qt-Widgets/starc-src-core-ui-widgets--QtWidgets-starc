#include "comic_book_text_model.h"

#include "comic_book_text_block_parser.h"
#include "comic_book_text_model_page_item.h"
#include "comic_book_text_model_panel_item.h"

#include <business_layer/model/characters/character_model.h>
#include <business_layer/model/characters/characters_model.h>
#include <business_layer/model/comic_book/comic_book_dictionaries_model.h>
#include <business_layer/model/comic_book/comic_book_information_model.h>
#include <business_layer/model/text/text_model_folder_item.h>
#include <business_layer/model/text/text_model_text_item.h>
#include <business_layer/templates/comic_book_template.h>
#include <data_layer/storage/settings_storage.h>
#include <data_layer/storage/storage_facade.h>
#include <utils/helpers/text_helper.h>
#include <utils/logging.h>
#include <utils/shugar.h>

#include <QRegularExpression>
#include <QStringListModel>
#include <QXmlStreamReader>


namespace BusinessLayer {

namespace {
const char* kMimeType = "application/x-starc/comicbook/text/item";
}

class ComicBookTextModel::Implementation
{
public:
    explicit Implementation(ComicBookTextModel* _q);

    /**
     * @brief Получить корневой элемент
     */
    TextModelItem* rootItem() const;

    /**
     * @brief Обновить номера страниц, панелей и реплик
     */
    void updateNumbering();


    /**
     * @brief Родительский элемент
     */
    ComicBookTextModel* q = nullptr;

    /**
     * @brief Модель информации о проекте
     */
    ComicBookInformationModel* informationModel = nullptr;

    /**
     * @brief Модель справочников
     */
    ComicBookDictionariesModel* dictionariesModel = nullptr;

    /**
     * @brief Модель персонажей
     */
    CharactersModel* charactersModel = nullptr;

    /**
     * @brief Нужно ли обновить справочники, которые строятся в рантайме
     */
    bool needUpdateRuntimeDictionaries = false;

    /**
     * @brief Справочники, которые строятся в рантайме
     */
    QStringListModel* charactersModelFromText = nullptr;
};

ComicBookTextModel::Implementation::Implementation(ComicBookTextModel* _q)
    : q(_q)
{
}

TextModelItem* ComicBookTextModel::Implementation::rootItem() const
{
    return q->itemForIndex({});
}

void ComicBookTextModel::Implementation::updateNumbering()
{
    int pageNumber = 1;
    int panelNumber = 1;
    int dialogueNumber = 1;
    std::function<void(const TextModelItem*)> updateChildNumbering;
    updateChildNumbering = [this, &pageNumber, &panelNumber, &dialogueNumber,
                            &updateChildNumbering](const TextModelItem* _item) {
        for (int childIndex = 0; childIndex < _item->childCount(); ++childIndex) {
            auto childItem = _item->childAt(childIndex);
            switch (childItem->type()) {
            case TextModelItemType::Folder: {
                updateChildNumbering(childItem);
                break;
            }

            case TextModelItemType::Group: {
                const auto groupItem = static_cast<TextModelGroupItem*>(childItem);
                if (groupItem->groupType() == TextGroupType::Page) {
                    panelNumber = 1;
                    dialogueNumber = 1;
                    updateChildNumbering(childItem);

                    auto pageItem = static_cast<ComicBookTextModelPageItem*>(childItem);
                    pageItem->setPageNumber(pageNumber, dictionariesModel->singlePageIntros(),
                                            dictionariesModel->multiplePageIntros());
                    pageItem->updateCounters();
                } else {
                    updateChildNumbering(childItem);
                    auto panelItem = static_cast<ComicBookTextModelPanelItem*>(childItem);
                    panelItem->setPanelNumber(panelNumber, dictionariesModel->singlePanelIntros(),
                                              dictionariesModel->multiplePanelIntros());
                }
                break;
            }

            case TextModelItemType::Text: {
                auto textItem = static_cast<TextModelTextItem*>(childItem);
                if (textItem->paragraphType() == TextParagraphType::Character
                    && !textItem->isCorrection()) {
                    textItem->setNumber(dialogueNumber++);
                }
                break;
            }

            default:
                break;
            }
        }
    };
    updateChildNumbering(rootItem());
}


// ****


ComicBookTextModel::ComicBookTextModel(QObject* _parent)
    : TextModel(_parent, ComicBookTextModel::createFolderItem(TextFolderType::Root))
    , d(new Implementation(this))
{
    auto updateNumbering = [this] { d->updateNumbering(); };

    //
    // Обновляем счётчики после того, как операции вставки и удаления будут обработаны клиентами
    // модели (главным образом внутри прокси-моделей), т.к. обновление элемента модели может
    // приводить к падению внутри них
    //
    connect(this, &ComicBookTextModel::afterRowsInserted, this, updateNumbering);
    connect(this, &ComicBookTextModel::afterRowsRemoved, this, updateNumbering);

    connect(this, &ComicBookTextModel::contentsChanged, this,
            [this] { d->needUpdateRuntimeDictionaries = true; });
}

ComicBookTextModel::~ComicBookTextModel() = default;

TextModelFolderItem* ComicBookTextModel::createFolderItem(TextFolderType _type) const
{
    Q_UNUSED(_type)

    return new TextModelFolderItem(this);
}

TextModelGroupItem* ComicBookTextModel::createGroupItem(TextGroupType _type) const
{
    switch (_type) {
    case TextGroupType::Page: {
        return new ComicBookTextModelPageItem(this);
    }

    case TextGroupType::Panel: {
        return new ComicBookTextModelPanelItem(this);
    }

    default: {
        Q_ASSERT(false);
        return nullptr;
    }
    }
}

TextModelTextItem* ComicBookTextModel::createTextItem() const
{
    return new TextModelTextItem(this);
}

void ComicBookTextModel::setInformationModel(ComicBookInformationModel* _model)
{
    if (d->informationModel == _model) {
        return;
    }

    d->informationModel = _model;
}

ComicBookInformationModel* ComicBookTextModel::informationModel() const
{
    return d->informationModel;
}

void ComicBookTextModel::setDictionariesModel(ComicBookDictionariesModel* _model)
{
    d->dictionariesModel = _model;
}

ComicBookDictionariesModel* ComicBookTextModel::dictionariesModel() const
{
    return d->dictionariesModel;
}

void ComicBookTextModel::setCharactersModel(CharactersModel* _model)
{
    if (d->charactersModel) {
        d->charactersModel->disconnect(this);
    }

    d->charactersModel = _model;

    connect(d->charactersModel, &CharactersModel::contentsChanged, this,
            [this] { d->needUpdateRuntimeDictionaries = true; });
}

QAbstractItemModel* ComicBookTextModel::charactersModel() const
{
    if (d->charactersModelFromText != nullptr) {
        return d->charactersModelFromText;
    }

    return d->charactersModel;
}

CharacterModel* ComicBookTextModel::character(const QString& _name) const
{
    return d->charactersModel->character(_name);
}

void ComicBookTextModel::createCharacter(const QString& _name)
{
    d->charactersModel->createCharacter(_name);
}

void ComicBookTextModel::updateCharacterName(const QString& _oldName, const QString& _newName)
{
    const auto oldName = TextHelper::smartToUpper(_oldName);
    std::function<void(const TextModelItem*)> updateCharacterBlock;
    updateCharacterBlock = [this, oldName, _newName,
                            &updateCharacterBlock](const TextModelItem* _item) {
        for (int childIndex = 0; childIndex < _item->childCount(); ++childIndex) {
            auto childItem = _item->childAt(childIndex);
            switch (childItem->type()) {
            case TextModelItemType::Folder:
            case TextModelItemType::Group: {
                updateCharacterBlock(childItem);
                break;
            }

            case TextModelItemType::Text: {
                auto textItem = static_cast<TextModelTextItem*>(childItem);
                if (textItem->paragraphType() == TextParagraphType::Character
                    && ComicBookCharacterParser::name(textItem->text()) == oldName) {
                    auto text = textItem->text();
                    text.remove(0, oldName.length());
                    text.prepend(_newName);
                    textItem->setText(text);
                    updateItem(textItem);
                } else if (textItem->text().contains(oldName, Qt::CaseInsensitive)) {
                    auto text = textItem->text();
                    const QRegularExpression nameMatcher(QString("\\b(%1)\\b").arg(oldName),
                                                         QRegularExpression::CaseInsensitiveOption);
                    auto match = nameMatcher.match(text);
                    while (match.hasMatch()) {
                        text.remove(match.capturedStart(), match.capturedLength());
                        const auto capturedName = match.captured();
                        const auto capitalizeEveryWord = true;
                        const auto newName = capturedName == oldName
                            ? TextHelper::smartToUpper(_newName)
                            : TextHelper::toSentenceCase(_newName, capitalizeEveryWord);
                        text.insert(match.capturedStart(), newName);

                        match = nameMatcher.match(text, match.capturedStart() + _newName.length());
                    }

                    textItem->setText(text);
                    updateItem(textItem);
                }
                break;
            }

            default:
                break;
            }
        }
    };

    emit rowsAboutToBeChanged();
    updateCharacterBlock(d->rootItem());
    emit rowsChanged();
}

QVector<QModelIndex> ComicBookTextModel::characterDialogues(const QString& _name) const
{
    QVector<QModelIndex> modelIndexes;
    for (int row = 0; row < rowCount(); ++row) {
        modelIndexes.append(index(row, 0));
    }
    QString lastCharacter;
    QVector<QModelIndex> dialoguesIndexes;
    while (!modelIndexes.isEmpty()) {
        const auto itemIndex = modelIndexes.takeFirst();
        const auto item = itemForIndex(itemIndex);
        if (item->type() == TextModelItemType::Text) {
            const auto textItem = static_cast<TextModelTextItem*>(item);
            switch (textItem->paragraphType()) {
            case TextParagraphType::Character: {
                lastCharacter = ComicBookCharacterParser::name(textItem->text());
                break;
            }

            case TextParagraphType::Parenthetical: {
                //
                // Не очищаем имя персонажа, идём до реплики
                //
                break;
            }

            case TextParagraphType::Dialogue:
            case TextParagraphType::Lyrics: {
                if (lastCharacter == _name) {
                    dialoguesIndexes.append(itemIndex);
                }
                break;
            }

            default: {
                lastCharacter.clear();
                break;
            }
            }
        }

        for (int childRow = 0; childRow < rowCount(itemIndex); ++childRow) {
            modelIndexes.append(index(childRow, 0, itemIndex));
        }
    }

    return dialoguesIndexes;
}

QSet<QString> ComicBookTextModel::findCharactersFromText() const
{
    QSet<QString> characters;
    std::function<void(const TextModelItem*)> findCharacters;
    findCharacters = [&characters, &findCharacters](const TextModelItem* _item) {
        for (int childIndex = 0; childIndex < _item->childCount(); ++childIndex) {
            auto childItem = _item->childAt(childIndex);
            switch (childItem->type()) {
            case TextModelItemType::Group: {
                findCharacters(childItem);
                break;
            }

            case TextModelItemType::Text: {
                auto textItem = static_cast<TextModelTextItem*>(childItem);
                if (textItem->paragraphType() == TextParagraphType::Character) {
                    characters.insert(ComicBookCharacterParser::name(textItem->text()));
                }
                break;
            }

            default:
                break;
            }
        }
    };
    findCharacters(d->rootItem());

    return characters;
}

void ComicBookTextModel::updateRuntimeDictionariesIfNeeded()
{
    if (!d->needUpdateRuntimeDictionaries) {
        return;
    }

    updateRuntimeDictionaries();

    d->needUpdateRuntimeDictionaries = false;
}

void ComicBookTextModel::updateRuntimeDictionaries()
{
    const bool useCharactersFromText
        = settingsValue(DataStorageLayer::kComponentsComicBookEditorUseCharactersFromTextKey)
              .toBool();

    //
    // Если нет необходимости собирать персонажей из текста
    //
    if (!useCharactersFromText) {
        //
        // ... удалим старый список, если он был создан
        //
        if (d->charactersModelFromText != nullptr) {
            d->charactersModelFromText->deleteLater();
            d->charactersModelFromText = nullptr;
        }
        //
        // ... и далее ничего не делаем
        //
        return;
    }

    //
    // В противном случае, собираем персонажей из текста
    //
    QSet<QString> characters;
    std::function<void(const TextModelItem*)> findCharactersInText;
    findCharactersInText = [this, &findCharactersInText, &characters](const TextModelItem* _item) {
        for (int childIndex = 0; childIndex < _item->childCount(); ++childIndex) {
            auto childItem = _item->childAt(childIndex);
            switch (childItem->type()) {
            case TextModelItemType::Folder:
            case TextModelItemType::Group: {
                findCharactersInText(childItem);
                break;
            }

            case TextModelItemType::Text: {
                auto textItem = static_cast<TextModelTextItem*>(childItem);

                switch (textItem->paragraphType()) {
                case TextParagraphType::Character: {
                    const auto character = ComicBookCharacterParser::name(textItem->text());
                    if (d->charactersModel->exists(character)) {
                        characters.insert(character);
                    }
                    break;
                }

                default: {
                    break;
                }
                }

                break;
            }

            default:
                break;
            }
        }
    };
    findCharactersInText(d->rootItem());
    //
    // ... не забываем приаттачить всех персонажей, у кого определена роль в истории
    //
    for (int row = 0; row < d->charactersModel->rowCount(); ++row) {
        const auto character = d->charactersModel->character(row);
        if (character->storyRole() != CharacterStoryRole::Undefined) {
            characters.insert(character->name());
        }
    }
    //
    // ... создаём (при необходимости) и наполняем модель
    //
    if (d->charactersModelFromText == nullptr) {
        d->charactersModelFromText = new QStringListModel(this);
    }
    d->charactersModelFromText->setStringList(characters.values());
}

QStringList ComicBookTextModel::mimeTypes() const
{
    return { kMimeType };
}

void ComicBookTextModel::initEmptyDocument()
{
    auto pageText = createTextItem();
    pageText->setParagraphType(TextParagraphType::PageHeading);
    auto page = createGroupItem(TextGroupType::Page);
    page->appendItem(pageText);
    appendItem(page);
}

void ComicBookTextModel::finalizeInitialization()
{
    emit rowsAboutToBeChanged();
    d->updateNumbering();
    emit rowsChanged();
}

ChangeCursor ComicBookTextModel::applyPatch(const QByteArray& _patch)
{
    const auto changeCursor = TextModel::applyPatch(_patch);

    d->updateNumbering();

    return changeCursor;
}

} // namespace BusinessLayer
