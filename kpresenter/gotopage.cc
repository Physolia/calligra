/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <gotopage.h>

#include <qlabel.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kdialog.h>

#include <kpresenter_doc.h>

/******************************************************************/
/* class KPGotoPage						  */
/******************************************************************/

/*================================================================*/
KPGotoPage::KPGotoPage( const KPresenterDoc *doc,
			const QValueList<int> &slides, int start,
			QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, i18n("Goto Page..."), Ok|Cancel)
    , oldPage(start)
{

    QWidget *page = new QWidget( this );
    setMainWidget(page);
    QVBoxLayout *ml = new QVBoxLayout( page, KDialog::marginHint(),
                                         KDialog::spacingHint());
    QLabel *label = new QLabel( i18n( "Goto Page:" ), page );
    ml->addWidget( label );
    spinbox = new QListBox( page );
    connect( spinbox, SIGNAL(doubleClicked( QListBoxItem* )),
	     this, SLOT(accept()) );
    connect( spinbox, SIGNAL(returnPressed( QListBoxItem* )),
	     this, SLOT(accept()) );
    ml->addWidget( spinbox );

    QPtrList<KPrPage> pageList = doc->getPageList(); // because of const doc, we can't do doc->pageList()->at()
    QValueList<int>::ConstIterator it = slides.begin();
    for ( ; it != slides.end(); ++it ) {
        QString t(pageList.at(*it)->pageTitle( i18n( "Slide %1" ).arg( *it ) ));
        // cut ultra long titles...
        if(t.length() > 30) {
            t.truncate(30);
            t+=i18n("...");
        }
        spinbox->insertItem( QString( "%1 - %2" ).arg( *it ).arg( t ), -1 );
        if ( *it == start )
            spinbox->setCurrentItem( spinbox->count()-1 );
    }

    if ( parent )
        parent->setCursor( Qt::forbiddenCursor );
}

/*================================================================*/
int KPGotoPage::gotoPage( const KPresenterDoc *doc,
			  const QValueList<int> &slides, int start,
			  QWidget *parent)
{
    KPGotoPage dia( doc, slides, start,parent, 0L );
    dia.exec();
    dia.resetCursor();
    return dia.page();
}

int KPGotoPage::page() const {
    if(result()==QDialog::Accepted)
        return spinbox->currentText().left( spinbox->currentText().find( "-" ) - 1 ).toInt();
    return oldPage;
}

void KPGotoPage::resetCursor() {
    if ( parentWidget() )
        parentWidget()->setCursor( Qt::blankCursor );
}
