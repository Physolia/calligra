/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SvgUtil.h"
#include "SvgGraphicContext.h"

#include <KarbonGlobal.h>

#include <KoUnit.h>

#include <QtCore/QString>
#include <QtCore/QRectF>
#include <QtCore/QStringList>

#include <math.h>

#define DPI 72.0

double SvgUtil::fromUserSpace(double value)
{
    return (value * DPI) / 90.0;
}

double SvgUtil::toUserSpace(double value)
{
    return (value * 90.0) / DPI;
}

double SvgUtil::toPercentage(QString s)
{
    if (s.endsWith('%'))
        return s.remove('%').toDouble();
    else
        return s.toDouble() * 100.0;
}

double SvgUtil::fromPercentage(QString s)
{
    if (s.endsWith('%'))
        return s.remove('%').toDouble() / 100.0;
    else
        return s.toDouble();
}

QPointF SvgUtil::objectToUserSpace(const QPointF &position, const QRectF &objectBound)
{
    qreal x = objectBound.left() + position.x() * objectBound.width();
    qreal y = objectBound.top() + position.y() * objectBound.height();
    return QPointF(x, y);
}

QSizeF SvgUtil::objectToUserSpace(const QSizeF &size, const QRectF &objectBound)
{
    qreal w = size.width() * objectBound.width();
    qreal h = size.height() * objectBound.height();
    return QSizeF(w, h);
}

QPointF SvgUtil::userSpaceToObject(const QPointF &position, const QRectF &objectBound)
{
    qreal x = 0.0;
    if (objectBound.width() != 0)
        x = (position.x() - objectBound.x()) / objectBound.width();
    qreal y = 0.0;
    if (objectBound.height() != 0)
        y = (position.y() - objectBound.y()) / objectBound.height();
    return QPointF(x, y);
}

QSizeF SvgUtil::userSpaceToObject(const QSizeF &size, const QRectF &objectBound)
{
    qreal w = objectBound.width() != 0 ? size.width() / objectBound.width() : 0.0;
    qreal h = objectBound.height() != 0 ? size.height() / objectBound.height() : 0.0;
    return QSizeF(w, h);
}

QTransform SvgUtil::parseTransform(const QString &transform)
{
    QTransform result;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transform.split(')', QString::SkipEmptyParts);
    QStringList::ConstIterator it = subtransforms.constBegin();
    QStringList::ConstIterator end = subtransforms.constEnd();
    for (; it != end; ++it) {
        QStringList subtransform = (*it).simplified().split('(', QString::SkipEmptyParts);
        if (subtransform.count() < 2)
            continue;
        
        subtransform[0] = subtransform[0].trimmed().toLower();
        subtransform[1] = subtransform[1].simplified();
        QRegExp reg("[,( ]");
        QStringList params = subtransform[1].split(reg, QString::SkipEmptyParts);
        
        if (subtransform[0].startsWith(';') || subtransform[0].startsWith(','))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);
        
        if (subtransform[0] == "rotate") {
            if (params.count() == 3) {
                double x = params[1].toDouble();
                double y = params[2].toDouble();
                
                result.translate(x, y);
                result.rotate(params[0].toDouble());
                result.translate(-x, -y);
            } else {
                result.rotate(params[0].toDouble());
            }
        } else if (subtransform[0] == "translate") {
            if (params.count() == 2) {
                result.translate(SvgUtil::fromUserSpace(params[0].toDouble()),
                                 SvgUtil::fromUserSpace(params[1].toDouble()));
            } else {   // Spec : if only one param given, assume 2nd param to be 0
                result.translate(SvgUtil::fromUserSpace(params[0].toDouble()) , 0);
            }
        } else if (subtransform[0] == "scale") {
            if (params.count() == 2) {
                result.scale(params[0].toDouble(), params[1].toDouble());
            } else {   // Spec : if only one param given, assume uniform scaling
                result.scale(params[0].toDouble(), params[0].toDouble());
            }
        } else if (subtransform[0].toLower() == "skewx") {
            result.shear(tan(params[0].toDouble() * KarbonGlobal::pi_180), 0.0F);
        } else if (subtransform[0].toLower() == "skewy") {
            result.shear(0.0F, tan(params[0].toDouble() * KarbonGlobal::pi_180));
        } else if (subtransform[0] == "matrix") {
            if (params.count() >= 6) {
                result.setMatrix(params[0].toDouble(), params[1].toDouble(), 0,
                                 params[2].toDouble(), params[3].toDouble(), 0,
                                 SvgUtil::fromUserSpace(params[4].toDouble()),
                                 SvgUtil::fromUserSpace(params[5].toDouble()), 1);
            }
        }
    }
    
    return result;
}

