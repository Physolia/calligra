/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998              */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* written for KDE (http://www.kde.org)                           */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Set PageLayout Command                                 */
/******************************************************************/

#include "kpresenter_view.h"
#include "kpresenter_doc.h"
#include "pglayoutcmd.h"
#include "pglayoutcmd.moc"

/******************************************************************/
/* Class: PgLayoutCmd                                             */
/******************************************************************/

/*======================== constructor ===========================*/
PgLayoutCmd::PgLayoutCmd(QString _name,KoPageLayout _layout,KoPageLayout _oldLayout,
			 KPresenterView_impl *_view)
  : Command(_name)
{
  layout = _layout;
  oldLayout = _oldLayout;
  view = _view;
}

/*====================== execute =================================*/
void PgLayoutCmd::execute()
{
  view->KPresenterDoc()->setPageLayout(layout,view->getDiffX(),view->getDiffY());
  view->getHRuler()->setPageLayout(layout);
  view->getVRuler()->setPageLayout(layout);
  view->setRanges();
}

/*====================== unexecute ===============================*/
void PgLayoutCmd::unexecute()
{
  view->KPresenterDoc()->setPageLayout(oldLayout,view->getDiffX(),view->getDiffY());
  view->getHRuler()->setPageLayout(oldLayout);
  view->getVRuler()->setPageLayout(oldLayout);
  view->setRanges();
}

