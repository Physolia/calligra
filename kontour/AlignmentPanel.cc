/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2002 Igor Jansen (rm@kde.org)

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

#include "AlignmentPanel.h"

#include <klocale.h>

AlignmentPanel::AlignmentPanel(KontourView *aView, QWidget *parent, const char *name):
QDockWindow(QDockWindow::InDock, parent, name)
{
  mView = aView;

  mAlignmentPanel = new QWidget(this);
  setWidget(mAlignmentPanel);
  setCaption(i18n("Align"));
}

#include "AlignmentPanel.moc"
