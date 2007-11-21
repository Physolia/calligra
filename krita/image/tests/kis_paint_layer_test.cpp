/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include <qtest_kde.h>

#include "kis_paint_layer_test.h"

#include <QImage>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_group_layer.h"
#include "kis_types.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_transparency_mask.h"
#include "testutil.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"

void KisPaintLayerTest::testProjection()
{

    QImage qimg( QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisImageSP image = new KisImage(0, qimg.width(), qimg.height(), cs, "merge test");

    KisPaintLayerSP layer = new KisPaintLayer( image, "test", OPACITY_OPAQUE );
    layer->paintDevice()->convertFromQImage( qimg, 0, 0, 0 );
    image->addNode( layer.data() );

    // Make sure the projection and the paint device are the same -- we don't have masks yet
    QVERIFY( layer->paintDevice().data() == layer->projection().data() );

    KisTransparencyMaskSP transparencyMask = new KisTransparencyMask();
    image->addNode( transparencyMask.data(), layer.data() );

    // Now there are masks. Verify that
    Q_ASSERT( layer->hasEffectMasks() );

    // Which also means that the projection is no longer the paint device
    QVERIFY( layer->paintDevice().data() != layer->projection().data() );

    // And the projection is still 0, because we've updated it
    QVERIFY( layer->projection().data() == 0 );

    // And now we're going to update the projection, but nothing is dirty yet
    layer->updateProjection( qimg.rect() );

    // Which also means that the projection is no longer the paint device
    QVERIFY( layer->paintDevice().data() != layer->projection().data() );

    // And the projection is no longer 0, because while we've updated it, nothing is dirty,
    // so nothing gets updated
    QVERIFY( layer->projection().data() == 0 );

    // Now the machinery will start to roll
    layer->setDirty( qimg.rect() );

    // And now we're going to update the projection, but nothing is dirty yet
    layer->updateProjection( qimg.rect() );

    // Which also means that the projection is no longer the paint device
    QVERIFY( layer->paintDevice().data() != layer->projection().data() );

    // And the projection is no longer 0, because while we've updated it, nothing is dirty,
    // so nothing gets updated
    QVERIFY( layer->projection().data() != 0 );

    // The selection is initially empty, so after an update, all pixels are still visible
    layer->updateProjection( qimg.rect() );

    // By default a new transparency mask blanks out the entire layer (photoshop mode "hide all")
    KisRectConstIterator it = layer->projection()->createRectConstIterator(0, 0, qimg.width(), qimg.height());
    while (!it.isDone()) {
        QVERIFY(cs->alpha(it.rawData()) == OPACITY_TRANSPARENT);
        ++it;
    }

    // Now fill the layer with some opaque pixels
    KisFillPainter gc(transparencyMask->selection()->getOrCreatePixelSelection());
    gc.fillRect(qimg.rect(), KoColor(QColor(0, 0, 0, 0), cs), MAX_SELECTED);
    gc.end();

    qDebug() << transparencyMask->selection()->pixelSelection()->selectedExactRect();
    qDebug() << transparencyMask->selection()->selectedExactRect();
    transparencyMask->selection()->updateProjection(qimg.rect());
    qDebug() << transparencyMask->selection()->selectedExactRect();

    layer->updateProjection( qimg.rect() );

    layer->projection()->convertToQImage(0, 0, 0, qimg.width(), qimg.height()).save("aaa.png");
    // Nothing is transparent anymore, so the projection and the paint device should be identical again
    QPoint errpoint;
    if ( !TestUtil::compareQImages( errpoint, qimg, layer->projection()->convertToQImage(0, 0, 0, qimg.width(), qimg.height() ) ) ) {
        QFAIL( QString( "Failed to create identical image, first different pixel: %1,%2 " ).arg( errpoint.x() ).arg( errpoint.y() ).toAscii() );
    }

}


QTEST_KDEMAIN(KisPaintLayerTest, GUI)
#include "kis_paint_layer_test.moc"
