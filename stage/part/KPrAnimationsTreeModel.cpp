/* This file is part of the KDE project
 * Copyright (C) 2012 Paul Mendez <paulestebanms@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "KPrAnimationsTreeModel.h"

//Qt Headers

//Stage Headers
#include "KPrPage.h"
#include "KPrView.h"
#include "KPrDocument.h"
#include <KPrCustomAnimationItem.h>
#include "animations/KPrAnimationStep.h"
#include "animations/KPrAnimationSubStep.h"
#include "commands/KPrAnimationRemoveCommand.h"
#include "commands/KPrReorderAnimationCommand.h"

//Calligra Headers
#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoPADocument.h>

//KDE Headers
#include <KIcon>
#include <KIconLoader>
#include <KLocale>

enum Column {Name, Shape, TriggerEvent, Type};
const int COLUMN_COUNT = 4;

KPrAnimationsTreeModel::KPrAnimationsTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_rootItem(0)
    , m_lastRemovedIndex(QModelIndex())
{
}

KPrAnimationsTreeModel::~KPrAnimationsTreeModel()
{
    delete m_rootItem;
}

void KPrAnimationsTreeModel::clear()
{
    delete m_rootItem;
    reset();
}

Qt::ItemFlags KPrAnimationsTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags theFlags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        theFlags |= Qt::ItemIsSelectable|Qt::ItemIsEnabled;
        if (index.column() == Name)
            theFlags |= Qt::ItemIsEditable;//|
                        //Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled;
    }
    return theFlags;
}

QVariant KPrAnimationsTreeModel::data(const QModelIndex &index, int role) const
{
    if (!m_rootItem || !index.isValid() || index.column() < 0 ||
        index.column() >= COLUMN_COUNT) {
        return QVariant();
    }
    if (KPrCustomAnimationItem *item = itemForIndex(index)) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
                case Name: return item->animationName();
                case TriggerEvent: return QVariant();
                case Shape: return QVariant();
                case Type: return item->typeText();
                default: Q_ASSERT(false);
            }
        }
        if (role == Qt::TextAlignmentRole) {
            if (index.column() == Name)
                return static_cast<int>(Qt::AlignVCenter|
                                        Qt::AlignLeft);
            return static_cast<int>(Qt::AlignVCenter|Qt::AlignRight);
        }
        if (role == Qt::DecorationRole) {
            switch (index.column()) {
                case Name: return QVariant();
                case TriggerEvent:
                    if (item->triggerEvent() == KPrShapeAnimation::On_Click)
                        return KIcon("onclick").pixmap(KIconLoader::SizeSmall,
                                                       KIconLoader::SizeSmall);
                    if (item->triggerEvent() == KPrShapeAnimation::After_Previous)
                        return KIcon("after_previous").pixmap(KIconLoader::SizeSmall,
                                                              KIconLoader::SizeSmall);
                    if (item->triggerEvent() == KPrShapeAnimation::With_Previous)
                        return KIcon("with_previous").pixmap(KIconLoader::SizeSmall,
                                                             KIconLoader::SizeSmall);
                case Shape: return item->thumbnail();
                case Type: return QVariant();
                default: Q_ASSERT(false);
            }
        }
        if (role == Qt::SizeHintRole) {
            switch (index.column()) {
                case Name: return QVariant();
                case TriggerEvent: return QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
                case Shape: return QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
                case Type: return QVariant();
                default: Q_ASSERT(false);
            }
        }
        if (role == Qt::ToolTipRole) {
                switch (index.column()) {
                case Name: return QVariant();
                case TriggerEvent:
                    if (item->triggerEvent() == KPrShapeAnimation::On_Click)
                        return i18n("start on mouse click");
                    if (item->triggerEvent() == KPrShapeAnimation::After_Previous)
                        return i18n("start after previous animation");
                    if (item->triggerEvent() == KPrShapeAnimation::With_Previous)
                        return i18n("start with previous animation");
                case Shape: {
                    return (item->shape()) ? item->shape()->name() : QString();
                }
                case Type: return QVariant();
                default: Q_ASSERT(false);
                }
            }
    }
    return QVariant();
}

QVariant KPrAnimationsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == Name) {
            return i18n("Animation");
        }
        else if (section == TriggerEvent) {
            return QString();
        }
        else if (section == Shape) {
            return i18n("Shape");
        }
        else if (section == Type) {
            return i18n("Type");
        }
    }
    return QVariant();
}

int KPrAnimationsTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0) {
        return 0;
    }
    KPrCustomAnimationItem *parentItem = itemForIndex(parent);
    return parentItem ? parentItem->childCount() : 0;
}

int KPrAnimationsTreeModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() && parent.column() != 0 ? 0 : COLUMN_COUNT;
}

QModelIndex KPrAnimationsTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_rootItem || row < 0 || column < 0 || column >= COLUMN_COUNT
        || (parent.isValid() && parent.column() != 0)) {
        return QModelIndex();
    }
    KPrCustomAnimationItem *parentItem = itemForIndex(parent);
    Q_ASSERT(parentItem);
    if (KPrCustomAnimationItem *item = parentItem->childAt(row)) {
        return createIndex(row, column, item);
    }
    return QModelIndex();
}

QModelIndex KPrAnimationsTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    if (KPrCustomAnimationItem *childItem = itemForIndex(index)) {
        if (KPrCustomAnimationItem *parentItem = childItem->parent()) {
            if (parentItem == m_rootItem)
                return QModelIndex();
            if (KPrCustomAnimationItem *grandParentItem = parentItem->parent()) {
                int row = grandParentItem->rowOfChild(parentItem);
                return createIndex(row, 0, parentItem);
            }
        }
    }
    return QModelIndex();
}

bool KPrAnimationsTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //TODO: Edition features are not yet implemented
    return false;
}

bool KPrAnimationsTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!m_rootItem) {
        m_rootItem = new  KPrCustomAnimationItem;
    }
    //KPrCustomAnimationItem *parentItem = parent.isValid() ? itemForIndex(parent)
    //                                        : m_rootItem;
    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        //TODO config new animation
    }
    endInsertRows();
    return true;
}

bool KPrAnimationsTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!m_rootItem) {
        return false;
    }
    //KPrCustomAnimationItem *item = parent.isValid() ? itemForIndex(parent)
    //                                  : m_rootItem;
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        //delete item
    }
    endRemoveRows();
    return true;
}

QModelIndex KPrAnimationsTreeModel::moveUp(const QModelIndex &index)
{
    if (!index.isValid() || index.row() < 0) {
        return index;
    }
    KPrCustomAnimationItem *item = itemForIndex(index);
    Q_ASSERT(item);
    KPrCustomAnimationItem *parent = item->parent();
    Q_ASSERT(parent);
    return moveItem(parent, index.row(), index.row() - 1);
}

QModelIndex KPrAnimationsTreeModel::moveDown(const QModelIndex &index)
{
    if (!index.isValid()) {
        return index;
    }
    KPrCustomAnimationItem *item = itemForIndex(index);
    Q_ASSERT(item);
    KPrCustomAnimationItem *parent = item->parent();
    int newRow = index.row() + 1;
    if (!parent || parent->childCount() <= newRow) {
        return index;
    }
    return moveItem(parent, index.row(), newRow);
}

/*
QModelIndex KPrAnimationsTreeModel::cut(const QModelIndex &index)
{
    if (!index.isValid()) {
        return index;
    }
    delete cutItem;
    cutItem = itemForIndex(index);
    Q_ASSERT(cutItem);
    KPrCustomAnimationItem *parent = cutItem->parent();
    Q_ASSERT(parent);
    int row = parent->rowOfChild(cutItem);
    Q_ASSERT(row == index.row());
    beginRemoveRows(index.parent(), row, row);
    KPrCustomAnimationItem *child = parent->takeChild(row);
    endRemoveRows();
    Q_ASSERT(child == cutItem);
    child = 0; // Silence compiler unused variable warning

    if (row > 0) {
        --row;
        return createIndex(row, 0, parent->childAt(row));
    }
    if (parent != m_rootItem) {
        KPrCustomAnimationItem *grandParent = parent->parent();
        Q_ASSERT(grandParent);
        return createIndex(grandParent->rowOfChild(parent), 0, parent);
    }
    return QModelIndex();
}*/

