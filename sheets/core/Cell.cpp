/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Marijn Kruisselbrink <mkruisselbrink@kde.org>
   SPDX-FileCopyrightText: 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   SPDX-FileCopyrightText: 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   SPDX-FileCopyrightText: 2004-2005 Tomas Mecir <mecirt@gmail.com>
   SPDX-FileCopyrightText: 2004-2006 Inge Wallin <inge@lysator.liu.se>
   SPDX-FileCopyrightText: 1999-2002, 2004, 2005 Laurent Montel <montel@kde.org>
   SPDX-FileCopyrightText: 2002-2005 Ariya Hidayat <ariya@kde.org>
   SPDX-FileCopyrightText: 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   SPDX-FileCopyrightText: 2002-2003 Norbert Andres <nandres@web.de>
   SPDX-FileCopyrightText: 2003 Reinhart Geiser <geiseri@kde.org>
   SPDX-FileCopyrightText: 2003-2005 Meni Livne <livne@kde.org>
   SPDX-FileCopyrightText: 2003 Peter Simonsson <psn@linux.se>
   SPDX-FileCopyrightText: 1999-2002 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2000-2002 Werner Trobin <trobin@kde.org>
   SPDX-FileCopyrightText: 1999, 2002 Harri Porten <porten@kde.org>
   SPDX-FileCopyrightText: 2002 John Dailey <dailey@vt.edu>
   SPDX-FileCopyrightText: 1998-2000 Torben Weis <weis@kde.org>
   SPDX-FileCopyrightText: 2000 Bernd Wuebben <wuebben@kde.org>
   SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org
   SPDX-FileCopyrightText: 1999 Stephan Kulow <coolo@kde.org>
   SPDX-FileCopyrightText: 1999 Michael Reiher <michael.reiher@gmx.de>
   SPDX-FileCopyrightText: 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   SPDX-FileCopyrightText: 1998-1999 Reginald Stadlbauer <reggie@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Local
#include "Cell.h"

#include "engine/CalculationSettings.h"
#include "engine/Formula.h"
#include "engine/NamedAreaManager.h"

#include "CellStorage.h"
#include "ColFormatStorage.h"
#include "Currency.h"
#include "Condition.h"
#include "Database.h"
#include "Map.h"
#include "RowFormatStorage.h"
#include "Sheet.h"
#include "ValueFormatter.h"
#include "StyleStorage.h"


using namespace Calligra::Sheets;

Cell::Cell()
{
    cs = nullptr;
}

Cell::Cell(Sheet* sheet, int col, int row)
        : CellBase(sheet, col, row)
{
    cs = sheet ? sheet->fullCellStorage() : nullptr;
}

Cell::Cell(Sheet* sheet, const QPoint& pos)
        : CellBase(sheet, pos)
{
    cs = sheet ? sheet->fullCellStorage() : nullptr;
}

Cell::Cell(const Cell& other)
        : CellBase(other)
{
    cs = fullSheet()->fullCellStorage();
}

Cell::~Cell()
{
}

Cell& Cell::operator=(const Cell& other)
{
    CellBase::operator=(other);
    return *this;
}

// Return the sheet that this cell belongs to.
Sheet* Cell::fullSheet() const
{
    Q_ASSERT(!isNull());
    return dynamic_cast<Sheet *>(sheet());
}

Localization* Cell::locale() const
{
    return sheet()->map()->calculationSettings()->locale();
}

// Return true if this is the default cell.
bool Cell::isDefault() const
{
    // check each stored attribute
    if (!value().isEmpty())
        return false;
    if (formula() != Formula::empty())
        return false;
    if (!link().isEmpty())
        return false;
    if (doesMergeCells() == true)
        return false;
    if (!style().isDefault())
        return false;
    if (!comment().isEmpty())
        return false;
    if (!conditions().isEmpty())
        return false;
    if (!validity().isEmpty())
        return false;
    return true;
}

// Return true if this is the default cell (apart from maybe a custom style).
bool Cell::hasDefaultContent() const
{
    // check each stored attribute
    if (value() != Value())
        return false;
    if (formula() != Formula::empty())
        return false;
    if (!link().isEmpty())
        return false;
    if (doesMergeCells() == true)
        return false;
    if (!comment().isEmpty())
        return false;
    if (!conditions().isEmpty())
        return false;
    if (!validity().isEmpty())
        return false;
    return true;
}

Conditions Cell::conditions() const
{
    return cs->conditions(column(), row());
}

void Cell::setConditions(const Conditions& conditions)
{
    cs->setConditions(Region(cellPosition()), conditions);
}

Database Cell::database() const
{
    return cs->database(column(), row());
}

