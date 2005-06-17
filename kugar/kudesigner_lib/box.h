/* This file is part of the KDE project
 Copyright (C) 2002-2004 Alexander Dymo <adymo@mksat.net>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
*/
#ifndef BOX_H
#define BOX_H

#include <list.h>
#include <property.h>

#include "commdefs.h"

using namespace KOProperty;

namespace Kudesigner
{

class Canvas;

class Box: public QCanvasRectangle
{
public:
    enum ResizeType {ResizeNothing = 0, ResizeLeft = 1, ResizeTop = 2, ResizeRight = 4, ResizeBottom = 8};

    Box( int x, int y, int width, int height, Canvas *canvas ) :
            QCanvasRectangle( x, y, width, height, ( QCanvas* ) canvas ), m_canvas( canvas )
    {
        setSelected( false );
    }
    virtual ~Box();

    virtual int rtti() const
    {
        return Rtti_Box;
    }
    virtual void draw( QPainter &painter );
    virtual QString getXml()
    {
        return "";
    }

    void scale( int scale );

    virtual int isInHolder( const QPoint )
    {
        return ResizeNothing;
    }
    virtual void drawHolders( QPainter & )
    {}

    virtual void updateGeomProps()
    {
        ;
    }

    PtrList props;

protected:
    void registerAs( int type );

    Canvas *m_canvas;
};

}

#endif
