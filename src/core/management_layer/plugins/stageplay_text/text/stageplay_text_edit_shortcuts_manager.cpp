#include "stageplay_text_edit_shortcuts_manager.h"

#include "stageplay_text_edit.h"

#include <business_layer/templates/stageplay_template.h>
#include <utils/helpers/shortcuts_helper.h>

#include <QShortcut>
#include <QSignalMapper>

using BusinessLayer::TextParagraphType;


namespace Ui {

class StageplayTextEditShortcutsManager::Implementation
{
public:
    explicit Implementation(StageplayTextEdit* _editor);

    /**
     * @brief Создать или обновить комбинацию для заданного типа
     */
    void createOrUpdateShortcut(TextParagraphType _forBlockType);

    //
    // Данные
    //

    /**
     * @brief Редактор сценария
     */
    StageplayTextEdit* stageplayEditor = nullptr;

    /**
     * @brief Виджет в контексте которого будут активироваться горячие клавиши
     */
    QWidget* shortcutsContext = nullptr;

    /**
     * @brief Тип блока - горячие клавиши
     */
    QHash<TextParagraphType, QShortcut*> paragraphTypeToShortcut;
};

StageplayTextEditShortcutsManager::Implementation::Implementation(StageplayTextEdit* _editor)
    : stageplayEditor(_editor)
{
}

void StageplayTextEditShortcutsManager::Implementation::createOrUpdateShortcut(
    TextParagraphType _forBlockType)
{
    if (shortcutsContext == nullptr) {
        return;
    }

    const QKeySequence keySequence(ShortcutsHelper::stageplayShortcut(_forBlockType));

    if (paragraphTypeToShortcut.contains(_forBlockType)) {
        paragraphTypeToShortcut.value(_forBlockType)->setKey(keySequence);
    } else {
        paragraphTypeToShortcut[_forBlockType]
            = new QShortcut(keySequence, shortcutsContext, 0, 0, Qt::WidgetWithChildrenShortcut);
    }
}


// ****


StageplayTextEditShortcutsManager::StageplayTextEditShortcutsManager(StageplayTextEdit* _parent)
    : QObject(_parent)
    , d(new Implementation(_parent))
{
    Q_ASSERT(_parent);
}

StageplayTextEditShortcutsManager::~StageplayTextEditShortcutsManager() = default;

void StageplayTextEditShortcutsManager::setShortcutsContext(QWidget* _context)
{
    if (d->shortcutsContext == _context) {
        return;
    }

    d->shortcutsContext = _context;
    qDeleteAll(d->paragraphTypeToShortcut);

    //
    // Создаём шорткаты
    //
    d->createOrUpdateShortcut(TextParagraphType::UnformattedText);
    d->createOrUpdateShortcut(TextParagraphType::SceneHeading);
    d->createOrUpdateShortcut(TextParagraphType::Character);
    d->createOrUpdateShortcut(TextParagraphType::Dialogue);
    d->createOrUpdateShortcut(TextParagraphType::Sound);
    d->createOrUpdateShortcut(TextParagraphType::Music);
    d->createOrUpdateShortcut(TextParagraphType::Cue);
    d->createOrUpdateShortcut(TextParagraphType::InlineNote);

    //
    // Настраиваем их
    //
    QSignalMapper* mapper = new QSignalMapper(this);
    for (auto shortcutIter = d->paragraphTypeToShortcut.begin();
         shortcutIter != d->paragraphTypeToShortcut.end(); ++shortcutIter) {
        connect(shortcutIter.value(), &QShortcut::activated, mapper,
                qOverload<>(&QSignalMapper::map));
        mapper->setMapping(shortcutIter.value(), static_cast<int>(shortcutIter.key()));
    }
    connect(mapper, &QSignalMapper::mappedInt, this, [this](int _value) {
        d->stageplayEditor->setCurrentParagraphType(static_cast<TextParagraphType>(_value));
    });
}

void StageplayTextEditShortcutsManager::reconfigure()
{
    //
    // Обновим сочетания клавиш для всех блоков
    //
    for (auto iter = d->paragraphTypeToShortcut.begin(); iter != d->paragraphTypeToShortcut.end();
         ++iter) {
        d->createOrUpdateShortcut(iter.key());
    }
}

void StageplayTextEditShortcutsManager::setEnabled(bool _enabled)
{
    for (auto shortcut : d->paragraphTypeToShortcut) {
        shortcut->setEnabled(_enabled);
    }
}

QString StageplayTextEditShortcutsManager::shortcut(TextParagraphType _forBlockType) const
{
    if (!d->paragraphTypeToShortcut.contains(_forBlockType)) {
        return {};
    }

    return d->paragraphTypeToShortcut.value(_forBlockType)
        ->key()
        .toString(QKeySequence::NativeText);
}

} // namespace Ui