QModelIndex KPrAnimationsTreeModel::removeItemByIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        return index;
    }
    KPrCustomAnimationItem *item = itemForIndex(index);
    Q_ASSERT(item);

    if (item && (!item->isDefaulAnimation())) {
        beginRemoveRows(index.parent(), index.row(), index.row());
        KPrDocument *doc = dynamic_cast<KPrDocument*>(m_view->kopaDocument());
        KPrAnimationRemoveCommand *command = new KPrAnimationRemoveCommand(doc, item->animation());
        delete(item);
        doc->addCommand(command);
        endRemoveRows();
    }
    return QModelIndex();
}

/*QModelIndex KPrAnimationsTreeModel::paste(const QModelIndex &index)
{
    if (!index.isValid() || !cutItem) {
        return index;
    }
    KPrCustomAnimationItem *sibling = itemForIndex(index);
    Q_ASSERT(sibling);
    KPrCustomAnimationItem *parent = sibling->parent();
    Q_ASSERT(parent);
    int row = parent->rowOfChild(sibling) + 1;
    beginInsertRows(index.parent(), row, row);
    parent->insertChild(row, cutItem);
    KPrCustomAnimationItem *child = cutItem;
    cutItem = 0;
    endInsertRows();
    return createIndex(row, 0, child);
}

QModelIndex KPrAnimationsTreeModel::promote(const QModelIndex &index)
{
    if (!index.isValid()) {
        return index;
    }
    KPrCustomAnimationItem *item = itemForIndex(index);
    Q_ASSERT(item);
    KPrCustomAnimationItem *parent = item->parent();
    Q_ASSERT(parent);
    if (parent == m_rootItem) {
        return index; // Already a top-level item
    }

    int row = parent->rowOfChild(item);
    KPrCustomAnimationItem *child = parent->takeChild(row);
    Q_ASSERT(child == item);
    KPrCustomAnimationItem *grandParent = parent->parent();
    Q_ASSERT(grandParent);
    row = grandParent->rowOfChild(parent) + 1;
    grandParent->insertChild(row, child);
    QModelIndex newIndex = createIndex(row, 0, child);
    emit dataChanged(newIndex, newIndex);
    return newIndex;
}

QModelIndex KPrAnimationsTreeModel::demote(const QModelIndex &index)
{
    if (!index.isValid()) {
        return index;
    }
    KPrCustomAnimationItem *item = itemForIndex(index);
    Q_ASSERT(item);
    KPrCustomAnimationItem *parent = item->parent();
    Q_ASSERT(parent);
    int row = parent->rowOfChild(item);
    if (row == 0) {
        return index; // No preceding sibling to move this under
    }
    KPrCustomAnimationItem *child = parent->takeChild(row);
    Q_ASSERT(child == item);
    KPrCustomAnimationItem *sibling = parent->childAt(row - 1);
    Q_ASSERT(sibling);
    sibling->addChild(child);
    QModelIndex newIndex = createIndex(sibling->childCount() - 1, 0,
                                       child);
    emit dataChanged(newIndex, newIndex);
    return newIndex;
}
*/
void KPrAnimationsTreeModel::setActivePage(KPrPage *activePage)
{
    Q_ASSERT(activePage);
    m_activePage = activePage;
    clear();
    m_rootItem = new KPrCustomAnimationItem;
    // Initialize tree
    m_rootItem->initAsRootAnimation(activePage);
    connect(m_rootItem, SIGNAL(rootModified()), this, SLOT(update()));
    emit rootChanged();
}

