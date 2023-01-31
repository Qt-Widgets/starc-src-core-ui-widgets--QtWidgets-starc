#include "comic_book_text_block_parser.h"

#include <utils/helpers/text_helper.h>

#include <QRegularExpression>
#include <QString>
#include <QStringList>


namespace BusinessLayer {

QString ComicBookPageParser::pageNumber(const QString& _text)
{
    return {};
}

QString ComicBookPanelParser::panelTitle(const QString& _text)
{
    return _text.split(":").constFirst().trimmed();
}

QString ComicBookPanelParser::panelDescription(const QString& _text)
{
    const auto sectionsDividerIndex = _text.indexOf(": ");
    if (sectionsDividerIndex == -1) {
        return {};
    }

    return _text.mid(sectionsDividerIndex + 2);
}

// ****

ComicBookCharacterParser::Section ComicBookCharacterParser::section(const QString& _text)
{
    if (_text.split("(").count() == 2) {
        return SectionState;
    } else {
        return SectionName;
    }
}

QString ComicBookCharacterParser::number(const QString& _text)
{
    const auto dotIndex = _text.indexOf(". ");
    if (dotIndex == -1) {
        return {};
    }

    return _text.left(dotIndex);
}

QString ComicBookCharacterParser::name(const QString& _text)
{
    //
    // В блоке персонажа так же могут быть указания, что он говорит за кадром и т.п.
    // эти указания даются в скобках
    //

    QString name = _text.trimmed();
    if (name.endsWith(':')) {
        name.chop(1);
    }
    return TextHelper::smartToUpper(name.remove(QRegularExpression("^\\d*[.]"))
                                        .remove(QRegularExpression("[(](.*)"))
                                        .simplified());
}

QString ComicBookCharacterParser::extension(const QString& _text)
{
    //
    // В блоке персонажа так же могут быть указания, что он говорит за кадром и т.п.
    // эти указания даются в скобках, они нам как раз и нужны
    //

    const QRegularExpression rx_state("[(](.*)");
    QRegularExpressionMatch match = rx_state.match(_text);
    QString state;
    if (match.hasMatch()) {
        state = match.captured(0);
        state = state.remove("(").remove(")");
    }
    return TextHelper::smartToUpper(state).simplified();
}

} // namespace BusinessLayer
