/* -*- C++ -*-

  $Id$
  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by  
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdlib.h>
#include <iostream.h>
#include <math.h>
#include <assert.h>
#ifdef __FreeBSD__
#include <float.h>
#else
#include <values.h>
#endif
#include "GBezier.h"
#include "GBezier.moc"

#include <qpntarry.h>
#include <klocale.h>
#include <kapp.h>

#define DELTA 0.05


static bool bezier_segment_part_contains (int x0, int y0, int x1,int y1,
					  const Coord& pp) {
  float  m, n, yp, xp;
  int xh, yh;
  
  if(pp.x () <= QMAX(x0,x1) + 5 && pp.x() >= QMIN(x0,x1) - 5 &&
     pp.y () <= QMAX(y0,y1) + 5 && pp.y() >= QMIN(y0,y1) - 5) {
    
    if (x1 <= x0) {
      // swap the points
      xh = x1;
      yh = y1;
      x1 = x0;
      y1 = y0;
      x0 = xh;
      y0 = yh;
    }
    
    if (x0 - 5 <= pp.x () && pp.x () <= x1 + 5) {
      if (abs (int (x0 - x1)) < 5) {
	if ((y0 <= pp.y () && pp.y () <= y1) ||
	    (y1 <= pp.y () && pp.y () <= y0)) {
	  return true;
	}
      }
      else {
	// y = m * x + n;
	m = (float)(y1 - y0) / (float)(x1 - x0);
	n = (float)y0 - m * (float)x0;
	
	if (m > 1) {
	  xp = ((float) pp.y () - n) / m;
	  if (xp - 5 <= pp.x () && pp.x () <= xp + 5) {
	    return true;
	    
	  }
	}
	else {
	  yp = m * (float) pp.x () + n;
	  if (yp - 5 <= pp.y () && pp.y () <= yp + 5) {
	    return true;
	  }
	}
      }
    }
  }
  return false;
}

static bool bezier_segment_contains (const Coord& p0, const Coord& p1,
         const Coord& p2, const Coord& p3, const Coord& c)
{
  /** every bezier_segment contains 1/DELTA lines, we have to compute the
      lines
      to check later in bezier_segment_part_contains, if the Coord c on a
      line
  **/

  float th,th2,th3;
  float t,t2,t3;
  int x0,y0,x1,y1;
  
  x1 = (int)p0.x();
  y1 = (int)p0.y();
  for(t = 0; t < 1.01; t += DELTA) {
    x0 = x1;
    y0 = y1;
    th = 1 - t;
    t2= t*t;
    t3 = t2*t;
    th2 = th*th;
    th3 = th2*th;
    x1 = (int) (th3*  p0.x() +
		3*t*th2*p1.x() +
		3*t2*th*p2.x() +
		t3*p3.x());
    y1 = (int) (th3* p0.y() +
		3*t*th2*p1.y() +
		3*t2*th*p2.y() +
		t3*p3.y());
    if (bezier_segment_part_contains(x0, y0, x1, y1, c)) return true;
  }
  
  return false;
}

GBezier::GBezier () : GPolyline () {
  wSegment = 0;
}
  
GBezier::GBezier (const list<XmlAttribute>& attribs) : GPolyline (attribs) {
  wSegment = -1;
}

GBezier::GBezier (const GBezier& obj) : GPolyline (obj) {
  wSegment = -1;
}
  
void GBezier::setPoint (int idx, const Coord& p) {
  QWMatrix mi = tMatrix.invert ();
  Coord np = p.transform (mi);

  points.at (idx)->x (np.x ());
  points.at (idx)->y (np.y());
  if (! isEndPoint (idx))
    updateBasePoint (cPoint (idx));
  calcBoundingBox ();
  emit changed ();
}

void GBezier::movePoint (int idx, float dx, float dy) {
  float x = points.at (idx)->x ();
  float y = points.at (idx)->y ();
  float ndx, ndy;

  QWMatrix mi = tMatrix.invert ();
  ndx = dx * mi.m11 () + dy * mi.m21 ();
  ndy = dy * mi.m22 () + dx * mi.m12 ();

  points.at (idx)->x (x + ndx);
  points.at (idx)->y (y + ndy);
  if (isEndPoint (idx)) {
    points.at (idx - 1)->x (points.at (idx - 1)->x () + ndx);
    points.at (idx - 1)->y (points.at (idx - 1)->y () + ndy);
    points.at (idx + 1)->x (points.at (idx + 1)->x () + ndx);
    points.at (idx + 1)->y (points.at (idx + 1)->y () + ndy);
  }
  else {
    updateBasePoint (cPoint (idx));
  }
  calcBoundingBox ();
  emit changed ();
}

