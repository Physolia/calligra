/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999, 2000 Montel Laurent <montell@club-internet.fr>
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

#include <qprinter.h>

#include "kspread_dlg_angle.h"
#include "kspread_view.h"
#include "kspread_canvas.h"
#include "kspread_util.h"
#include "kspread_layout.h"
#include "kspread_table.h"
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>
#include <kbuttonbox.h>
//#include <iostream.h>
#include <kdebug.h>

KSpreadAngle::KSpreadAngle( KSpreadView* parent, const char* name,const QPoint &_marker)
	: QDialog( parent, name,TRUE )
{

  m_pView=parent;
  marker=_marker;
  setCaption( i18n("Change Angle" ));
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setMargin( 5 );
  lay1->setSpacing( 10 );
  KSpreadCell *cell = m_pView->activeTable()->cellAt( marker.x(), marker.y() );
  int size=cell->getAngle();
  m_pSize2=new KIntNumInput(size, this, 10);
  m_pSize2->setRange(-90, 90, 1);
  m_pSize2->setLabel(i18n("Angle"));
  lay1->addWidget(m_pSize2);

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  m_pOk = bb->addButton( i18n("OK") );
  m_pOk->setDefault( TRUE );
  m_pClose = bb->addButton( i18n( "Close" ) );
  bb->layout();
  lay1->addWidget( bb );
  lay1->activate();
  m_pSize2->setFocus();
  connect( m_pClose, SIGNAL( clicked() ), this, SLOT( slotClose() ) );
  connect( m_pOk, SIGNAL( clicked() ), this, SLOT( slotOk() ) );

}

void KSpreadAngle::slotOk()
{

m_pView->activeTable()->setSelectionAngle(marker,m_pSize2->value());
accept();
}

void KSpreadAngle::slotClose()
{

reject();
}


#include "kspread_dlg_angle.moc"