void KPrAnimationsTreeModel::setDocumentView(KPrView *view)
{
    m_view = view;
    if (m_view->kopaDocument())
    {
        connect(m_view->kopaDocument(), SIGNAL(shapeRemoved(KoShape*)), this, SLOT(updateData()));
        connect(m_view->kopaDocument(), SIGNAL(shapeAdded(KoShape*)), this, SLOT(updateData()));
        KPrDocument *doc = dynamic_cast<KPrDocument*>(m_view->kopaDocument());
        if (doc) {
            connect(doc, SIGNAL(animationAdded(KPrShapeAnimation*)), this, SLOT(updateAnimationData(KPrShapeAnimation*)));
            connect(doc, SIGNAL(animationRemoved(KPrShapeAnimation*)), this, SLOT(updateAnimationData(KPrShapeAnimation*)));
            connect(doc, SIGNAL(animationReplaced(KPrShapeAnimation*,KPrShapeAnimation*)), this, SLOT(updateByAnimationReplaced(KPrShapeAnimation*,KPrShapeAnimation*)));
        }
    }
    reset();
}

QModelIndex KPrAnimationsTreeModel::indexByShape(KoShape *shape)
{
    QModelIndex parent = QModelIndex();
    if (!shape) {
        return QModelIndex();
    }
    for (int row = 0; row < rowCount(parent); ++row) {
        QModelIndex thisIndex = index(row, 0, parent);
        KPrCustomAnimationItem *item = itemForIndex(thisIndex);
        if (item->shape() == shape) {
            return thisIndex;
        }
        if (item->childCount() > 0) {
            foreach (KPrCustomAnimationItem *child, item->children()) {
                if (child->shape() == shape) {
                    thisIndex = indexByItem(child);
                    return thisIndex;
                }
            }
        }
    }
    return QModelIndex();
}

