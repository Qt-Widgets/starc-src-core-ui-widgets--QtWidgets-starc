#include "shortcuts_helper.h"

#include <business_layer/templates/screenplay_template.h>
#include <data_layer/storage/settings_storage.h>
#include <data_layer/storage/storage_facade.h>


QString ShortcutsHelper::treatmentShortcut(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(
               QString("%1/%2").arg(DataStorageLayer::kComponentsTreatmentEditorShortcutsKey,
                                    BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setTreatmentShortcut(BusinessLayer::TextParagraphType _type,
                                           const QString& _shortcut)
{
    setSettingsValue(QString("%1/%2").arg(DataStorageLayer::kComponentsTreatmentEditorShortcutsKey,
                                          BusinessLayer::toString(_type)),
                     _shortcut);
}

QString ShortcutsHelper::treatmentJumpByTab(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(QString("%1/styles-jumping/from-%2-by-tab")
                             .arg(DataStorageLayer::kComponentsTreatmentEditorKey,
                                  BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setTreatmentJumpByTab(BusinessLayer::TextParagraphType _fromType,
                                            BusinessLayer::TextParagraphType _toType)
{
    setSettingsValue(QString("%1/styles-jumping/from-%2-by-tab")
                         .arg(DataStorageLayer::kComponentsTreatmentEditorKey,
                              BusinessLayer::toString(_fromType)),
                     BusinessLayer::toString(_toType));
}

QString ShortcutsHelper::treatmentJumpByEnter(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(QString("%1/styles-jumping/from-%2-by-enter")
                             .arg(DataStorageLayer::kComponentsTreatmentEditorKey,
                                  BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setTreatmentJumpByEnter(BusinessLayer::TextParagraphType _fromType,
                                              BusinessLayer::TextParagraphType _toType)
{
    setSettingsValue(QString("%1/styles-jumping/from-%2-by-enter")
                         .arg(DataStorageLayer::kComponentsTreatmentEditorKey,
                              BusinessLayer::toString(_fromType)),
                     BusinessLayer::toString(_toType));
}

QString ShortcutsHelper::treatmentChangeByTab(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(QString("%1/styles-changing/from-%2-by-tab")
                             .arg(DataStorageLayer::kComponentsTreatmentEditorKey,
                                  BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setTreatmentChangeByTab(BusinessLayer::TextParagraphType _fromType,
                                              BusinessLayer::TextParagraphType _toType)
{
    setSettingsValue(QString("%1/styles-changing/from-%2-by-tab")
                         .arg(DataStorageLayer::kComponentsTreatmentEditorKey,
                              BusinessLayer::toString(_fromType)),
                     BusinessLayer::toString(_toType));
}

QString ShortcutsHelper::treatmentChangeByEnter(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(QString("%1/styles-changing/from-%2-by-enter")
                             .arg(DataStorageLayer::kComponentsTreatmentEditorKey,
                                  BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setTreatmentChangeByEnter(BusinessLayer::TextParagraphType _fromType,
                                                BusinessLayer::TextParagraphType _toType)
{
    setSettingsValue(QString("%1/styles-changing/from-%2-by-enter")
                         .arg(DataStorageLayer::kComponentsTreatmentEditorKey,
                              BusinessLayer::toString(_fromType)),
                     BusinessLayer::toString(_toType));
}

//

QString ShortcutsHelper::screenplayShortcut(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(
               QString("%1/%2").arg(DataStorageLayer::kComponentsScreenplayEditorShortcutsKey,
                                    BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setScreenplayShortcut(BusinessLayer::TextParagraphType _type,
                                            const QString& _shortcut)
{
    setSettingsValue(QString("%1/%2").arg(DataStorageLayer::kComponentsScreenplayEditorShortcutsKey,
                                          BusinessLayer::toString(_type)),
                     _shortcut);
}

QString ShortcutsHelper::screenplayJumpByTab(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(QString("%1/styles-jumping/from-%2-by-tab")
                             .arg(DataStorageLayer::kComponentsScreenplayEditorKey,
                                  BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setScreenplayJumpByTab(BusinessLayer::TextParagraphType _fromType,
                                             BusinessLayer::TextParagraphType _toType)
{
    setSettingsValue(QString("%1/styles-jumping/from-%2-by-tab")
                         .arg(DataStorageLayer::kComponentsScreenplayEditorKey,
                              BusinessLayer::toString(_fromType)),
                     BusinessLayer::toString(_toType));
}

QString ShortcutsHelper::screenplayJumpByEnter(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(QString("%1/styles-jumping/from-%2-by-enter")
                             .arg(DataStorageLayer::kComponentsScreenplayEditorKey,
                                  BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setScreenplayJumpByEnter(BusinessLayer::TextParagraphType _fromType,
                                               BusinessLayer::TextParagraphType _toType)
{
    setSettingsValue(QString("%1/styles-jumping/from-%2-by-enter")
                         .arg(DataStorageLayer::kComponentsScreenplayEditorKey,
                              BusinessLayer::toString(_fromType)),
                     BusinessLayer::toString(_toType));
}

QString ShortcutsHelper::screenplayChangeByTab(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(QString("%1/styles-changing/from-%2-by-tab")
                             .arg(DataStorageLayer::kComponentsScreenplayEditorKey,
                                  BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setScreenplayChangeByTab(BusinessLayer::TextParagraphType _fromType,
                                               BusinessLayer::TextParagraphType _toType)
{
    setSettingsValue(QString("%1/styles-changing/from-%2-by-tab")
                         .arg(DataStorageLayer::kComponentsScreenplayEditorKey,
                              BusinessLayer::toString(_fromType)),
                     BusinessLayer::toString(_toType));
}

QString ShortcutsHelper::screenplayChangeByEnter(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(QString("%1/styles-changing/from-%2-by-enter")
                             .arg(DataStorageLayer::kComponentsScreenplayEditorKey,
                                  BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setScreenplayChangeByEnter(BusinessLayer::TextParagraphType _fromType,
                                                 BusinessLayer::TextParagraphType _toType)
{
    setSettingsValue(QString("%1/styles-changing/from-%2-by-enter")
                         .arg(DataStorageLayer::kComponentsScreenplayEditorKey,
                              BusinessLayer::toString(_fromType)),
                     BusinessLayer::toString(_toType));
}

QString ShortcutsHelper::audioplayShortcut(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(
               QString("%1/%2").arg(DataStorageLayer::kComponentsAudioplayEditorShortcutsKey,
                                    BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setAudioplayShortcut(BusinessLayer::TextParagraphType _type,
                                           const QString& _shortcut)
{
    setSettingsValue(QString("%1/%2").arg(DataStorageLayer::kComponentsAudioplayEditorShortcutsKey,
                                          BusinessLayer::toString(_type)),
                     _shortcut);
}

QString ShortcutsHelper::stageplayShortcut(BusinessLayer::TextParagraphType _type)
{
    return settingsValue(
               QString("%1/%2").arg(DataStorageLayer::kComponentsStageplayEditorShortcutsKey,
                                    BusinessLayer::toString(_type)))
        .toString();
}

void ShortcutsHelper::setStageplayShortcut(BusinessLayer::TextParagraphType _type,
                                           const QString& _shortcut)
{
    setSettingsValue(QString("%1/%2").arg(DataStorageLayer::kComponentsStageplayEditorShortcutsKey,
                                          BusinessLayer::toString(_type)),
                     _shortcut);
}
