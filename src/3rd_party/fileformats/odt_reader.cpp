/***********************************************************************
 *
 * Copyright (C) 2011, 2012, 2013, 2014 Graeme Gott <graeme@gottcode.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "odt_reader.h"

#include "qtzip/QtZipReader"

#include <QTextDocument>

namespace {
	qreal pixelsFromInches(qint32 _inches)
	{
		qreal pixels = _inches * 96.0;
		return pixels;
	}
}

//-----------------------------------------------------------------------------

OdtReader::OdtReader() :
	m_in_block(true)
{
	m_xml.setNamespaceProcessing(false);
}

//-----------------------------------------------------------------------------

bool OdtReader::canRead(QIODevice* device)
{
	return QtZipReader::canRead(device) &&
			(device->peek(77).right(47) == "mimetypeapplication/vnd.oasis.opendocument.text");
}

//-----------------------------------------------------------------------------

void OdtReader::readData(QIODevice* device)
{
	m_in_block = m_cursor.document()->blockCount();
	m_block_format = m_cursor.blockFormat();

	// Open archive
	QtZipReader zip(device);

	// Read archive
	if (zip.isReadable()) {
		const QString files[] = { QString::fromLatin1("styles.xml"), QString::fromLatin1("content.xml") };
		for (int i = 0; i < 2; ++i) {
			QByteArray data = zip.fileData(files[i]);
			if (data.isEmpty()) {
				continue;
			}
			m_xml.addData(data);
			readDocument();
			if (m_xml.hasError()) {
				m_error = m_xml.errorString();
				break;
			}
			m_xml.clear();
		}
	} else {
		m_error = tr("Unable to open archive.");
	}

	// Close archive
	zip.close();

	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

//-----------------------------------------------------------------------------

void OdtReader::readDocument()
{
	m_xml.readNextStartElement();
	while (m_xml.readNextStartElement()) {
		if (m_xml.qualifiedName() == "office:styles") {
			readStylesGroup();
		} else if (m_xml.qualifiedName() == "office:automatic-styles") {
			readStylesGroup();
		} else if (m_xml.qualifiedName() == "office:body") {
			readBody();
		}  else {
			m_xml.skipCurrentElement();
		}
	}
}

//-----------------------------------------------------------------------------

void OdtReader::readStylesGroup()
{
	while (m_xml.readNextStartElement()) {
		if (m_xml.qualifiedName() == "style:style") {
			readStyle();
		} else {
			m_xml.skipCurrentElement();
		}
	}
}

//-----------------------------------------------------------------------------

void OdtReader::readStyle()
{
	QXmlStreamAttributes attributes = m_xml.attributes();

	QString name = attributes.value(QLatin1String("style:name")).toString();

	int type = -1;
	QStringRef family = attributes.value(QLatin1String("style:family"));
	if (family == "paragraph") {
		type = 0;
	} else if (family == "text") {
		type = 1;
	} else {
		m_xml.skipCurrentElement();
		return;
	}

	if (!m_styles[type].contains(name)) {
		m_styles[type][name] = Style(m_block_format);
	}
	Style& style = m_styles[type][name];

	QString parent_style = attributes.value(QLatin1String("style:parent-style-name")).toString();
	if (!parent_style.isEmpty()) {
		Style& parent = m_styles[type][parent_style];
		style.block_format = parent.block_format;
		style.char_format = parent.char_format;
		parent.children += name;
	}

	if (name.startsWith("Head")) {
		int heading = qBound(1, name.at(name.length() - 1).digitValue(), 6);
		style.block_format.setProperty(QTextFormat::UserProperty, heading);
	}

	while (m_xml.readNextStartElement()) {
		if (m_xml.qualifiedName() == "style:paragraph-properties") {
			readStyleParagraphProperties(style.block_format);
		} else if (m_xml.qualifiedName() == "style:text-properties") {
			readStyleTextProperties(style.char_format);
		} else {
			m_xml.skipCurrentElement();
		}
	}

	// Recursively apply style to children
	QStringList children = style.children;
	while (!children.isEmpty()) {
		Style& childstyle = m_styles[type][children.takeFirst()];

		QTextBlockFormat block_format = style.block_format;
		block_format.merge(childstyle.block_format);
		childstyle.block_format = block_format;

		QTextCharFormat char_format = style.char_format;
		char_format.merge(childstyle.char_format);
		childstyle.char_format = char_format;

		children.append(childstyle.children);
	}
}

//-----------------------------------------------------------------------------

void OdtReader::readStyleParagraphProperties(QTextBlockFormat& format)
{
	QXmlStreamAttributes attributes = m_xml.attributes();

	QStringRef align = attributes.value(QLatin1String("fo:text-align"));
	if (align == "start") {
		format.setAlignment(Qt::AlignLeading);
	} else if (align == "end") {
		format.setAlignment(Qt::AlignTrailing);
	} else if (align == "left") {
		format.setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
	} else if (align == "right") {
		format.setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
	} else if (align == "center") {
		format.setAlignment(Qt::AlignHCenter);
	} else if (align == "justify") {
		format.setAlignment(Qt::AlignJustify);
	}

	QStringRef direction = attributes.value(QLatin1String("style:writing-mode"));
	if (direction == "rl-tb" || direction == "rl") {
		format.setLayoutDirection(Qt::RightToLeft);
	} else if (direction == "lr-tb" || direction == "lr") {
		format.setLayoutDirection(Qt::LeftToRight);
	}

	if (attributes.hasAttribute(QLatin1String("fo:margin-right"))) {
		QString margin = attributes.value(QLatin1String("fo:margin-right")).toString();
		QString type = margin.right(2);
		margin.chop(2);

		// Internal indent units are 0.5in
		int indent = 0;
		if (type == QLatin1String("in")) {
			indent = qRound(margin.toDouble() * 2.0);
		} else if (type == QLatin1String("cm")) {
			indent = qRound(margin.toDouble() / 1.27);
		} else if (type == QLatin1String("mm")) {
			indent = qRound(margin.toDouble() / 12.7);
		} else if (type == QLatin1String("pt")) {
			// 72pt to inch
			indent = qRound(margin.toDouble() / 36.0);
		} else if (type == QLatin1String("pc")) {
			// 6pc to inch
			indent = qRound(margin.toDouble() / 3.0);
		} else if (type == QLatin1String("px")) {
			// 96px to inch
			indent = qRound(margin.toDouble() / 48.0);
		}
		format.setRightMargin(::pixelsFromInches(indent));
	}

	if (attributes.hasAttribute(QLatin1String("fo:margin-left"))) {
		QString margin = attributes.value(QLatin1String("fo:margin-left")).toString();
		QString type = margin.right(2);
		margin.chop(2);

		// Internal indent units are 0.5in
		int indent = 0;
		if (type == QLatin1String("in")) {
			indent = qRound(margin.toDouble() * 2.0);
		} else if (type == QLatin1String("cm")) {
			indent = qRound(margin.toDouble() / 1.27);
		} else if (type == QLatin1String("mm")) {
			indent = qRound(margin.toDouble() / 12.7);
		} else if (type == QLatin1String("pt")) {
			// 72pt to inch
			indent = qRound(margin.toDouble() / 36.0);
		} else if (type == QLatin1String("pc")) {
			// 6pc to inch
			indent = qRound(margin.toDouble() / 3.0);
		} else if (type == QLatin1String("px")) {
			// 96px to inch
			indent = qRound(margin.toDouble() / 48.0);
		}
		format.setLeftMargin(::pixelsFromInches(indent));
	}

	if (attributes.hasAttribute(QLatin1String("style:default-outline-level"))) {
		QString level = attributes.value(QLatin1String("style:default-outline-level")).toString();
		int heading = qBound(1, level.toInt(), 6);
		format.setProperty(QTextFormat::UserProperty, heading);
	}

	m_xml.skipCurrentElement();
}

//-----------------------------------------------------------------------------

void OdtReader::readStyleTextProperties(QTextCharFormat& format)
{
	QXmlStreamAttributes attributes = m_xml.attributes();

	if (attributes.hasAttribute(QLatin1String("fo:font-weight"))) {
		if (attributes.value(QLatin1String("fo:font-weight")) == "bold") {
			format.setFontWeight(QFont::Bold);
		} else {
			format.setFontWeight(QFont::Normal);
		}
	}

	if (attributes.hasAttribute(QLatin1String("fo:font-style"))) {
		format.setFontItalic(attributes.value(QLatin1String("fo:font-style")) != "normal");
	}

	if (attributes.hasAttribute(QLatin1String("fo:text-transform"))) {
		if (attributes.value(QLatin1String("fo:text-transform")) == "uppercase") {
			format.setFontCapitalization(QFont::AllUppercase);
		}
	}

	if (attributes.hasAttribute(QLatin1String("style:text-underline-style"))) {
		format.setFontUnderline(attributes.value(QLatin1String("style:text-underline-style")) != "none");
	}

	if (attributes.hasAttribute(QLatin1String("style:text-line-through-type"))) {
		format.setFontStrikeOut(attributes.value(QLatin1String("style:text-line-through-type")) != "none");
	}

	if (attributes.hasAttribute(QLatin1String("style:text-position"))) {
		QStringRef position = attributes.value((QLatin1String("style:text-position")));
		if (position == "super") {
			format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
		} else if (position == "sub") {
			format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
		} else {
			QString value = position.toString();
			value.chop(1);
			if (value.toInt() > 0) {
				format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
			} else {
				format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
			}
		}
	}

	m_xml.skipCurrentElement();
}

//-----------------------------------------------------------------------------

void OdtReader::readBody()
{
	m_cursor.beginEditBlock();
	while (m_xml.readNextStartElement()) {
		if (m_xml.qualifiedName() == "office:text") {
			readBodyText();
		} else {
			m_xml.skipCurrentElement();
		}
	}
	m_cursor.endEditBlock();
}

//-----------------------------------------------------------------------------

void OdtReader::readBodyText()
{
	while (m_xml.readNextStartElement()) {
		if (m_xml.qualifiedName() == "text:p") {
			readParagraph();
		} else if (m_xml.qualifiedName() == "text:h") {
			int heading = -1;
			QXmlStreamAttributes attributes = m_xml.attributes();
			if (attributes.hasAttribute(QLatin1String("text:outline-level"))) {
				QString level = attributes.value(QLatin1String("text:outline-level")).toString();
				heading = qBound(1, level.toInt(), 6);
			}
			readParagraph(heading);
		} else if (m_xml.qualifiedName() == "text:section") {
			readBodyText();
		} else {
			m_xml.skipCurrentElement();
		}
	}
}

//-----------------------------------------------------------------------------

void OdtReader::readParagraph(int level)
{
	QTextBlockFormat block_format;
	QTextCharFormat char_format;

	// Style paragraph
	QXmlStreamAttributes attributes = m_xml.attributes();
	if (attributes.hasAttribute(QLatin1String("text:style-name"))) {
		const Style& style = m_styles[0][attributes.value(QLatin1String("text:style-name")).toString()];
		block_format = style.block_format;
		char_format = style.char_format;
	}

	if (level == -1) {
		level = qBound(1, block_format.property(QTextFormat::UserProperty).toInt(), 6);
	} else if (level == 0) {
		level = qBound(0, block_format.property(QTextFormat::UserProperty).toInt(), 6);
	}
	if (level) {
		block_format.setProperty(QTextFormat::UserProperty, level);
		char_format = QTextCharFormat();
	}

	// Create paragraph
	if (!m_in_block) {
		m_cursor.insertBlock(block_format, char_format);
		m_in_block = true;
	} else {
		m_cursor.mergeBlockFormat(block_format);
		m_cursor.mergeBlockCharFormat(char_format);
	}

	// Read paragraph text
	readText();
	m_in_block = false;

	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

//-----------------------------------------------------------------------------

void OdtReader::readSpan()
{
	QXmlStreamAttributes attributes = m_xml.attributes();

	// Style text
	QTextCharFormat format = m_cursor.charFormat();
	if (attributes.hasAttribute(QLatin1String("text:style-name"))) {
		const Style& style = m_styles[1][attributes.value(QLatin1String("text:style-name")).toString()];
		m_cursor.mergeCharFormat(style.char_format);
	}

	if (attributes.hasAttribute(QLatin1String("text:class-names"))) {
		QStringList styles = attributes.value(QLatin1String("text:class-names")).toString().simplified().split(QLatin1Char(' '), QString::SkipEmptyParts);
		int count = styles.count();
		for (int i = 0; i < count; ++i) {
			const Style& style = m_styles[1][styles.at(i)];
			m_cursor.mergeCharFormat(style.char_format);
		}
	}

	// Read styled text
	readText();

	// Restore previous style
	m_cursor.setCharFormat(format);
}

//-----------------------------------------------------------------------------

void OdtReader::readText()
{
	int depth = 1;
	while (depth && (m_xml.readNext() != QXmlStreamReader::Invalid)) {
		if (m_xml.isCharacters()) {
			m_cursor.insertText(m_xml.text().toString());
		} else if (m_xml.isStartElement()) {
			++depth;
			if (m_xml.qualifiedName() == "text:span") {
				readSpan();
				--depth;
			} else if (m_xml.qualifiedName() == "text:s") {
				int spaces = m_xml.attributes().value(QLatin1String("text:c")).toString().toInt();
				m_cursor.insertText(QString(qMax(1, spaces), QLatin1Char(' ')));
			} else if (m_xml.qualifiedName() == "text:tab") {
				m_cursor.insertText(QLatin1String("\t"));
			} else if (m_xml.qualifiedName() == "text:line-break") {
				m_cursor.insertText(QChar(0x2028));
			}
		} else if (m_xml.isEndElement()) {
			--depth;
		}
	}
}

//-----------------------------------------------------------------------------
