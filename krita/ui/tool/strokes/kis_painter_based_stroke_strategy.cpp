/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_painter_based_stroke_strategy.h"

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_transaction.h"
#include "kis_image.h"
#include "kis_distance_information.h"


KisPainterBasedStrokeStrategy::PainterInfo::PainterInfo()
    : painter(new KisPainter()),
      dragDistance(new KisDistanceInformation())
{
}

KisPainterBasedStrokeStrategy::PainterInfo::PainterInfo(const QPointF &lastPosition, int lastTime)
    : painter(new KisPainter()),
      dragDistance(new KisDistanceInformation(lastPosition, lastTime))
{
}

KisPainterBasedStrokeStrategy::PainterInfo::PainterInfo(const PainterInfo &rhs)
    : painter(new KisPainter()),
      dragDistance(new KisDistanceInformation(*rhs.dragDistance))
{
}

KisPainterBasedStrokeStrategy::PainterInfo::~PainterInfo()
{
    delete(painter);
    delete(dragDistance);
}

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const QString &id,
                                                             const KUndo2MagicString &name,
                                                             KisResourcesSnapshotSP resources,
                                                             QVector<PainterInfo*> painterInfos)
    : KisSimpleStrokeStrategy(id, name),
      m_resources(resources),
      m_painterInfos(painterInfos),
      m_transaction(0)
{
    init();
}

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const QString &id,
                                                             const KUndo2MagicString &name,
                                                             KisResourcesSnapshotSP resources,
                                                             PainterInfo *painterInfo)
    : KisSimpleStrokeStrategy(id, name),
      m_resources(resources),
      m_painterInfos(QVector<PainterInfo*>() <<  painterInfo),
      m_transaction(0)
{
    init();
}

void KisPainterBasedStrokeStrategy::init()
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
}

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const KisPainterBasedStrokeStrategy &rhs)
    : KisSimpleStrokeStrategy(rhs),
      m_resources(rhs.m_resources)
{
    foreach(PainterInfo *info, rhs.m_painterInfos) {
        m_painterInfos.append(new PainterInfo(*info));
    }

    KIS_ASSERT_RECOVER_NOOP(
        !rhs.m_transaction &&
        !rhs.m_targetDevice &&
        !rhs.m_activeSelection &&
        "After the stroke has been started, no copying must happen");
}

KisPaintDeviceSP KisPainterBasedStrokeStrategy::targetDevice() const
{
    return m_targetDevice;
}

KisSelectionSP KisPainterBasedStrokeStrategy::activeSelection() const
{
    return m_activeSelection;
}

const QVector<KisPainterBasedStrokeStrategy::PainterInfo*>
KisPainterBasedStrokeStrategy::painterInfos() const
{
    return m_painterInfos;
}

void KisPainterBasedStrokeStrategy::initPainters(KisPaintDeviceSP targetDevice,
                                                 KisSelectionSP selection,
                                                 bool hasIndirectPainting,
                                                 const QString &indirectPaintingCompositeOp)
{
    foreach(PainterInfo *info, m_painterInfos) {
        KisPainter *painter = info->painter;

        painter->begin(targetDevice, !hasIndirectPainting ? selection : 0);
        m_resources->setupPainter(painter);

        if(hasIndirectPainting) {
            painter->setCompositeOp(targetDevice->colorSpace()->compositeOp(indirectPaintingCompositeOp));
            painter->setOpacity(OPACITY_OPAQUE_U8);
            painter->setChannelFlags(QBitArray());
        }
    }
}

void KisPainterBasedStrokeStrategy::deletePainters()
{
    foreach(PainterInfo *info, m_painterInfos) {
        delete info;
    }

    m_painterInfos.clear();
}

void KisPainterBasedStrokeStrategy::initStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisPaintDeviceSP paintDevice = node->paintDevice();
    KisPaintDeviceSP targetDevice = paintDevice;
    bool hasIndirectPainting = needsIndirectPainting();

    KisSelectionSP selection = m_resources->activeSelection();

    if (hasIndirectPainting) {
        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(node.data());

        if (indirect) {
            targetDevice = paintDevice->createCompositionSourceDevice();
            targetDevice->setParentNode(node);
            indirect->setTemporaryTarget(targetDevice);
            indirect->setTemporaryCompositeOp(m_resources->compositeOp());
            indirect->setTemporaryOpacity(m_resources->opacity());
            indirect->setTemporarySelection(selection);

            QBitArray channelLockFlags = m_resources->channelLockFlags();
            indirect->setTemporaryChannelFlags(channelLockFlags);
        }
        else {
            hasIndirectPainting = false;
        }
    }

    m_transaction = new KisTransaction(name(), targetDevice);

    initPainters(targetDevice, selection, hasIndirectPainting, indirectPaintingCompositeOp());

    m_targetDevice = targetDevice;
    m_activeSelection = selection;

    // sanity check: selection should be applied only once
    if (selection && !m_painterInfos.isEmpty()) {
        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(node.data());
        KIS_ASSERT_RECOVER_RETURN(hasIndirectPainting || m_painterInfos.first()->painter->selection());
        KIS_ASSERT_RECOVER_RETURN(!hasIndirectPainting || !indirect->temporarySelection() || !m_painterInfos.first()->painter->selection());
    }
}

void KisPainterBasedStrokeStrategy::finishStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(layer && indirect && indirect->hasTemporaryTarget()) {
        KUndo2MagicString transactionText = m_transaction->text();
        m_transaction->end();

        indirect->mergeToLayer(layer,
                               m_resources->postExecutionUndoAdapter(),
                               transactionText);
    }
    else {
        m_transaction->commit(m_resources->postExecutionUndoAdapter());
    }
    delete m_transaction;
    deletePainters();
}

void KisPainterBasedStrokeStrategy::cancelStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(indirect && indirect->hasTemporaryTarget()) {
        delete m_transaction;
        deletePainters();

        QRegion region = indirect->temporaryTarget()->region();
        indirect->setTemporaryTarget(0);
        node->setDirty(region);
    } else {
        m_transaction->revert();
        delete m_transaction;
        deletePainters();
    }
}
