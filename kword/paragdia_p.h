/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef paragdia_p_h
#define paragdia_p_h

// This file hides those definitions from "users" of paragdia.h
// to reduce compile-time dependencies.

#include <qgroupbox.h>
#include <koparagcounter.h>
#include <qspinbox.h>
class QWidget;
class QPainter;


/******************************************************************/
/* Class: KWSpinBox                                               */
/******************************************************************/
class KWSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    enum counterType{ NONE,NUM,ALPHAB_L,ALPHAB_U,ROM_NUM_L,ROM_NUM_U};

    KWSpinBox( int minValue, int maxValue, int step = 1,
           QWidget * parent = 0, const char * name = 0 );
    KWSpinBox( QWidget * parent = 0, const char * name = 0 );
    virtual ~KWSpinBox();
    virtual QString mapValueToText( int value );
    void setCounterType(counterType _type);
 private:
    counterType m_Etype;

};

/******************************************************************/
/* class KWPagePreview                                            */
/******************************************************************/
class KWPagePreview : public QGroupBox
{
    Q_OBJECT

public:
    KWPagePreview( QWidget*, const char* = 0L );
    ~KWPagePreview() {}

    void setLeft( double _left )
    { left = _left; repaint( false ); }
    void setRight( double _right )
    { right = _right; repaint( false ); }
    void setFirst( double _first )
    { first = _first; repaint( false ); }
    void setSpacing( double _spacing )
    { spacing = _spacing; repaint( false ); }
    void setBefore( double _before )
    { before = _before; repaint( false ); }
    void setAfter( double _after )
    { after = _after; repaint( false ); }

protected:
    void drawContents( QPainter* );

    double left, right, first, spacing, before, after;

};

/******************************************************************/
/* class KWPagePreview2                                           */
/******************************************************************/

class KWPagePreview2 : public QGroupBox
{
    Q_OBJECT

public:
    KWPagePreview2( QWidget*, const char* = 0L );
    ~KWPagePreview2() {}

    void setAlign( int _align )
    { align = _align; repaint( false ); }

protected:
    void drawContents( QPainter* );

    int align;

};

/******************************************************************/
/* class KWBorderPreview                                          */
/******************************************************************/

class KWBorderPreview : public QFrame/*QGroupBox*/
{
    Q_OBJECT

public:
    KWBorderPreview( QWidget*, const char* = 0L );
    ~KWBorderPreview() {}

    KoBorder leftBorder() { return m_leftBorder; }
    void setLeftBorder( KoBorder _leftBorder ) { m_leftBorder = _leftBorder; repaint( true ); }
    KoBorder rightBorder() { return m_rightBorder; }
    void setRightBorder( KoBorder _rightBorder ) { m_rightBorder = _rightBorder; repaint( true ); }
    KoBorder topBorder() { return m_topBorder; }
    void setTopBorder( KoBorder _topBorder ) { m_topBorder = _topBorder; repaint( true ); }
    KoBorder bottomBorder() { return m_bottomBorder; }
    void setBottomBorder( KoBorder _bottomBorder ) { m_bottomBorder = _bottomBorder; repaint( true ); }

protected:
    virtual void mousePressEvent( QMouseEvent* _ev );
    void drawContents( QPainter* );
    QPen setBorderPen( KoBorder _brd );

    KoBorder m_leftBorder, m_rightBorder, m_topBorder, m_bottomBorder;
signals:
    void choosearea(QMouseEvent * _ev);

};

/******************************************************************/
/* class KWNumPreview                                             */
/******************************************************************/
class KWTextDocument;
class KWNumPreview : public QGroupBox
{
    Q_OBJECT

public:
    KWNumPreview( QWidget*, const char* = 0L );
    ~KWNumPreview();

    void setCounter( const KoParagCounter & counter );
    void setStyle(KoStyle *style);

protected:
    void drawContents( QPainter* );

    KWTextDocument *m_textdoc;
    KoZoomHandler *m_zoomHandler;
};

#endif
