/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _STATETOOL_H_
#define _STATETOOL_H_

#include <KoTool.h>

class StateShape;

class StateTool : public KoTool 
{
    Q_OBJECT
  public:
    explicit StateTool(KoCanvasBase *canvas);
    ~StateTool();
    
    /// reimplemented
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented
    virtual void mousePressEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void mouseReleaseEvent( KoPointerEvent *event );

  private:
    StateShape* m_currentShape;
};

#endif