QModelIndex KPrAnimationsTreeModel::indexByItem(KPrCustomAnimationItem *item)
{
    QModelIndex parent = QModelIndex();
    if (!item) {
        return QModelIndex();
    }
    for (int row = 0; row < rowCount(parent); ++row) {
        QModelIndex thisIndex = index(row, 0, parent);
        KPrCustomAnimationItem *newItem = itemForIndex(thisIndex);
        if (item == newItem) {
            return thisIndex;
        }
        if (newItem->childCount() > 0) {
            int childRow = 0;
            foreach (KPrCustomAnimationItem *child, newItem->children()) {
                if (child == item) {
                    QModelIndex childIndex = index(childRow, 0, thisIndex);
                    return childIndex;
                }
                childRow++;
            }
        }
    }
    return QModelIndex();
}

KPrCustomAnimationItem *KPrAnimationsTreeModel::rootItem() const
{
    return m_rootItem;
}

void KPrAnimationsTreeModel::notifyBranchesSwap(KPrCustomAnimationItem *itemOld, KPrCustomAnimationItem *itemNew)
{
    layoutAboutToBeChanged();
    updateBranch(itemOld);
    updateBranch(itemNew);
    layoutChanged();
}

void KPrAnimationsTreeModel::updateData()
{
    setActivePage(m_activePage);
    reset();
}

void KPrAnimationsTreeModel::update()
{
    emit layoutAboutToBeChanged();
    reset();
    emit layoutChanged();
}