qreal SvgUtil::parseUnit(SvgGraphicsContext *gc, const QString &unit, bool horiz, bool vert, const QRectF &bbox)
{
    if (unit.isEmpty())
        return 0.0;
    QByteArray unitLatin1 = unit.toLatin1();
    // TODO : percentage?
    const char *start = unitLatin1.data();
    if (!start) {
        return 0.0;
    }
    double value = 0.0;
    const char *end = parseNumber(start, value);

    if (int(end - start) < unit.length()) {
        if (unit.right(2) == "px")
            value = SvgUtil::fromUserSpace(value);
        else if (unit.right(2) == "cm")
            value = CM_TO_POINT(value);
        else if (unit.right(2) == "pc")
            value = PI_TO_POINT(value);
        else if (unit.right(2) == "mm")
            value = MM_TO_POINT(value);
        else if (unit.right(2) == "in")
            value = INCH_TO_POINT(value);
        else if (unit.right(2) == "em")
            value = value * gc->font.pointSize();
        else if (unit.right(2) == "ex") {
            QFontMetrics metrics(gc->font);
            value = value * metrics.xHeight();
        } else if (unit.right(1) == "%") {
            if (horiz && vert)
                value = (value / 100.0) * (sqrt(pow(bbox.width(), 2) + pow(bbox.height(), 2)) / sqrt(2.0));
            else if (horiz)
                value = (value / 100.0) * bbox.width();
            else if (vert)
                value = (value / 100.0) * bbox.height();
        }
    } else {
        value = SvgUtil::fromUserSpace(value);
    }
    /*else
    {
        if( m_gc.top() )
        {
            if( horiz && vert )
                value *= sqrt( pow( m_gc.top()->matrix.m11(), 2 ) + pow( m_gc.top()->matrix.m22(), 2 ) ) / sqrt( 2.0 );
            else if( horiz )
                value /= m_gc.top()->matrix.m11();
            else if( vert )
                value /= m_gc.top()->matrix.m22();
        }
    }*/
    //value *= 90.0 / DPI;

    return value;
}

qreal SvgUtil::parseUnitX(SvgGraphicsContext *gc, const QString &unit)
{
    if (gc->forcePercentage) {
        return SvgUtil::fromPercentage(unit) * gc->currentBoundbox.width();
    } else {
        return SvgUtil::parseUnit(gc, unit, true, false, gc->currentBoundbox);
    }
}

qreal SvgUtil::parseUnitY(SvgGraphicsContext *gc, const QString &unit)
{
    if (gc->forcePercentage) {
        return SvgUtil::fromPercentage(unit) * gc->currentBoundbox.height();
    } else {
        return SvgUtil::parseUnit(gc, unit, false, true, gc->currentBoundbox);
    }
}

qreal SvgUtil::parseUnitXY(SvgGraphicsContext *gc, const QString &unit)
{
    if (gc->forcePercentage) {
        const qreal value = SvgUtil::fromPercentage(unit);
        return value * sqrt(pow(gc->currentBoundbox.width(), 2) + pow(gc->currentBoundbox.height(), 2)) / sqrt(2.0);
    } else {
        return SvgUtil::parseUnit(gc, unit, true, true, gc->currentBoundbox);
    }
}

const char * SvgUtil::parseNumber(const char *ptr, qreal &number)
{
    int integer, exponent;
    qreal decimal, frac;
    int sign, expsign;

    exponent = 0;
    integer = 0;
    frac = 1.0;
    decimal = 0;
    sign = 1;
    expsign = 1;

    // read the sign
    if (*ptr == '+') {
        ptr++;
    } else if (*ptr == '-') {
        ptr++;
        sign = -1;
    }

    // read the integer part
    while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
        integer = (integer * 10) + *(ptr++) - '0';
    if (*ptr == '.') { // read the decimals
        ptr++;
        while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
            decimal += (*(ptr++) - '0') * (frac *= 0.1);
    }

    if (*ptr == 'e' || *ptr == 'E') { // read the exponent part
        ptr++;

        // read the sign of the exponent
        if (*ptr == '+') {
            ptr++;
        } else if (*ptr == '-') {
            ptr++;
            expsign = -1;
        }

        exponent = 0;
        while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9') {
            exponent *= 10;
            exponent += *ptr - '0';
            ptr++;
        }
    }
    number = integer + decimal;
    number *= sign * pow((double)10, double(expsign * exponent));

    return ptr;
}
