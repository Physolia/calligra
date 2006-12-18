/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KPrView.h"

#include <QGridLayout>
#include <QToolBar>
#include <QScrollBar>
#include <QTimer>

#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoZoomHandler.h>
#include <KoToolBoxFactory.h>
#include <KoShapeSelectorFactory.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoZoomAction.h>
#include <KoTextSelectionHandler.h>

#include <KPrCanvas.h>
#include <KPrDocument.h>
#include <KPrPage.h>

#include <klocale.h>
#include <ktoggleaction.h>

KPrView::KPrView( KPrDocument *document, QWidget *parent )
: KoView( document, parent )
, m_doc( document )
{
    m_activePage = 0;
    initGUI();
    initActions();
    if(m_doc->pageCount() > 0)
        setActivePage(m_doc->pageByIndex(0));
}

KPrView::~KPrView()
{
    KoToolManager::instance()->removeCanvasController( m_canvasController );
}


KPrPage* KPrView::activePage() const
{
    return m_activePage;
}

void KPrView::updateReadWrite( bool readwrite )
{
}

void KPrView::initGUI()
{
    QGridLayout * gridLayout = new QGridLayout( this );
    gridLayout->setMargin( 0 );
    gridLayout->setSpacing( 0 );
    setLayout( gridLayout );

    m_canvas = new KPrCanvas( this, m_doc );
    m_canvasController = new KoCanvasController( this );
    m_canvasController->setCanvas( m_canvas );
    KoToolManager::instance()->addControllers( m_canvasController );

    //Ruler
    m_horizontalRuler = new KoRuler(this, Qt::Horizontal, viewConverter());
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->setUnit(m_doc->unit());
    m_verticalRuler = new KoRuler(this, Qt::Vertical, viewConverter());
    m_verticalRuler->setUnit(m_doc->unit());
    m_verticalRuler->setShowMousePosition(true);

    connect(m_doc, SIGNAL(unitChanged(KoUnit)), m_horizontalRuler, SLOT(setUnit(KoUnit)));
    connect(m_doc, SIGNAL(unitChanged(KoUnit)), m_verticalRuler, SLOT(setUnit(KoUnit)));

    gridLayout->addWidget(m_horizontalRuler, 0, 1);
    gridLayout->addWidget(m_verticalRuler, 1, 0);
    gridLayout->addWidget( m_canvasController, 1, 1 );

    connect(m_canvasController, SIGNAL(canvasOffsetXChanged(int)),
            m_horizontalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(canvasOffsetYChanged(int)),
            m_verticalRuler, SLOT(setOffset(int)));

    KoToolBoxFactory toolBoxFactory( "Tools" );
    createDockWidget( &toolBoxFactory );
    KoShapeSelectorFactory shapeSelectorFactory;
    createDockWidget( &shapeSelectorFactory );
    show();
}

