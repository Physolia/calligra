/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2001-2002 Igor Jansen (rm@kde.org)

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

#include "GOval.h"

#include <cmath>

#include <qdom.h>

#include <klocale.h>
#include <koVectorPath.h>
#include <koPainter.h>

#include "kontour_global.h"
#include "GPath.h"

GOval::GOval(double rx, double ry, Type aType, double sa, double a):
GObject()
{
  mRX = rx;
  mRY = ry;
  mType = aType;
  mStartAngle = sa;
  mAngle = a;
  calcBoundingBox();
}

GOval::GOval(const QDomElement &element):
GObject(element.namedItem("go").toElement())
{

  mRX = element.attribute("rx").toDouble();
  mRY = element.attribute("ry").toDouble();
  mType = static_cast<Type>(element.attribute("type").toInt());
  mStartAngle = element.attribute("sa").toDouble();
  mAngle = element.attribute("a").toDouble();
  calcBoundingBox();
}

GOval::GOval(const GOval &obj):
GObject(obj)
{
  mRX = obj.mRX;
  mRY = obj.mRY;
  mType = obj.mType;
  mStartAngle = obj.mStartAngle;
  mAngle = obj.mAngle;
  calcBoundingBox();
}

void GOval::type(Type t)
{
  mType = t;
}

GObject *GOval::copy() const
{
  return new GOval(*this);
}

QString GOval::typeName() const
{
  if(mRX == mRY)
    return i18n("Circle");
  else
    return i18n("Ellipse");
}

QDomElement GOval::writeToXml(QDomDocument &document)
{
  QDomElement oval = document.createElement("oval");
  oval.setAttribute("rx", mRX);
  oval.setAttribute("ry", mRY);
  oval.setAttribute("type", mType);
  oval.setAttribute("sa", mStartAngle);
  oval.setAttribute("a", mAngle);
  oval.appendChild(GObject::writeToXml(document));
  return oval;
}

void GOval::draw(KoPainter *p, const QWMatrix &m, bool withBasePoints, bool /*outline*/, bool)
{
  setPen(p);
  setBrush(p);
  KoVectorPath *v = KoVectorPath::arc(0.0, 0.0, mRX, mRY, mStartAngle, mAngle);
  v->transform(tmpMatrix * m);
  p->drawVectorPath(v);
  delete v;
  if(withBasePoints)
  {
    int x;
    int y;
    KoPoint c;
    c = segPoint[0].transform(tmpMatrix * m);
    x = static_cast<int>(c.x());
    y = static_cast<int>(c.y());
    drawNode(p, x, y, false);
    c = segPoint[1].transform(tmpMatrix * m);
    x = static_cast<int>(c.x());
    y = static_cast<int>(c.y());
    drawNode(p, x, y, false);
  }
}

void GOval::calcBoundingBox()
{
  KoPoint p1(-mRX, mRY);
  KoPoint p2(mRX, mRY);
  KoPoint p3(mRX, -mRY);
  KoPoint p4(-mRX, -mRY);
  mSBox = calcUntransformedBoundingBox(p1, p2, p3, p4);
  mBBox = mSBox;
  adjustBBox(mBBox);

  double x, y, angle;

  angle = mStartAngle * Kontour::pi / 180.0;
  x = mRX * cos(angle);
  y = mRY * sin(angle);
  segPoint[0].setX(x);
  segPoint[0].setY(y);

  angle = (mStartAngle + mAngle) * Kontour::pi / 180.0;
  x = mRX * cos(angle);
  y = mRY * sin(angle);
  segPoint[1].setX(x);
  segPoint[1].setY(y);
}

int GOval::getNeighbourPoint(const KoPoint &p, const double /*distance*/)
{
  for(int i = 0; i < 2; i++)
  {
    KoPoint c = segPoint[i].transform(tMatrix);
    if(c.isNear(p, Kontour::nearDistance))
      return i;
  }
  return -1;
}

void GOval::movePoint(int idx, double dx, double dy, bool)
{
  KoPoint c = segPoint[idx];
  c = c.transform(tmpMatrix);
  c.setX(c.x() + dx);
  c.setY(c.y() + dy);
  segPoint[idx] = c.transform(iMatrix);
  if(idx == 0)
  {
    double ea = mStartAngle + mAngle;
    mStartAngle = 180.0 * atan(segPoint[0].y() / segPoint[0].x()) / Kontour::pi;
    if(segPoint[0].x() < 0.0)
      mStartAngle += 180.0;
    if(mStartAngle < 0.0)
      mStartAngle += 360.0;
    mAngle = ea - mStartAngle;
    double angle = mStartAngle * Kontour::pi / 180.0;
    double x = mRX * cos(angle);
    double y = mRY * sin(angle);
    segPoint[0].setX(x);
    segPoint[0].setY(y);
    kdDebug() << "angle = " << mStartAngle << endl;
  }
  else
  {
  
  }
}

void GOval::removePoint(int /*idx*/)
{
}

bool GOval::contains(const KoPoint &p)
{
  QPoint pp = iMatrix.map(QPoint(static_cast<int>(p.x()), static_cast<int>(p.y())));
  double x = pp.x();
  if(-mRX <= x && x <= mRX)
  {
    double y = sqrt(mRY * mRY * (1.0 - (x * x) / (mRX * mRX)));
    if(-y <= pp.y() && pp.y() <= y)
      return true;
    else
      return false;
  }
  else
    return false;
}

GPath *GOval::convertToPath() const
{
  GPath *path = new GPath();
  path->beginTo(-1.0, 0.0);
  path->arcTo(-1.0, 1.0, 0.0, 1.0, 1.0);
  path->arcTo(1.0, 1.0, 1.0, 0.0, 1.0);
  path->arcTo(1.0, -1.0, 0.0, -1.0, 1.0);
  path->arcTo(-1.0, -1.0, -1.0, 0.0, 1.0);
  QWMatrix m;
  m.scale(mRX, mRY);
  path->matrix(m * matrix());
  path->style(style());
  return path;
}

bool GOval::isConvertible() const
{
  return true;
}

#include "GOval.moc"