void KPrAnimationsTreeModel::updateAnimationData(KPrShapeAnimation *modifiedAnimation)
{
    bool animationDeleted = true;
    KPrCustomAnimationItem *newItem;
    //Look for added animation
    foreach (KPrAnimationStep *step, m_activePage->animationSteps()) {
        for (int i=0; i < step->animationCount(); i++) {
            QAbstractAnimation *animation = step->animationAt(i);
            if (KPrAnimationSubStep *a = dynamic_cast<KPrAnimationSubStep*>(animation)) {
                for (int j=0; j < a->animationCount(); j++) {
                    QAbstractAnimation *shapeAnimation = a->animationAt(j);
                    if (KPrShapeAnimation *b = dynamic_cast<KPrShapeAnimation*>(shapeAnimation)) {
                        if ((b->presetClass() != KPrShapeAnimation::None) && (m_view->shapeManager()->shapes().contains(b->shape()))) {
                            //Animation Added
                            if (b == modifiedAnimation) {
                                animationDeleted = false;
                                newItem = new KPrCustomAnimationItem(b, m_rootItem);
                                QModelIndex index = indexByItem(newItem);
                                if (index.isValid()) {
                                    m_lastRemovedIndex = index;
                                    beginInsertRows(index.parent(), index.row(), index.row());
                                    endInsertRows();
                                    //announceItemChanged(newItem);
                                }
                                int count = newItem->childCount();
                                if (count > 0) {
                                    QModelIndex endIndex = indexByItem(newItem->childAt(count - 1));
                                    if (endIndex.isValid()) {
                                        emit dataChanged(index, endIndex);
                                    }
                                }
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    if (animationDeleted) {
        // look for deleted animation:
        if (m_lastRemovedIndex.isValid()) {
             beginRemoveRows(m_lastRemovedIndex.parent(), m_lastRemovedIndex.row(), m_lastRemovedIndex.row());
             endRemoveRows();
        }
    }
}

void KPrAnimationsTreeModel::updateByAnimationReplaced(KPrShapeAnimation *oldAnimation, KPrShapeAnimation *newAnimation)
{
    int updateRow = -1;
    int row = 0;
    KPrCustomAnimationItem *itemToUpdate = 0;
    foreach (KPrCustomAnimationItem *item, m_rootItem->children()) {
        if (row > 0) {
            if ((item->animation() == oldAnimation) || (item->animation() == newAnimation) ) {
                updateRow = row;
                itemToUpdate = item;
                break;
            }
            int childRow = 0;
            foreach (KPrCustomAnimationItem *child, item->children()) {
                if ((child->animation() == newAnimation) || (child->animation() == oldAnimation)) {
                    updateRow = childRow;
                    itemToUpdate = child;
                    break;
                }
                childRow++;
            }
        }
        row++;
    }

    if (updateRow >= 0) {
        QModelIndex startIndex = createIndex(updateRow, static_cast<int>(Name),
                                             itemToUpdate);
        QModelIndex endIndex = createIndex(updateRow, static_cast<int>(Type),
                                           itemToUpdate);
        emit dataChanged(startIndex, endIndex);
    }
}

void KPrAnimationsTreeModel::updateBranch(KPrCustomAnimationItem *item)
{
    if (!item) {
        return;
    }
    KPrCustomAnimationItem *parent = item->parent();
    Q_ASSERT(parent);
    int row = parent->rowOfChild(item);
    QModelIndex startIndex = createIndex(row, static_cast<int>(Name),
                                         item);
    QModelIndex endIndex = createIndex(row, static_cast<int>(Type),
                                       item);
    emit dataChanged(startIndex, endIndex);
    if (item->children().isEmpty()) {
        return;
    }
    // update Childs
    foreach (KPrCustomAnimationItem *child, item->children()) {
        updateBranch(child);
    }
}

KPrCustomAnimationItem *KPrAnimationsTreeModel::itemForIndex(const QModelIndex &index) const
{
    if (!m_rootItem) {
        return 0;
    }
    if (index.isValid()) {
        if (KPrCustomAnimationItem *item = static_cast<KPrCustomAnimationItem*>(
                index.internalPointer()))
            return item;
    }
    return m_rootItem;
}

void KPrAnimationsTreeModel::announceItemChanged(KPrCustomAnimationItem *item)
{
    if (item == m_rootItem) {
        return;
    }
    KPrCustomAnimationItem *parent = item->parent();
    Q_ASSERT(parent);
    int row = parent->rowOfChild(item);
    QModelIndex startIndex = createIndex(row, static_cast<int>(Name),
                                         item);
    QModelIndex endIndex = createIndex(row, static_cast<int>(Type),
                                       item);
    emit dataChanged(startIndex, endIndex);
    // Update the parent & parent's parent etc too
    announceItemChanged(parent);
}

QModelIndex KPrAnimationsTreeModel::moveItem(KPrCustomAnimationItem *parent, int oldRow, int newRow)
{
    //First item can't be moved
    Q_ASSERT(0 < oldRow && oldRow < parent->childCount() &&
             0 < newRow && newRow < parent->childCount());
    QModelIndex newIndex;
    // swap top level items
    if (parent == m_rootItem) {
        KPrCustomAnimationItem *itemOld = itemForIndex(index(oldRow, 0));
        KPrCustomAnimationItem *itemNew = itemForIndex(index(newRow, 0));
        if (itemOld && itemNew) {
            if (KPrDocument *doc = dynamic_cast<KPrDocument*>(m_view->kopaDocument())) {
                KPrReorderAnimationCommand *cmd = new KPrReorderAnimationCommand(m_activePage, itemOld->animation()->step(), itemNew->animation()->step(), this);
                doc->addCommand(cmd);
                newIndex = indexByItem(itemNew);
                notifyBranchesSwap(itemOld, itemNew);
            }
        }
    }
    return newIndex;
}