Style Cell::style() const
{
    return cs->style(column(), row());
}

Style Cell::effectiveStyle() const
{
    Style style = cs->style(column(), row());
    // use conditional formatting attributes
    const Style conditionalStyle = conditions().testConditions(*this);
    if (!conditionalStyle.isEmpty()) {
        style.merge(conditionalStyle);
    }
    return style;
}

void Cell::setStyle(const Style& style)
{
    cs->setStyle(Region(cellPosition()), style);
    cs->styleStorage()->contains(cellPosition());
}




void Cell::setUserInput(const QString& string)
{
    QString old = userInput();

    CellBase::setUserInput(string);

    if (old != string)
        // remove any existing richtext
        setRichText(QSharedPointer<QTextDocument>());
}

// Overrides the parser to disable parsing if the style is set to string
Value Cell::parsedUserInput(const QString& text)
{
    if (style().formatType() == Format::Text)
        return Value(text);

    return CellBase::parsedUserInput(text);
}



// Return the out text, i.e. the text that is visible in the cells
// square when shown.  This could, for instance, be the calculated
// result of a formula.
//
QString Cell::displayText(const Style& s, Value *v, bool *showFormula) const
{
    if (isNull())
        return QString();

    QString string;
    const Style style = s.isEmpty() ? effectiveStyle() : s;
    // Display a formula if warranted.  If not, display the value instead;
    // this is the most common case.
    if ( isFormula() && !(fullSheet()->isProtected() && style.hideFormula()) &&
         ( (showFormula && *showFormula) || (!showFormula && fullSheet()->getShowFormula()) ) )
    {
        string = userInput();
        if (showFormula)
            *showFormula = true;
    } else if (!isEmpty()) {
        Value theValue = fullSheet()->fullMap()->formatter()->formatText(value(), style.formatType(), style.precision(),
                 style.floatFormat(), style.prefix(),
                 style.postfix(), style.currency().symbol(),
                 style.customFormat(), style.thousandsSep());
        if (v) *v = theValue;
        string = theValue.asString();
        if (showFormula)
            *showFormula = false;
    }
    return string;
}


QSharedPointer<QTextDocument> Cell::richText() const
{
    return cs->richText(column(), row());
}

void Cell::setRichText(QSharedPointer<QTextDocument> text)
{
    cs->setRichText(column(), row(), text);
}

// FIXME: Continue commenting and cleaning here (ingwa)


void Cell::copyFormat(const Cell& cell)
{
    Q_ASSERT(!isNull());   // trouble ahead...
    Q_ASSERT(!cell.isNull());
    Value value = this->value();
    value.setFormat(cell.value().format());
    sheet()->cellStorage()->setValue(column(), row(), value);
    if (!style().isDefault() || !cell.style().isDefault())
        setStyle(cell.style());
    if (!conditions().isEmpty() || !cell.conditions().isEmpty())
        setConditions(cell.conditions());
}

void Cell::copyAll(const Cell& cell)
{
    Q_ASSERT(!isNull());   // trouble ahead...
    Q_ASSERT(!cell.isNull());
    copyFormat(cell);
    copyContent(cell);
    if (!comment().isEmpty() || !cell.comment().isEmpty())
        setComment(cell.comment());
    if (!validity().isEmpty() || !cell.validity().isEmpty())
        setValidity(cell.validity());
}

void Cell::copyContent(const Cell& cell)
{
    Q_ASSERT(!isNull());   // trouble ahead...
    Q_ASSERT(!cell.isNull());
    if (cell.isFormula()) {
        // change all the references, e.g. from A1 to A3 if copying
        // from e.g. B2 to B4
        Formula formula(sheet(), *this);
        formula.setExpression(decodeFormula(cell.encodeFormula()));
        setFormula(formula);
    } else {
        // copy the user input
        sheet()->cellStorage()->setUserInput(column(), row(), cell.userInput());
    }
    // copy the value in both cases
    sheet()->cellStorage()->setValue(column(), row(), cell.value());
}

bool Cell::needsPrinting() const
{
    if (!userInput().trimmed().isEmpty())
        return true;
    if (!comment().trimmed().isEmpty())
        return true;

    const Style style = effectiveStyle();

    // Cell borders?
    if (style.hasAttribute(Style::TopPen) ||
            style.hasAttribute(Style::LeftPen) ||
            style.hasAttribute(Style::RightPen) ||
            style.hasAttribute(Style::BottomPen) ||
            style.hasAttribute(Style::FallDiagonalPen) ||
            style.hasAttribute(Style::GoUpDiagonalPen))
        return true;

    // Background color or brush?
    if (style.hasAttribute(Style::BackgroundBrush)) {
        QBrush brush = style.backgroundBrush();

        // Only brushes that are visible (ie. they have a brush style
        // and are not white) need to be drawn
        if ((brush.style() != Qt::NoBrush) &&
                (brush.color() != Qt::white || !brush.texture().isNull()))
            return true;
    }

    if (style.hasAttribute(Style::BackgroundColor)) {
        debugSheetsRender << "needsPrinting: Has background color";
        QColor backgroundColor = style.backgroundColor();

        // We don't need to print anything, if the background is white opaque or fully transparent.
        if (!(backgroundColor == Qt::white || backgroundColor.alpha() == 0))
            return true;
    }

    return false;
}


