/* This file is part of the KDE project
   Copyright (C) 2012 Paul Mendez <paulestebanms@gmail.com>

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

#ifndef KPREDITANIMATIONSWIDGET_H
#define KPREDITANIMATIONSWIDGET_H

#include <QWidget>

class KPrAnimationsTimeLineView;
class KPrAnimationsDataModel;
class KPrView;
class KoPAViewBase;
class QModelIndex;
class KoShape;
class QModelIndex;
class KPrCustomAnimationItem;
class QComboBox;
class QTimeEdit;

/** This Widget holds the configuration popup dialog
    used to edit animation properties of existing or new
    animation.
    Animations on this view are displayed in groups that
    have as parent animations started by a click*/
class KPrEditAnimationsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KPrEditAnimationsWidget(QWidget *parent = 0);
    void setView(KoPAViewBase *view);
    void setParentItem(KPrCustomAnimationItem *item, KPrCustomAnimationItem *rootItem);
    void setCurrentIndex(const QModelIndex &index);
    void setActiveShape(KoShape *shape);
    
signals:
    void itemClicked(const QModelIndex &index);
    void requestAnimationPreview();

public slots:
    void updateIndex(const QModelIndex &index);

private:
    KPrView* m_view;
    KPrAnimationsTimeLineView *m_timeLineView;
    KPrAnimationsDataModel *m_timeLineModel;
    QComboBox *m_triggerEventList;
    QTimeEdit *m_delayEdit;
    QTimeEdit *m_durationEdit;
    
};

#endif // KPREDITANIMATIONSWIDGET_H