void KPrView::initActions()
{
    if ( !m_doc->isReadWrite() )
       setXMLFile( "kpresenter_readonly.rc" );
    else
       setXMLFile( "kpresenter.rc" );

    m_actionFormatBold = new KToggleAction(KIcon("text_bold"), i18n( "Bold" ), actionCollection(), "format_bold");
    m_actionFormatBold->setShortcut(KShortcut(Qt::CTRL + Qt::Key_B));
    connect( m_actionFormatBold, SIGNAL(toggled(bool)), this, SLOT(textBold(bool)) );

    m_actionFormatItalic = new KToggleAction(KIcon("text_italic"), i18n( "Italic" ), actionCollection(), "format_italic" );
    m_actionFormatItalic->setShortcut(KShortcut( Qt::CTRL + Qt::Key_I));
    connect( m_actionFormatBold, SIGNAL(toggled(bool)), this, SLOT(textItalic(bool)) );

    m_actionFormatUnderline = new KToggleAction(KIcon("text_under"), i18n( "Underline" ), actionCollection(), "format_underline" );
    m_actionFormatUnderline->setShortcut(KShortcut( Qt::CTRL + Qt::Key_U));
    connect( m_actionFormatUnderline, SIGNAL(toggled(bool)), this, SLOT(textUnderline(bool)) );

    m_actionFormatStrikeOut = new KToggleAction(KIcon("text_strike"), i18n( "Strike Out" ), actionCollection(), "format_strike" );
    connect( m_actionFormatStrikeOut, SIGNAL(toggled(bool)), this, SLOT(textStrikeOut(bool)) );

    // ------------------- Actions with a key binding and no GUI item
    KAction *action = new KAction( i18n( "Insert Non-Breaking Space" ), actionCollection(), "nonbreaking_space" );
    action->setShortcut( KShortcut( Qt::CTRL+Qt::Key_Space));
    connect(action, SIGNAL(triggered()), this, SLOT( slotNonbreakingSpace() ));

    action = new KAction( i18n( "Insert Non-Breaking Hyphen" ), actionCollection(), "nonbreaking_hyphen" );
    action->setShortcut( KShortcut( Qt::CTRL+Qt::SHIFT+Qt::Key_Minus));
    connect(action, SIGNAL(triggered()), this, SLOT( slotNonbreakingHyphen() ));

    action = new KAction( i18n( "Insert Soft Hyphen" ), actionCollection(), "soft_hyphen" );
    action->setShortcut( KShortcut( Qt::CTRL+Qt::Key_Minus));
    connect(action, SIGNAL(triggered()), this, SLOT( slotSoftHyphen() ));

    action = new KAction( i18n( "Line Break" ), actionCollection(), "line_break" );
    action->setShortcut( KShortcut( Qt::SHIFT+Qt::Key_Return));
    connect(action, SIGNAL(triggered()), this, SLOT( slotLineBreak() ));


    m_actionFormatFont = new KAction( i18n( "Font..." ), actionCollection(), "format_font" );
    m_actionFormatFont->setToolTip( i18n( "Change character size, font, boldface, italics etc." ) );
    m_actionFormatFont->setWhatsThis( i18n( "Change the attributes of the currently selected characters." ) );
    connect(m_actionFormatFont, SIGNAL(triggered()), this, SLOT( formatFont() ));

    m_actionViewShowGrid = new KToggleAction( i18n( "Show &Grid" ), KShortcut(),
                                            this, SLOT( viewGrid() ),
                                            actionCollection(), "view_grid" );
    m_actionViewShowGrid->setCheckedState(KGuiItem(i18n("Hide &Grid")));

    m_actionViewSnapToGrid= new KToggleAction( i18n( "Snap to Grid" ), KShortcut(),
                                             this, SLOT(viewSnapToGrid() ),
                                             actionCollection(), "view_snaptogrid" );

    m_viewZoomAction = new KoZoomAction(KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE,
                                        i18n("Zoom"), KIcon("viewmag"), KShortcut(),
                                        actionCollection(), "view_zoom");
    connect(m_viewZoomAction, SIGNAL(zoomChanged(KoZoomMode::Mode, int)),
            this, SLOT(viewZoom(KoZoomMode::Mode, int)));
    // plug the zoom action into view bar
    viewBar()->addAction(m_viewZoomAction);
}

void KPrView::viewSnapToGrid()
{
}

void KPrView::viewGrid()
{

}

