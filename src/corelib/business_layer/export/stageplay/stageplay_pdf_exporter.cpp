#include "stageplay_pdf_exporter.h"

#include "stageplay_export_options.h"

#include <business_layer/document/text/text_block_data.h>
#include <business_layer/model/stageplay/stageplay_information_model.h>
#include <business_layer/model/stageplay/text/stageplay_text_model.h>
#include <business_layer/model/text/text_model_text_item.h>
#include <business_layer/templates/stageplay_template.h>
#include <utils/helpers/measurement_helper.h>
#include <utils/helpers/text_helper.h>

#include <QLocale>
#include <QPainter>


namespace BusinessLayer {

StageplayPdfExporter::StageplayPdfExporter()
    : StageplayExporter()
    , AbstractPdfExporter()
{
}

void StageplayPdfExporter::updateExportOptions(TextModel* _model,
                                               ExportOptions& _exportOptions) const
{
    const auto textModel = qobject_cast<StageplayTextModel*>(_model);
    Q_ASSERT(textModel);

    _exportOptions.header = textModel->informationModel()->header();
    _exportOptions.footer = textModel->informationModel()->footer();
}

void StageplayPdfExporter::printBlockDecorations(QPainter* _painter, qreal _pageYPos,
                                                 const QRectF& _body,
                                                 TextParagraphType _paragraphType,
                                                 const QRectF& _blockRect, const QTextBlock& _block,
                                                 const ExportOptions& _exportOptions) const
{
    const auto& exportTemplate = documentTemplate(_exportOptions);
    const auto& exportOptions = static_cast<const StageplayExportOptions&>(_exportOptions);

    //
    // Покажем номер блока, если необходимо
    //
    if (exportOptions.showBlockNumbers && !_block.text().isEmpty()
        && (_paragraphType == TextParagraphType::Dialogue
            || _paragraphType == TextParagraphType::Sound
            || _paragraphType == TextParagraphType::Music
            || _paragraphType == TextParagraphType::Cue)) {

        const auto blockData = static_cast<TextBlockData*>(_block.userData());
        if (blockData != nullptr) {
            QFont font = _block.charFormat().font();
            font.setBold(false);
            font.setUnderline(false);
            _painter->setFont(font);
            //
            const auto textItem = static_cast<TextModelTextItem*>(blockData->item());
            if (textItem && textItem->number().has_value()) {
                const QString dialogueNumber = textItem->number()->text;
                const int numberDelta
                    = TextHelper::fineTextWidthF(dialogueNumber, _painter->font());
                const auto characterBlockStyle
                    = exportTemplate.paragraphStyle(TextParagraphType::Character);
                QRectF dialogueNumberRect;
                if (QLocale().textDirection() == Qt::LeftToRight) {
                    if (characterBlockStyle.marginsOnHalfPage().left() > 0) {
                        dialogueNumberRect = QRectF(
                            MeasurementHelper::mmToPx(exportTemplate.pageMargins().left()),
                            _blockRect.top() <= _pageYPos
                                ? (_pageYPos
                                   + MeasurementHelper::mmToPx(exportTemplate.pageMargins().top()))
                                : _blockRect.top(),
                            numberDelta, _blockRect.height());
                    } else {
                        const int distanceBetweenSceneNumberAndText = 10;
                        dialogueNumberRect = QRectF(
                            0,
                            _blockRect.top() <= _pageYPos
                                ? (_pageYPos
                                   + MeasurementHelper::mmToPx(exportTemplate.pageMargins().top()))
                                : _blockRect.top(),
                            MeasurementHelper::mmToPx(exportTemplate.pageMargins().left())
                                - distanceBetweenSceneNumberAndText,
                            _blockRect.height());
                    }
                } else {
                    if (_block.blockFormat().rightMargin() > numberDelta) {
                        dialogueNumberRect = QRectF(
                            MeasurementHelper::mmToPx(exportTemplate.pageMargins().left())
                                + _body.width() - numberDelta,
                            _blockRect.top() <= _pageYPos
                                ? (_pageYPos
                                   + MeasurementHelper::mmToPx(exportTemplate.pageMargins().top()))
                                : _blockRect.top(),
                            numberDelta, _blockRect.height());
                    } else {
                        const int distanceBetweenSceneNumberAndText = 10;
                        dialogueNumberRect = QRectF(
                            MeasurementHelper::mmToPx(exportTemplate.pageMargins().left())
                                + _body.width() + distanceBetweenSceneNumberAndText,
                            _blockRect.top() <= _pageYPos
                                ? (_pageYPos
                                   + MeasurementHelper::mmToPx(exportTemplate.pageMargins().top()))
                                : _blockRect.top(),
                            MeasurementHelper::mmToPx(exportTemplate.pageMargins().right())
                                - distanceBetweenSceneNumberAndText,
                            _blockRect.height());
                    }
                }
                _painter->drawText(dialogueNumberRect, Qt::AlignRight | Qt::AlignTop,
                                   dialogueNumber);
            }
        }
    }

    //
    // Прорисовка префикса/постфикса для блока текста, если это не пустая декорация
    //
    if (!_block.text().isEmpty()
        || !_block.blockFormat().boolProperty(TextBlockStyle::PropertyIsCorrection)) {
        const auto correctedBlockRect
            = QRectF({ _block.blockFormat().leftMargin()
                           + MeasurementHelper::mmToPx(exportTemplate.pageMargins().left()),
                       _blockRect.top() },
                     _blockRect.size());
        //
        // ... префикс
        //
        if (_block.charFormat().hasProperty(TextBlockStyle::PropertyPrefix)) {
            _painter->setFont(_block.charFormat().font());

            const auto prefix = _block.charFormat().stringProperty(TextBlockStyle::PropertyPrefix);
            auto prefixRect = _blockRect;
            prefixRect.setWidth(TextHelper::fineTextWidthF(prefix, _painter->font()));
            prefixRect.moveLeft(prefixRect.left() + _block.blockFormat().leftMargin()
                                - prefixRect.width());
            prefixRect.setHeight(_painter->fontMetrics().boundingRect(prefix).height());
            _painter->drawText(prefixRect, Qt::AlignLeft | Qt::AlignBottom, prefix);
        }
        //
        // ... постфикс
        //
        if (_block.charFormat().hasProperty(TextBlockStyle::PropertyPostfix)) {
            _painter->setFont(_block.charFormat().font());

            const auto postfix
                = _block.charFormat().stringProperty(TextBlockStyle::PropertyPostfix);

            //
            // Почему-то если взять просто ширину последней строки текста, то получается
            // слишком широко в некоторых случаях, так, что постфикс рисуется очень далеко
            // от текста. Поэтому решил брать текст последней строки, добавлять к нему
            // постфикс, считать их совместную ширину и брать её, как конечную точку
            //
            const auto lastLineText
                = TextHelper::lastLineText(_block.text(), _painter->font(), _blockRect.width())
                + postfix;
            const QPoint bottomRight
                = QPoint(_blockRect.left() + _block.blockFormat().leftMargin()
                             + TextHelper::fineTextWidthF(lastLineText, _painter->font()),
                         correctedBlockRect.bottom());
            const QPoint topLeft
                = QPoint(bottomRight.x() - TextHelper::fineTextWidthF(postfix, _painter->font()),
                         correctedBlockRect.bottom()
                             - _painter->fontMetrics().boundingRect(postfix).height());
            const QRect postfixRect(topLeft, bottomRight);
            _painter->drawText(postfixRect, Qt::AlignLeft | Qt::AlignBottom, postfix);
        }
    }
}

} // namespace BusinessLayer
