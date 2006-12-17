/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeLockCommand.h"

#include <klocale.h>

KoShapeLockCommand::KoShapeLockCommand(const KoSelectionSet &shapes, const QList<bool> &oldLock, const QList<bool> &newLock)
{
    m_shapes = shapes.toList();
    m_oldLock = oldLock;
    m_newLock = newLock;

    Q_ASSERT(m_shapes.count() == m_oldLock.count());
    Q_ASSERT(m_shapes.count() == m_newLock.count());
}

KoShapeLockCommand::KoShapeLockCommand(const QList<KoShape*> &shapes, const QList<bool> &oldLock, const QList<bool> &newLock)
{
    m_shapes = shapes;
    m_oldLock = oldLock;
    m_newLock = newLock;

    Q_ASSERT(m_shapes.count() == m_oldLock.count());
    Q_ASSERT(m_shapes.count() == m_newLock.count());
}

KoShapeLockCommand::~KoShapeLockCommand()
{
}

void KoShapeLockCommand::execute()
{
    for(int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setLocked(m_newLock[i]);
    }
}

void KoShapeLockCommand::unexecute()
{
    for(int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setLocked(m_oldLock[i]);
    }
}

QString KoShapeLockCommand::name () const
{
    return i18n("Lock shapes");
}