QString Cell::encodeFormula(bool fixedReferences) const
{
    if (!isFormula())
        return QString();

    QString result('=');
    const Tokens tokens = formula().tokens();
    for (int i = 0; i < tokens.count(); ++i) {
        const Token token = tokens[i];
        switch (token.type()) {
        case Token::Cell:
        case Token::Range: {
            if (sheet()->map()->namedAreaManager()->contains(token.text())) {
                result.append(token.text()); // simply keep the area name
                break;
            }
            const Region region = sheet()->map()->regionFromName(token.text(), sheet());
            // Actually, a contiguous region, but the fixation is needed
            Region::ConstIterator end = region.constEnd();
            for (Region::ConstIterator it = region.constBegin(); it != end; ++it) {
                if (!(*it)->isValid())
                    continue;
                if ((*it)->type() == Region::Element::Point) {
                    if ((*it)->sheet())
                        result.append((*it)->sheet()->sheetName() + '!');
                    const QPoint pos = (*it)->rect().topLeft();
                    if ((*it)->isColumnFixed())
                        result.append(QString("$%1").arg(pos.x()));
                    else if (fixedReferences)
                        result.append(QChar(0xA7) + QString("%1").arg(pos.x()));
                    else
                        result.append(QString("#%1").arg(pos.x() - (int)column()));
                    if ((*it)->isRowFixed())
                        result.append(QString("$%1#").arg(pos.y()));
                    else if (fixedReferences)
                        result.append(QChar(0xA7) + QString("%1#").arg(pos.y()));
                    else
                        result.append(QString("#%1#").arg(pos.y() - (int)row()));
                } else { // ((*it)->type() == Region::Range)
                    if ((*it)->sheet())
                        result.append((*it)->sheet()->sheetName() + '!');
                    QPoint pos = (*it)->rect().topLeft();
                    if ((*it)->isLeftFixed())
                        result.append(QString("$%1").arg(pos.x()));
                    else if (fixedReferences)
                        result.append(QChar(0xA7) + QString("%1").arg(pos.x()));
                    else
                        result.append(QString("#%1").arg(pos.x() - (int)column()));
                    if ((*it)->isTopFixed())
                        result.append(QString("$%1#").arg(pos.y()));
                    else if (fixedReferences)
                        result.append(QChar(0xA7) + QString("%1#").arg(pos.y()));
                    else
                        result.append(QString("#%1#").arg(pos.y() - (int)row()));
                    result.append(':');
                    pos = (*it)->rect().bottomRight();
                    if ((*it)->isRightFixed())
                        result.append(QString("$%1").arg(pos.x()));
                    else if (fixedReferences)
                        result.append(QChar(0xA7) + QString("%1").arg(pos.x()));
                    else
                        result.append(QString("#%1").arg(pos.x() - (int)column()));
                    if ((*it)->isBottomFixed())
                        result.append(QString("$%1#").arg(pos.y()));
                    else if (fixedReferences)
                        result.append(QChar(0xA7) + QString("%1#").arg(pos.y()));
                    else
                        result.append(QString("#%1#").arg(pos.y() - (int)row()));
                }
            }
            break;
        }
        default: {
            result.append(token.text());
            break;
        }
        }
    }
    //debugSheets << result;
    return result;
}

