/* This file is part of the KDE project
   Copyright (C) 2002 Ariya Hidayat <ariya@kde.org>

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

#include "transeffectdia.h"

#include "global.h"
#include "kpbackground.h"
#include "kpobject.h"
#include "kptextobject.h"
#include "kpresenter_view.h"
#include "kprcanvas.h"
#include "kppageeffects.h"
#include "kpresenter_sound_player.h"

#include <qsplitter.h>
#include <qheader.h>
#include <qwmatrix.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qframe.h>

#include <kdebug.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

KPEffectPreview::KPEffectPreview( QWidget *parent, KPresenterDoc *_doc, KPresenterView *_view )
    : QLabel( parent ), doc( _doc ), view( _view )
{
    setFrameStyle( StyledPanel | Sunken );
}

void KPEffectPreview::setPixmap( const QPixmap& pixmap )
{
    // find the right size
    QRect rect = pixmap.rect();
    int w = rect.width();
    int h = rect.height();
    if ( w > h ) {
        w = 297;
        h = 210;
    }
    else if ( w < h ) {
        w = 210;
        h = 297;
    }
    else if ( w == h ) {
        w = 297;
        h = 297;
    }

    setMinimumSize( w, h );

    // create smaller pixmap for preview
    const QImage img( pixmap.convertToImage().smoothScale( w, h, QImage::ScaleFree ) );
    m_pixmap.convertFromImage( img );

    QLabel::setPixmap( m_pixmap );
}

void KPEffectPreview::run( PageEffect effect, PresSpeed speed )
{
    QRect rect = m_pixmap.rect();
    QPixmap target( rect.size() );
    target.fill( Qt::black );

    kPchangePages( this, m_pixmap, target, effect, speed );

    QLabel::update();
}


KPTransEffectDia::KPTransEffectDia( QWidget *parent, const char *name,
                                    KPresenterDoc *_doc, KPresenterView *_view )
    : KDialogBase( parent, name, true, "", Ok|Cancel ),
      doc( _doc ), view( _view ), soundPlayer( 0 )
{
    enableButtonSeparator( true );

    QWidget *page = new QWidget( this );
    setMainWidget(page);

    QBoxLayout *topLayout = new QHBoxLayout( page, KDialog::marginHint(), KDialog::spacingHint() );
    QWidget* leftpart = new QWidget( page );
    topLayout->addWidget( leftpart );
    QWidget* rightpart = new QWidget( page );
    topLayout->addWidget( rightpart );

    // right-side of the dialog, for showing effect preview

    QVBoxLayout *rightlayout = new QVBoxLayout( rightpart, KDialog::marginHint(), KDialog::spacingHint() );
    rightlayout->setAutoAdd( true );

    effectPreview = new KPEffectPreview( rightpart, doc, view );

    int pgnum = view->getCurrPgNum() - 1;  // getCurrPgNum() is 1-based
    KPrPage* pg = doc->pageList().at( pgnum );

    // pixmap for effect preview
    QRect rect= pg->getZoomPageRect();
    QPixmap pix( rect.size() );
    pix.fill( Qt::white );
    view->getCanvas()->drawPageInPix( pix, pgnum, 100 );
    effectPreview->setPixmap( pix );

    pageEffect = pg->getPageEffect();
    speed = doc->getPresSpeed();

    QVBoxLayout *leftlayout = new QVBoxLayout( leftpart, KDialog::marginHint(), KDialog::spacingHint() );
    leftlayout->setAutoAdd( true );

    new QLabel( i18n("Effect:"), leftpart );

    effectList = new QListBox( leftpart );
    effectList->insertItem( i18n( "No Effect" ) );
    effectList->insertItem( i18n( "Close Horizontal" ) );
    effectList->insertItem( i18n( "Close Vertical" ) );
    effectList->insertItem( i18n( "Close From All Directions" ) );
    effectList->insertItem( i18n( "Open Horizontal" ) );
    effectList->insertItem( i18n( "Open Vertical" ) );
    effectList->insertItem( i18n( "Open From All Directions" ) );
    effectList->insertItem( i18n( "Interlocking Horizontal 1" ) );
    effectList->insertItem( i18n( "Interlocking Horizontal 2" ) );
    effectList->insertItem( i18n( "Interlocking Vertical 1" ) );
    effectList->insertItem( i18n( "Interlocking Vertical 2" ) );
    effectList->insertItem( i18n( "Surround 1" ) );
    effectList->insertItem( i18n( "Fly Away 1" ) );
    effectList->insertItem( i18n( "Blinds Horizontal" ) );
    effectList->insertItem( i18n( "Blinds Vertical" ) );
    effectList->insertItem( i18n( "Box In" ) );
    effectList->insertItem( i18n( "Box Out" ) );
    effectList->insertItem( i18n( "Checkerboard Across" ) );
    effectList->insertItem( i18n( "Checkerboard Down" ) );
    effectList->insertItem( i18n( "Cover Down" ) );
    effectList->insertItem( i18n( "Uncover Down" ) );
    effectList->insertItem( i18n( "Cover Up" ) );
    effectList->insertItem( i18n( "Uncover Up" ) );
    effectList->insertItem( i18n( "Cover Left" ) );
    effectList->insertItem( i18n( "Uncover Left" ) );
    effectList->insertItem( i18n( "Cover Right" ) );
    effectList->insertItem( i18n( "Uncover Right" ) );
    effectList->insertItem( i18n( "Cover Left-Up" ) );
    effectList->insertItem( i18n( "Uncover Left-Up" ) );
    effectList->insertItem( i18n( "Cover Left-Down" ) );
    effectList->insertItem( i18n( "Uncover Left-Down" ) );
    effectList->insertItem( i18n( "Cover Right-Up" ) );
    effectList->insertItem( i18n( "Uncover Right-Up" ) );
    effectList->insertItem( i18n( "Cover Right-Bottom" ) );
    effectList->insertItem( i18n( "Uncover Right-Bottom" ) );
    effectList->insertItem( i18n( "Dissolve" ) );
    effectList->insertItem( i18n( "Strips Left-Up" ) );
    effectList->insertItem( i18n( "Strips Left-Down" ) );
    effectList->insertItem( i18n( "Strips Right-Up" ) );
    effectList->insertItem( i18n( "Strips Right-Down" ) );
    effectList->insertItem( i18n( "Melting" ) );
    effectList->insertItem( i18n( "Random Transition" ) );
    effectList->setCurrentItem( static_cast<int>( pageEffect ) );

    // workaround, because Random Effect is always negative
    if( pageEffect == PEF_RANDOM )
        effectList->setCurrentItem( effectList->count()-1 );

    connect( effectList, SIGNAL(highlighted(int)), this, SLOT(effectChanged(int)) );

    new QLabel( i18n("Speed:"), leftpart );

    QWidget* sp = new QWidget( leftpart );
    QBoxLayout* speedLayout = new QHBoxLayout( sp, KDialog::marginHint(), KDialog::spacingHint() );
    speedLayout->setAutoAdd( true );

    new QLabel( i18n("Slow"), sp );
    speedSlider = new QSlider( 1, 10, 1, 1, Qt::Horizontal, sp );
    speedSlider->setValue( speed );
    speedSlider->setTickmarks( QSlider::Below );
    speedSlider->setTickInterval( 1 );
    new QLabel( i18n("Fast"), sp );

    connect( speedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)) );


    QWidget* previewgrp = new QWidget( leftpart );
    QBoxLayout* previewLayout = new QHBoxLayout( previewgrp, KDialog::marginHint(), KDialog::spacingHint() );
    previewLayout->setAutoAdd( true );

    automaticPreview = new QCheckBox( i18n( "Automatic preview" ), previewgrp );
    automaticPreview->setChecked( true );

    QWidget* previewspacer = new QWidget( previewgrp );
    previewspacer->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                               QSizePolicy::Expanding ) );

    previewButton = new QPushButton( previewgrp );
    previewButton->setText( i18n("Preview") );
    connect( previewButton, SIGNAL(clicked()), this, SLOT(preview()) );

    QFrame* line = new QFrame( leftpart );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    soundFileName = pg->getPageSoundFileName();
    soundEffect = pg->getPageSoundEffect();

    checkSoundEffect = new QCheckBox( i18n( "Sound effect" ), leftpart );
    checkSoundEffect->setChecked( soundEffect );
    QWhatsThis::add( checkSoundEffect, i18n("If you use a sound effect, please do not select 'No Effect'.") );
    connect( checkSoundEffect, SIGNAL( clicked() ), this, SLOT( soundEffectChanged() ) );

    QWidget* soundgrp = new QWidget( leftpart );
    QBoxLayout* soundLayout = new QHBoxLayout( soundgrp, KDialog::marginHint(), KDialog::spacingHint() );
    soundLayout->setAutoAdd( true );

    lSoundEffect = new QLabel( i18n( "File name:" ), soundgrp );
    requester = new KURLRequester( soundgrp );
    requester->setURL( soundFileName );
    connect( requester, SIGNAL( openFileDialog( KURLRequester * ) ),
             this, SLOT( slotRequesterClicked( KURLRequester * ) ) );
    connect( requester, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( slotSoundFileChanged( const QString& ) ) );

    buttonTestPlaySoundEffect = new QPushButton( soundgrp );
    buttonTestPlaySoundEffect->setPixmap( BarIcon("1rightarrow", KIcon::SizeSmall) );
    QToolTip::add( buttonTestPlaySoundEffect, i18n("Play") );

    connect( buttonTestPlaySoundEffect, SIGNAL( clicked() ), this, SLOT( playSound() ) );

    buttonTestStopSoundEffect = new QPushButton( soundgrp );
    buttonTestStopSoundEffect->setPixmap( BarIcon("player_stop", KIcon::SizeSmall) );
    QToolTip::add( buttonTestStopSoundEffect, i18n("Stop") );

    connect( buttonTestStopSoundEffect, SIGNAL( clicked() ), this, SLOT( stopSound() ) );

    soundEffect = pg->getPageSoundEffect();
    connect( this, SIGNAL( okClicked() ), this, SLOT( slotTransEffectDiaOK() ) );
    //connect( this, SIGNAL( okClicked() ), this, SLOT( accept() ) );

    slideTime = pg->getPageTimer();

    new QLabel( i18n("Automatically advance to the next slide after:"), rightpart );

    timeSlider = new KIntNumInput( slideTime, rightpart );
    timeSlider->setRange( 1, 600, 1 );
    timeSlider->setSuffix( i18n( " seconds" ) );
    connect( timeSlider, SIGNAL(valueChanged(int)), this, SLOT(timeChanged(int)) );

    QWidget* rspacer = new QWidget( rightpart );
    rspacer->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );

    QWidget* lspacer = new QWidget( leftpart );
    lspacer->setMinimumSize( 10, spacingHint() );

    soundEffectChanged();
}

void KPTransEffectDia::preview()
{
    if( pageEffect==PEF_NONE)
        return;
    effectPreview->run( pageEffect, speed );
}

void KPTransEffectDia::effectChanged( int index )
{
    if( effectList->currentText() == i18n( "Random Transition" ) )
        pageEffect = PEF_RANDOM;
    else
        pageEffect =  static_cast<PageEffect>( index );

    if( automaticPreview->isChecked() ) preview();
}

void KPTransEffectDia::speedChanged( int value )
{
    if( value <= 0 ) value = 1;
    speed = static_cast<PresSpeed>(value);
}

void KPTransEffectDia::timeChanged( int value )
{
    if( value <= 0 ) value = 1;
    slideTime = value;
}

void KPTransEffectDia::soundEffectChanged()
{
    soundEffect = checkSoundEffect->isChecked();

    lSoundEffect->setEnabled( checkSoundEffect->isChecked() );
    requester->setEnabled( checkSoundEffect->isChecked() );

    if ( !requester->url().isEmpty() ) {
        buttonTestPlaySoundEffect->setEnabled( checkSoundEffect->isChecked() );
        buttonTestStopSoundEffect->setEnabled( checkSoundEffect->isChecked() );
    }
    else {
        buttonTestPlaySoundEffect->setEnabled( false );
        buttonTestStopSoundEffect->setEnabled( false );
    }
}

static QString getSoundFileFilter()
{
    QStringList fileList;
    fileList << "wav" << "au" << "mp3" << "mp1" << "mp2" << "mpg" << "dat"
             << "mpeg" << "ogg" << "cdda" << "cda " << "vcd" << "null";
    fileList.sort();

    bool comma = false;
    QString full, str;
    for ( QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it ) {
        if ( comma )
            str += '\n';
        comma = true;
        str += QString( i18n( "*.%1|%2 Files" ) ).arg( *it ).arg( (*it).upper() );

        full += QString( "*.") + (*it) + ' ';
    }

    str = full + '|' + i18n( "All Supported Files" ) + '\n' + str;
    str += "\n*|" + i18n( "All Files" );

    return str;
}

void KPTransEffectDia::slotRequesterClicked( KURLRequester * )
{
    QString filter = getSoundFileFilter();
    requester->fileDialog()->setFilter( filter );

    // find the first "sound"-resource that contains files
    QStringList soundDirs = KGlobal::dirs()->resourceDirs( "sound" );
    if ( !soundDirs.isEmpty() ) {
        KURL soundURL;
        QDir dir;
        dir.setFilter( QDir::Files | QDir::Readable );
        QStringList::ConstIterator it = soundDirs.begin();
        while ( it != soundDirs.end() ) {
            dir = *it;
            if ( dir.isReadable() && dir.count() > 2 ) {
                soundURL.setPath( *it );
                requester->fileDialog()->setURL( soundURL );
                break;
            }
            ++it;
        }
    }
}

void KPTransEffectDia::slotSoundFileChanged( const QString& text )
{
    soundFileName = text;

    buttonTestPlaySoundEffect->setEnabled( !text.isEmpty() );
    buttonTestStopSoundEffect->setEnabled( !text.isEmpty() );
}

void KPTransEffectDia::playSound()
{
    delete soundPlayer;
    soundPlayer = new KPresenterSoundPlayer( requester->url() );
    soundPlayer->play();

    buttonTestPlaySoundEffect->setEnabled( false );
    buttonTestStopSoundEffect->setEnabled( true );
}

void KPTransEffectDia::stopSound()
{
    if ( soundPlayer ) {
        soundPlayer->stop();
        delete soundPlayer;
        soundPlayer = 0;

        buttonTestPlaySoundEffect->setEnabled( true );
        buttonTestStopSoundEffect->setEnabled( false );
    }
}

#include "transeffectdia.moc"