const char* GBezier::typeName () {
  return i18n ("Bezier curve");
}

void GBezier::draw (Painter& p, bool withBasePoints) {
  QPen pen (outlineInfo.color, (uint) outlineInfo.width, 
            outlineInfo.style);
  p.save ();

  p.setPen (pen);

  p.setWorldMatrix (tmpMatrix, true);
  unsigned int i, num = points.count ();
  for (i = 1; i + 3 < num; i += 3) {
    if (points.at (i + 1)->x () == MAXFLOAT ||
	points.at (i + 2)->x () == MAXFLOAT) {
      p.drawLine (points.at (i)->x (), points.at (i)->y (),
		  points.at (i + 3)->x (), points.at (i + 3)->y ());
    }
    else {
      p.drawBezier (points,i);
    }
  }
  p.restore ();
  if (sArrow != 0L) {
    Coord pp = points.at (1)->transform (tmpMatrix);
    sArrow->draw (p, pp, outlineInfo.color, 
		  outlineInfo.width, sAngle);
  }
  if (eArrow != 0L) {
    Coord pp = points.at (num - 2)->transform (tmpMatrix);
    eArrow->draw (p, pp, outlineInfo.color, 
		  outlineInfo.width, eAngle);
  }
  if (withBasePoints)
    drawHelpLines (p);
  if (wSegment != -1)
    drawHelpLinesForWorkingSegment (p);
}

void GBezier::drawHelpLines (Painter& p) {
  unsigned int i, num = points.count ();

  p.save ();
  for (i = 0; i < num; i++) {
    Coord c = points.at (i)->transform (tmpMatrix);
    int x = (int) c.x ();
    int y = (int) c.y ();
    if (isEndPoint (i))
      p.setPen (black);
    else
      p.setPen (blue);
    p.drawRect (x - 2, y - 2, 4, 4);
  }

  QPen pen (blue, 1, DotLine);
  p.setPen (pen);
  for (i = 0; i + 2 < num; i += 3) {
    if (points.at (i)->x () == MAXFLOAT ||
	points.at (i + 2)->x () == MAXFLOAT)
      continue;
    
    Coord c1 = points.at (i)->transform (tmpMatrix);
    Coord c2 = points.at (i + 2)->transform (tmpMatrix);
    p.drawLine (c1.x (), c1.y (), c2.x (), c2.y ());
  }
  p.restore ();
}

void GBezier::drawHelpLinesForWorkingSegment (Painter& p) {
  p.save ();

  QPen pen1 (blue, 1, DotLine);
  QPen pen2 (blue);

  for (int i = wSegment * 3; i <= (wSegment + 1) * 3; i += 3) {
    if (i + 2 >= (int) points.count () || 
	points.at (i)->x () == MAXFLOAT ||
	points.at (i + 2)->x () == MAXFLOAT) {
      return;
    }

    p.setPen (pen1);
    Coord c1 = points.at (i)->transform (tmpMatrix);
    Coord c2 = points.at (i + 2)->transform (tmpMatrix);
    p.drawLine (c1.x (), c1.y (), c2.x (), c2.y ());
    p.setPen (pen2);
    p.drawRect (c1.x () - 2, c1.y () - 2, 4, 4);
    p.drawRect (c2.x () - 2, c2.y () - 2, 4, 4);
  }
  p.restore ();
}

bool GBezier::contains (const Coord& p) {
  if (box.contains (p)) {
    QWMatrix mi = tMatrix.invert ();
    Coord pc = p.transform (mi);

    for (unsigned int i = 1; i + 3 < points.count (); i += 3) {
      // detect the containing curve segment
      Rect r;
      Coord p = *(points.at (i));
      r.left (p.x ());
      r.top (p.y ());
      r.right (p.x ());
      r.bottom (p.y ());

      for (unsigned int j = i + 1; j < i + 4; j++) {
	Coord pn = *(points.at (j));
	r.left (QMIN(pn.x (), r.left ()));
	r.top (QMIN(pn.y (), r.top ()));
	r.right (QMAX(pn.x (), r.right ()));
	r.bottom (QMAX(pn.y (), r.bottom ()));
      }
      if (r.contains (pc)) {
       	if (bezier_segment_contains (*(points.at (i)), *(points.at (i + 1)),
				     *(points.at (i + 2)), 
				     *(points.at (i + 3)), pc))
	return true;
      }
    }
  }
  return false;
}

GObject* GBezier::copy () {
  return new GBezier (*this);
}