QString Cell::decodeFormula(const QString &_text) const
{
    QString erg;
    unsigned int pos = 0;
    const unsigned int length = _text.length();

    if (_text.isEmpty())
        return QString();

    while (pos < length) {
        if (_text[pos] == '"') {
            erg += _text[pos++];
            while (pos < length && _text[pos] != '"') {
                erg += _text[pos++];
                // Allow escaped double quotes (\")
                if (pos < length && _text[pos] == '\\' && _text[pos+1] == '"') {
                    erg += _text[pos++];
                    erg += _text[pos++];
                }
            }
            if (pos < length)
                erg += _text[pos++];
        } else if (_text[pos] == '#' || _text[pos] == '$' || _text[pos] == QChar(0xA7)) {
            bool abs1 = false;
            bool abs2 = false;
            bool era1 = false; // if 1st is relative but encoded absolutely
            bool era2 = false;

            QChar _t = _text[pos++];
            if (_t == '$')
                abs1 = true;
            else if (_t == QChar(0xA7))
                era1 = true;

            int col = 0;
            unsigned int oldPos = pos;
            while (pos < length && (_text[pos].isDigit() || _text[pos] == '-')) ++pos;
            if (pos != oldPos)
                col = _text.midRef(oldPos, pos - oldPos).toInt();
            if (!abs1 && !era1)
                col += column();
            // Skip '#' or '$'

            _t = _text[pos++];
            if (_t == '$')
                abs2 = true;
            else if (_t == QChar(0xA7))
                era2 = true;

            int _row = 0;
            oldPos = pos;
            while (pos < length && (_text[pos].isDigit() || _text[pos] == '-')) ++pos;
            if (pos != oldPos)
                _row = _text.midRef(oldPos, pos - oldPos).toInt();
            if (!abs2 && !era2)
                _row += row();
            // Skip '#' or '$'
            ++pos;
            if (_row < 1 || col < 1 || _row > KS_rowMax || col > KS_colMax) {
                debugSheetsODF << "Cell::decodeFormula: row or column out of range (col:" << col << " | row:" << _row << ')';
                erg += Value::errorREF().errorMessage();
            } else {
                if (abs1)
                    erg += '$';
                erg += Cell::columnName(col); //Get column text

                if (abs2)
                    erg += '$';
                erg += QString::number(_row);
            }
        } else
            erg += _text[pos++];
    }

    return erg;
}


// ----------------------------------------------------------------
//                          Formula handling


int Cell::effectiveAlignX() const
{
    const Style style = effectiveStyle();
    int align = style.halign();
    if (align == Style::HAlignUndefined) {
        //numbers should be right-aligned by default, as well as BiDi text
        if ((style.formatType() == Format::Text) || value().isString())
            align = (displayText().isRightToLeft()) ? Style::Right : Style::Left;
        else {
            Value val = value();
            while (val.isArray()) val = val.element(0, 0);
            if (val.isBoolean() || val.isNumber())
                align = Style::Right;
            else
                align = Style::Left;
        }
    }
    return align;
}

double Cell::width() const
{
    const int rightCol = column() + mergedXCells();
    return fullSheet()->columnFormats()->totalColWidth(column(), rightCol);
}

double Cell::height() const
{
    const int bottomRow = row() + mergedYCells();
    return fullSheet()->rowFormats()->totalRowHeight(row(), bottomRow);
}

QString Cell::link() const
{
    return cs->link(column(), row());
}

void Cell::setLink(const QString& link)
{
    cs->setLink(column(), row(), link);

    if (!link.isEmpty() && userInput().isEmpty())
        parseUserInput(link);
}

bool Cell::isDate() const
{
    const Format::Type t = style().formatType();
    return (Format::isDate(t) || ((t == Format::Generic) && (value().format() == Value::fmt_Date)));
}

bool Cell::isTime() const
{
    const Format::Type t = style().formatType();
    return (Format::isTime(t) || ((t == Format::Generic) && (value().format() == Value::fmt_Time)));
}

bool Cell::isText() const
{
    const Format::Type t = style().formatType();
    return t == Format::Text;
}

// Return true if this cell is part of a merged cell, but not the
// master cell.

bool Cell::isPartOfMerged() const
{
    return cs->isPartOfMerged(column(), row());
}

Cell Cell::masterCell() const
{
    return cs->masterCell(column(), row());
}

// Merge a number of cells, i.e. make this cell obscure a number of
// other cells.  If _x and _y == 0, then the merging is removed.
void Cell::mergeCells(int _col, int _row, int _x, int _y)
{
    cs->mergeCells(_col, _row, _x, _y);
}

bool Cell::doesMergeCells() const
{
    return cs->doesMergeCells(column(), row());
}

int Cell::mergedXCells() const
{
    return cs->mergedXCells(column(), row());
}

int Cell::mergedYCells() const
{
    return cs->mergedYCells(column(), row());
}


bool Cell::compareData(const Cell& other) const
{
    if (value() != other.value())
        return false;
    if (formula() != other.formula())
        return false;
    if (link() != other.link())
        return false;
    if (mergedXCells() != other.mergedXCells())
        return false;
    if (mergedYCells() != other.mergedYCells())
        return false;
    if (style() != other.style())
        return false;
    if (comment() != other.comment())
        return false;
    if (conditions() != other.conditions())
        return false;
    if (validity() != other.validity())
        return false;
    return true;
}