void KPrView::viewZoom(KoZoomMode::Mode mode, int zoom)
{
    // No point trying to zoom something that isn't there...
    if(m_activePage == 0) {
        return;
    }

    int newZoom = zoom;
    QString zoomString;

    if(mode == KoZoomMode::ZOOM_WIDTH) {
        zoomString = KoZoomMode::toString(mode);
        KoPageLayout layout = m_activePage->pageLayout();
        newZoom = qRound(static_cast<double>(m_canvasController->visibleWidth() * 100) /
            (m_zoomHandler.resolutionX() * layout.ptWidth)) - 1;

        if((newZoom != m_zoomHandler.zoomInPercent()) &&
            m_canvasController->verticalScrollBar()->isHidden())
        {
        QTimer::singleShot(0, this, SLOT(recalculateZoom()));
        }
    } else if(mode == KoZoomMode::ZOOM_PAGE) {
        zoomString = KoZoomMode::toString(mode);
        KoPageLayout layout = m_activePage->pageLayout();
        double height = m_zoomHandler.resolutionY() * layout.ptHeight;
        double width = m_zoomHandler.resolutionX() * layout.ptWidth;
        newZoom = qMin(qRound(static_cast<double>(m_canvasController->visibleHeight() * 100) / height),
                    qRound(static_cast<double>(m_canvasController->visibleWidth() * 100) / width)) - 1;
    } else {
        zoomString = i18n("%1%", newZoom);
    }

    // Don't allow smaller zoom then 10% or bigger then 2000%
    if((newZoom < 10) || (newZoom > 2000) || (newZoom == m_zoomHandler.zoomInPercent())) {
        return;
    }

    m_zoomHandler.setZoomMode(mode);
    m_viewZoomAction->setZoom(zoomString);
    setZoom(newZoom);
    m_canvas->setFocus();
}

void KPrView::setZoom(int zoom)
{
    m_zoomHandler.setZoom(zoom);
    m_canvas->updateSize();
    m_viewZoomAction->setEffectiveZoom(zoom);
}

KoShapeManager* KPrView::shapeManager() const
{
    return m_canvas->shapeManager();
}

KPrCanvas* KPrView::canvasWidget() const
{
    return m_canvas;
}

void KPrView::setActivePage(KPrPage* page)
{
    if ( !page )
        return;
    m_activePage=page;

    m_canvas->shapeController()->setShapeControllerBase(m_activePage);
    shapeManager()->setShapes(m_activePage->shapes());

    KoPageLayout layout = m_activePage->pageLayout();
    m_horizontalRuler->setRulerLength(layout.ptWidth);
    m_verticalRuler->setRulerLength(layout.ptHeight);
    m_horizontalRuler->setActiveRange(layout.ptLeft, layout.ptWidth - layout.ptRight);
    m_verticalRuler->setActiveRange(layout.ptTop, layout.ptHeight - layout.ptBottom);

}

void KPrView::resizeEvent(QResizeEvent* event)
{
    KoView::resizeEvent(event);

    if(m_zoomHandler.zoomMode() != KoZoomMode::ZOOM_CONSTANT) {
        recalculateZoom();
    }

    m_horizontalRuler->setOffset(m_canvasController->canvasOffsetX());
    m_verticalRuler->setOffset(m_canvasController->canvasOffsetY());
}

void KPrView::recalculateZoom()
{
    viewZoom(m_zoomHandler.zoomMode(), m_zoomHandler.zoomInPercent());
}

void KPrView::textBold(bool bold) {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->bold(bold);
}

void KPrView::textItalic(bool italic) {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->italic(italic);
}

void KPrView::textUnderline(bool underline) {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->underline(underline);
}

void KPrView::textStrikeOut(bool strikeout) {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->strikeOut(strikeout);
}

void KPrView::slotNonbreakingSpace() {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->insert(QString(QChar(0xa0)));
}

void KPrView::slotNonbreakingHyphen() {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->insert(QString(QChar(0x2013)));
}

void KPrView::slotSoftHyphen() {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->insert(QString(QChar(0xad)));
}

void KPrView::slotLineBreak() {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->insert(QString(QChar('\n')));
}

void KPrView::formatFont() {
    KoTextSelectionHandler *handler = qobject_cast<KoTextSelectionHandler*> (kprcanvas()->toolProxy()->selection());
    if(handler)
        handler->selectFont(this);
}



#include "KPrView.moc"
