/*
 *  brushtool.cc - part of KImageShop
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

#include "brushtool.h"
#include "brush.h"
#include "canvas.h"

BrushTool::BrushTool(Canvas *_canvas, brush *_brush) : Tool(_canvas)
{
  m_dragging = false;
  m_pBrush = _brush;
}

BrushTool::~BrushTool() {}

void BrushTool::setBrush(brush *_brush)
{
  m_pBrush = _brush;
}

void BrushTool::mousePress(const KImageShop::MouseEvent& e)
{
  m_dragging = true;
  QPoint pos(e.posX, e.posY);
  m_dragStart = pos;
  
  if (!m_pBrush)
    return;

  m_pCanvas->paintBrush(pos, m_pBrush);

  QRect updateRect(pos - m_pBrush->hotSpot(), m_pBrush->brushSize());
  m_pCanvas->compositeImage(updateRect);
  m_pCanvas->repaintAll(updateRect);
}

void BrushTool::mouseMove(const KImageShop::MouseEvent& e)
{
  if(m_dragging)
    {
      QPoint pos(e.posX, e.posY);
      m_dragStart = pos;
      
      if (!m_pBrush)
	return;
      
      m_pCanvas->paintBrush(pos, m_pBrush);
      
      QRect updateRect(pos - m_pBrush->hotSpot(), m_pBrush->brushSize());
      m_pCanvas->compositeImage(updateRect);
      m_pCanvas->repaintAll(updateRect);
    }
}

void BrushTool::mouseRelease(const KImageShop::MouseEvent& e)
{
  m_dragging = false;
}
