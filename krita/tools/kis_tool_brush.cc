/*
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qpainter.h>
#include <qpen.h>
#include <qcolor.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <koColor.h>

#include "kis_vec.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_tool_brush.h"
#include "kis_tool_paint.h"
#include "kis_layer.h"
#include "kis_alpha_mask.h"
#include "kis_cursor.h"

KisToolBrush::KisToolBrush()
        : super(),
          m_mode( HOVER ),
	  m_dragDist ( 0 ),
	  m_usePattern ( false ),
	  m_useGradient ( false )
{
#if 0 // until we got a decent cursor
	setCursor(KisCursor::crossCursor());
#endif
        m_painter = 0;
	m_currentImage = 0;
}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}

void KisToolBrush::mousePress(QMouseEvent *e)
{
        if (!m_subject) return;

        if (!m_subject->currentBrush()) return;

        if (e->button() == QMouseEvent::LeftButton) {
		m_mode = PAINT;
		initPaint(e -> pos());
		m_painter -> paintAt(e->pos(), 128, 0, 0);
		// XXX: get the rect that should be notified
		m_currentImage -> notify( m_painter -> dirtyRect() );
         }
}


void KisToolBrush::mouseRelease(QMouseEvent* e)
{
	if (e->button() == QMouseEvent::LeftButton && m_mode == PAINT) {
		endPaint();
        }
}


void KisToolBrush::mouseMove(QMouseEvent *e)
{
	if (m_mode == PAINT) {
		paintLine(m_dragStart, e -> pos(), 128, 0, 0);
	}
}

void KisToolBrush::tabletEvent(QTabletEvent *e)
{
         if (e->device() == QTabletEvent::Stylus) {
		 if (!m_subject) {
			 e -> accept();
			 return;
		 }

		 if (!m_subject -> currentBrush()) {
			 e->accept();
			 return;
		 }

		 Q_INT32 pressure = e -> pressure();

		 if (pressure < 5 && m_mode == PAINT_STYLUS) {
			 endPaint();
		 }
		 else if (pressure >= 5 && m_mode == HOVER) {
			 m_mode = PAINT_STYLUS;
			 initPaint(e -> pos());
			 m_painter -> paintAt(e -> pos(), e->pressure(), e->xTilt(), e->yTilt());
			 // XXX: Get the rect that should be updated
			 m_currentImage -> notify( m_painter -> dirtyRect() );

		 }
		 else if (pressure >= 5 && m_mode == PAINT_STYLUS) {
			 paintLine(m_dragStart, e -> pos(), pressure, e -> xTilt(), e -> yTilt());
		 }
         }
	 e -> accept();
}


void KisToolBrush::initPaint(const QPoint & pos)
{
	m_dragStart = pos;
	m_dragDist = 0;

	// Create painter
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		if (m_painter)
			delete m_painter;
		m_painter = new KisPainter( device );
		m_painter -> beginTransaction(i18n("brush"));
	}

	// First set colour, then brush, otherwise dab won't be computed with the
	// right colour. This will be fixed when we no longer compute dabs but
	// composite the mask directly onto the layer with the currently set colour.
	m_painter -> setPaintColor(m_subject -> fgColor());
	m_painter -> setBrush(m_subject -> currentBrush());

	// Set the cursor -- ideally. this should be a pixmap created from the brush,
	// now that X11 can handle colored cursors.
#if 0
	// Setting cursors has no effect until the tool is selected again; this
	// should be fixed.
	setCursor(KisCursor::brushCursor());
#endif

}

void KisToolBrush::endPaint() 
{
	m_mode = HOVER;
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
		if (adapter && m_painter) {
			// If painting in mouse release, make sure painter
			// is destructed or end()ed
			adapter -> addCommand(m_painter->endTransaction());
		}
		delete m_painter;
		m_painter = 0;

	}
}

void KisToolBrush::paintLine(const QPoint & pos1,
			     const QPoint & pos2,
			     const Q_INT32 pressure,
			     const Q_INT32 xtilt,
			     const Q_INT32 ytilt)
{

	m_dragDist = m_painter -> paintLine(pos1, pos2, pressure, xtilt, ytilt, m_dragDist);
	m_currentImage -> notify( m_painter -> dirtyRect() );
	m_dragStart = pos2;
}


void KisToolBrush::setup(KActionCollection *collection)
{
        KToggleAction *toggle;
        toggle = new KToggleAction(i18n("&Brush"),
				   "handdrawn", 0, this,
                                   SLOT(activate()), collection,
                                   "tool_brush");
        toggle -> setExclusiveGroup("tools");
}
