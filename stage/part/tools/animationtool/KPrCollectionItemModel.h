/* This file is part of the KDE project
 * Copyright (C) 2012 Paul Mendez <paulestebanms@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

// Heavily based in CollectionItemModel work of Peter Simonsson <peter.simonsson@gmail.com>

#ifndef KPRCOLLECTIONITEMMODEL_H
#define KPRCOLLECTIONITEMMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QString>
#include <QIcon>
#include <KoXmlReader.h>

/**
 * Struct containing the information stored in CollectionItemModel item
 */
struct KPrCollectionItem
{
    QString id;
    QString name;
    QString toolTip;
    QIcon icon;
    KoXmlElement animationContext;
};

class KPrCollectionItemModel : public QAbstractListModel
{
    Q_OBJECT
    public:
        KPrCollectionItemModel(QObject* parent = 0);

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;

        /**
         * Set the list of KoCollectionItem to be stored in the model
         */
        void setAnimationClassList(const QList<KPrCollectionItem>& newlist);
        QList<KPrCollectionItem> animationClassList () const { return m_animationClassList; }

        KoXmlElement animationContext(const QModelIndex& index) const;

    private:
        QList<KPrCollectionItem> m_animationClassList;
        QString m_family;
};
#endif // KPRCOLLECTIONITEMMODEL_H
