/*
 *  movetool.cc - part of KImageShop
 *
 *  Copyright (c) 1999 The KImageShop team (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "movetool.h"
#include "canvas.h"

MoveTool::MoveTool(Canvas *_canvas) : Tool(_canvas)
{
  m_dragging = false;
}

MoveTool::~MoveTool() {}

void MoveTool::mousePress(const KImageShop::MouseEvent& e)
{
  m_dragging = true;
  m_dragStart.setX(e.posX);
  m_dragStart.setY(e.posY);
} 

void MoveTool::mouseMove(const KImageShop::MouseEvent& e)
{
  if(m_dragging)
    {
      QPoint pos(e.posX, e.posY);
      QPoint dragSize = pos - m_dragStart;

      QRect updateRect(m_pCanvas->getCurrentLayer()->imageExtents());
      m_pCanvas->moveLayer(dragSize.x(), dragSize.y());
      updateRect=updateRect.unite(m_pCanvas->getCurrentLayer()->imageExtents());
      m_pCanvas->compositeImage(updateRect);

      m_dragStart = pos;

      m_pCanvas->repaintAll(updateRect);
    }
}

void MoveTool::mouseRelease(const KImageShop::MouseEvent& e)
{
  m_dragging = false;
}