void GBezier::initBasePoint (int idx) {
  Coord epoint = *(points.at (idx + 1));
  float dx = epoint.x ();
  float dy = epoint.y ();
  points.at (idx)->x (2 * dx - points.at (idx + 2)->x ());
  points.at (idx)->y (2 * dy - points.at (idx + 2)->y ());
  emit changed ();
}

void GBezier::updateBasePoint (int idx) {
  int eidx = (idx == 0 ? 1 : (cPoint (idx) < idx ? idx - 1 : idx + 1));
  if (idx < 0 || eidx < 0)
    return;

  Coord epoint = *(points.at (eidx));
  if (points.at (cPoint (idx))->x () == MAXFLOAT)
    return;

  float dx = epoint.x ();
  float dy = epoint.y ();
  points.at (idx)->x (2 * dx - points.at (cPoint (idx))->x ());
  points.at (idx)->y (2 * dy - points.at (cPoint (idx))->y ());
  emit changed ();
}

void GBezier::setWorkingSegment (int seg) {
  wSegment = seg;
  emit changed ();
}

void GBezier::calcBoundingBox () {
  // don't include first and last base point in bounding box computation
  Rect r;
  unsigned int num = points.count ();
  if (num > 1) {
    Coord p = points.at (1)->transform (tmpMatrix);

    r.left (p.x ());
    r.top (p.y ());
    r.right (p.x ());
    r.bottom (p.y ());
  }
  for (unsigned int i = 2; i < num - 1; i++) {
    Coord p = points.at (i)->transform (tmpMatrix);

    r.left (QMIN(p.x (), r.left ()));
    r.top (QMIN(p.y (), r.top ()));
    r.right (QMAX(p.x (), r.right ()));
    r.bottom (QMAX(p.y (), r.bottom ()));
  }

  if (sArrow != 0L) {
    Coord p1 = points.at (0)->transform (tmpMatrix);
    Coord p2 = points.at (2)->transform (tmpMatrix);
    sAngle = calcArrowAngle (p1, p2, 0);
  }
  if (eArrow != 0L) {
    Coord p1 = points.at (num - 3)->transform (tmpMatrix);
    Coord p2 = points.at (num - 1)->transform (tmpMatrix);
    eAngle = calcArrowAngle (p1, p2, 1);
  }

  updateBoundingBox (r);
}

int GBezier::cPoint (int idx) { 
  if (idx > 1)
    return idx + (isEndPoint (idx - 1) ? -2 : 2); 
  else
    return idx + (isEndPoint (idx + 1) ? 2 : -2); 
}

void GBezier::writeToPS (ostream& os) {
  GObject::writeToPS (os);
  os << points.at (1)->x () << ' ' << points.at (1)->y () << " moveto\n";
  unsigned int num = points.count ();
  for (unsigned int i = 2; i < num - 2; i += 3) {
    for (unsigned int j = 0; j < 3; j++) 
      os << points.at (i + j)->x () << ' '
         << points.at (i + j)->y () << ' ';
    os << "curveto\n";
  }
  os << "SOCol stroke\nsetmatrix\n";
}

void GBezier::writeToXml (XmlWriter& xml) {
  xml.startTag ("bezier", false);
  writePropertiesToXml (xml);
  xml.addAttribute ("arrow1", outlineInfo.startArrowId);
  xml.addAttribute ("arrow2", outlineInfo.endArrowId);
  xml.closeTag (false);

  for (QListIterator<Coord> it (points); it.current (); ++it) {
    xml.startTag ("point", false);
    xml.addAttribute ("x", it.current ()->x ());
    xml.addAttribute ("y", it.current ()->y ());
    xml.closeTag (true);
  }
  xml.endTag ();
}

bool GBezier::findNearestPoint (const Coord& p, float max_dist, 
				float& dist, int& pidx) {
  float dx, dy, d1, d2;
  pidx = -1;

  QWMatrix mi = tMatrix.invert ();
  Coord np = p.transform (mi);

  dx = points.at (1)->x () - np.x ();
  dy = points.at (1)->y () - np.y ();
  d1 = sqrt (dx * dx + dy * dy);

  dx = points.at (points.count () - 2)->x () - np.x ();
  dy = points.at (points.count () - 2)->y () - np.y ();
  d2 = sqrt (dx * dx + dy * dy);

  if (d1 < d2) {
    if (d1 < max_dist) {
      dist = d1;
      pidx = 1;
    }
  }
  else if (d2 < max_dist) {
    dist = d2;
    pidx = points.count () - 2;
  }
  return pidx >= 0;
}
