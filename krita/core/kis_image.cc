/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <stdlib.h>
#include <math.h>

#include <config.h>
#include LCMS_HEADER

#include <qimage.h>
#include <qpainter.h>
#include <qsize.h>
#include <qtl.h>
#include <qapplication.h>
#include <qthread.h>

#include <kcommand.h>
#include <kocommandhistory.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_image_iface.h"

#include "kis_annotation.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_color.h"
#include "kis_command.h"
#include "kis_types.h"
//#include "kis_guide.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_paint_device_action.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_layer.h"
#include "kis_colorspace_convert_visitor.h"
#include "kis_background.h"
#include "kis_nameserver.h"
#include "kis_undo_adapter.h"
#include "kis_merge_visitor.h"
#include "kis_transaction.h"
#include "kis_scale_visitor.h"
#include "kis_crop_visitor.h"
#include "kis_profile.h"

class KisImage::KisImagePrivate {
public:
    KisColor backgroundColor;

#ifdef __BIG_ENDIAN__
    cmsHTRANSFORM bigEndianTransform;
#endif

};


namespace {

    class KisResizeImageCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisResizeImageCmd(KisUndoAdapter *adapter,
                          KisImageSP img,
                          Q_INT32 width,
                          Q_INT32 height,
                          Q_INT32 oldWidth,
                          Q_INT32 oldHeight) : super(i18n("Resize Image"))
            {
                m_adapter = adapter;
                m_img = img;
                m_before = QSize(oldWidth, oldHeight);
                m_after = QSize(width, height);
            }

        virtual ~KisResizeImageCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter -> setUndo(false);
                m_img -> resize(m_after.width(), m_after.height());
                m_adapter -> setUndo(true);
                m_img -> notify(QRect( 0, 0, QMAX(m_before.width(), m_after.width()), QMAX(m_before.height(), m_after.height())) );
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);
                m_img -> resize(m_before.width(), m_before.height());
                m_adapter -> setUndo(true);
                m_img -> notify(QRect( 0, 0, QMAX(m_before.width(), m_after.width()), QMAX(m_before.height(), m_after.height())) );
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        QSize m_before;
        QSize m_after;
    };

    // -------------------------------------------------------

    class KisChangeLayersCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisChangeLayersCmd(KisUndoAdapter *adapter, KisImageSP img,
                           KisGroupLayerSP oldRootLayer, KisGroupLayerSP newRootLayer, const QString& name)
            : super(name)
            {
                m_adapter = adapter;
                m_img = img;
                m_oldRootLayer = oldRootLayer;
                m_newRootLayer = newRootLayer;
            }

        virtual ~KisChangeLayersCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter -> setUndo(false);
                m_img -> setRootLayer(m_newRootLayer);
                m_adapter -> setUndo(true);
                m_img -> notifyLayersChanged();
                m_img -> notify();
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);
                m_img -> setRootLayer(m_oldRootLayer);
                m_adapter -> setUndo(true);
                m_img -> notifyLayersChanged();
                m_img -> notify();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisGroupLayerSP m_oldRootLayer;
        KisGroupLayerSP m_newRootLayer;
    };


    // -------------------------------------------------------

    class KisConvertImageTypeCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisConvertImageTypeCmd(KisUndoAdapter *adapter, KisImageSP img,
                               KisColorSpace * beforeColorSpace, KisColorSpace * afterColorSpace
            ) : super(i18n("Convert Image Type"))
            {
                m_adapter = adapter;
                m_img = img;
                m_beforeColorSpace = beforeColorSpace;
                m_afterColorSpace = afterColorSpace;
            }

        virtual ~KisConvertImageTypeCmd()
            {
            }

    public:
        virtual void execute()
            {
                m_adapter -> setUndo(false);

                m_img -> setColorSpace(m_afterColorSpace);
                m_img -> setProfile(m_afterColorSpace -> getProfile());

                m_adapter -> setUndo(true);
                m_img -> notify();
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);

                m_img -> setColorSpace(m_beforeColorSpace);
                m_img -> setProfile(m_beforeColorSpace -> getProfile());

                m_adapter -> setUndo(true);
                m_img -> notify();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisColorSpace * m_beforeColorSpace;
        KisColorSpace * m_afterColorSpace;
    };


    // -------------------------------------------------------

    class KisImageCommand : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        KisImageCommand(const QString& name, KisImageSP image);
        virtual ~KisImageCommand() {}

        virtual void execute() = 0;
        virtual void unexecute() = 0;

    protected:
        void setUndo(bool undo);

        KisImageSP m_image;
    };

    KisImageCommand::KisImageCommand(const QString& name, KisImageSP image) :
        super(name), m_image(image)
    {
    }

    void KisImageCommand::setUndo(bool undo)
    {
        if (m_image -> undoAdapter()) {
            m_image->undoAdapter()->setUndo(undo);
        }
    }


    // -------------------------------------------------------

    class KisLayerPositionCommand : public KisImageCommand {
        typedef KisImageCommand super;

    public:
        KisLayerPositionCommand(const QString& name, KisImageSP image, KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis) : super(name, image)
            {
                m_layer = layer;
                m_oldParent = layer->parent();
                m_oldAboveThis = layer->nextSibling();
                m_newParent = parent;
                m_newAboveThis = aboveThis;
           }

        virtual void execute()
            {
                setUndo(false);
                m_image -> moveLayer(m_layer, m_newParent, m_newAboveThis);
                setUndo(true);
            }

        virtual void unexecute()
            {
                setUndo(false);
                m_image -> moveLayer(m_layer, m_oldParent, m_oldAboveThis);
                setUndo(true);
            }

    private:
        KisLayerSP m_layer;
        KisGroupLayerSP m_oldParent;
        KisLayerSP m_oldAboveThis;
        KisGroupLayerSP m_newParent;
        KisLayerSP m_newAboveThis;
    };


    // -------------------------------------------------------

    class LayerAddCmd : public KisCommand {
        typedef KisCommand super;

    public:
        LayerAddCmd(KisUndoAdapter *adapter, KisImageSP img, KisLayerSP layer) : super(i18n("Add Layer"), adapter)
            {
                m_img = img;
                m_layer = layer;
                m_parent = layer->parent();
                m_aboveThis = layer->nextSibling();
            }

        virtual ~LayerAddCmd()
            {
            }

        virtual void execute()
            {
                adapter() -> setUndo(false);
                m_img -> addLayer(m_layer, m_parent.data(), m_aboveThis);
                adapter() -> setUndo(true);
            }

        virtual void unexecute()
            {
                adapter() -> setUndo(false);
                m_img -> removeLayer(m_layer);
                adapter() -> setUndo(true);
            }

    private:
        KisImageSP m_img;
        KisLayerSP m_layer;
        KisGroupLayerSP m_parent;
        KisLayerSP m_aboveThis;
    };

    // -------------------------------------------------------

    class LayerRmCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        LayerRmCmd(KisUndoAdapter *adapter, KisImageSP img,
                   KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAbove)
            : super(i18n("Remove Layer"))
            {
                m_adapter = adapter;
                m_img = img;
                m_layer = layer;
                m_prevParent = wasParent;
                m_prevAbove = wasAbove;
            }

        virtual ~LayerRmCmd()
            {
            }

        virtual void execute()
            {
                m_adapter -> setUndo(false);
                m_img -> removeLayer(m_layer);
                m_adapter -> setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);
                m_img -> addLayer(m_layer, m_prevParent.data(), m_prevAbove);
                m_adapter -> setUndo(true);
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisLayerSP m_layer;
        KisGroupLayerSP m_prevParent;
        KisLayerSP m_prevAbove;
    };

    class LayerMoveCmd: public KNamedCommand {
        typedef KNamedCommand super;

    public:
        LayerMoveCmd(KisUndoAdapter *adapter, KisImageSP img,
                         KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP wasAbove)
            : super(i18n("Move Layer"))
            {
                m_adapter = adapter;
                m_img = img;
                m_layer = layer;
                m_prevParent = wasParent;
                m_prevAbove = wasAbove;
                m_newParent = layer -> parent();
                m_newAbove = layer -> nextSibling();
            }

        virtual ~LayerMoveCmd()
            {
            }

        virtual void execute()
            {
                m_adapter -> setUndo(false);
                m_img -> moveLayer(m_layer, m_newParent.data(), m_newAbove);
                m_adapter -> setUndo(true);
            }

        virtual void unexecute()
            {
                m_adapter -> setUndo(false);
                m_img -> moveLayer(m_layer, m_prevParent.data(), m_prevAbove);
                m_adapter -> setUndo(true);
            }

    private:
        KisUndoAdapter *m_adapter;
        KisImageSP m_img;
        KisLayerSP m_layer;
        KisGroupLayerSP m_prevParent;
        KisLayerSP m_prevAbove;
        KisGroupLayerSP m_newParent;
        KisLayerSP m_newAbove;
    };


    // -------------------------------------------------------

    class LayerPropsCmd : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        LayerPropsCmd(KisLayerSP layer,
                      KisImageSP img,
                      KisUndoAdapter *adapter,
                      const QString& name,
                      Q_INT32 opacity,
                      const KisCompositeOp& compositeOp) : super(i18n("Layer Property Changes"))
            {
                m_layer = layer;
                m_img = img;
                m_adapter = adapter;
                m_name = name;
                m_opacity = opacity;
                m_compositeOp = compositeOp;
            }

        virtual ~LayerPropsCmd()
            {
            }

    public:
        virtual void execute()
            {
                QString name = m_layer -> name();
                Q_INT32 opacity = m_layer -> opacity();
                KisCompositeOp compositeOp = m_layer -> compositeOp();

                m_adapter -> setUndo(false);
                m_img -> setLayerProperties(m_layer,
                                            m_opacity,
                                            m_compositeOp,
                                            m_name);
                m_adapter -> setUndo(true);
                m_name = name;
                m_opacity = opacity;
                m_compositeOp = compositeOp;
                m_img -> notify();
            }

        virtual void unexecute()
            {
                execute();
            }

    private:
        KisUndoAdapter *m_adapter;
        KisLayerSP m_layer;
        KisImageSP m_img;
        QString m_name;
        Q_INT32 m_opacity;
        KisCompositeOp m_compositeOp;
    };
}

KisImage::KisImage(KisUndoAdapter *adapter, Q_INT32 width, Q_INT32 height,  KisColorSpace * colorSpace, const QString& name)
    : QObject(0, name.latin1()), KShared()
{
    init(adapter, width, height, colorSpace, name);
    setName(name);
    m_dcop = 0L;
}

KisImage::KisImage(const KisImage& rhs) : QObject(), KShared(rhs)
{
    m_dcop = 0L;
    if (this != &rhs) {
        m_private = new KisImagePrivate(*rhs.m_private);
        m_undoHistory = rhs.m_undoHistory;
        m_uri = rhs.m_uri;
        m_name = QString::null;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_xres = rhs.m_xres;
        m_yres = rhs.m_yres;
        m_unit = rhs.m_unit;
        m_colorSpace = rhs.m_colorSpace;
        m_dirty = rhs.m_dirty;
        m_adapter = rhs.m_adapter;

        m_bkg = new KisBackground();
        Q_CHECK_PTR(m_bkg);

        m_rootLayer = static_cast<KisGroupLayer*>(rhs.m_rootLayer->clone().data());
        m_annotations = rhs.m_annotations; // XXX the annotations would probably need to be deep-copied

        m_nserver = new KisNameServer(i18n("Layer %1"), rhs.m_nserver -> currentSeed() + 1);
        Q_CHECK_PTR(m_nserver);

        //m_guides = rhs.m_guides;
    }
}



DCOPObject * KisImage::dcopObject()
{
    if (!m_dcop) {
        m_dcop = new KisImageIface(this);
        Q_CHECK_PTR(m_dcop);
    }
    return m_dcop;
}

KisImage::~KisImage()
{
    delete m_private;
    delete m_nserver;
    delete m_dcop;
}

QString KisImage::name() const
{
    return m_name;
}

void KisImage::setName(const QString& name)
{
    if (!name.isEmpty())
        m_name = name;
}

QString KisImage::description() const
{
    return m_description;
}

void KisImage::setDescription(const QString& description)
{
    if (!description.isEmpty())
        m_description = description;
}


KisColor KisImage::backgroundColor() const
{
    return m_private->backgroundColor;
}

void KisImage::setBackgroundColor(const KisColor & color)
{
    m_private->backgroundColor = color;
}


QString KisImage::nextLayerName() const
{
    if (m_nserver -> currentSeed() == 0) {
        m_nserver -> number();
        return i18n("background");
    }

    return m_nserver -> name();
}

void KisImage::init(KisUndoAdapter *adapter, Q_INT32 width, Q_INT32 height,  KisColorSpace * colorSpace, const QString& name)
{
    m_private = new KisImagePrivate();
    m_private->backgroundColor = KisColor(Qt::white, colorSpace);

    Q_ASSERT(colorSpace != 0);
    m_renderinit = false;
    m_adapter = adapter;

    m_nserver = new KisNameServer(i18n("Layer %1"), 1);
    Q_CHECK_PTR(m_nserver);
    m_name = name;

    m_colorSpace = colorSpace;
    m_bkg = new KisBackground();
    Q_CHECK_PTR(m_bkg);

    m_rootLayer = new KisGroupLayer(this,"root", OPACITY_OPAQUE);
    Q_CHECK_PTR(m_rootLayer);

    m_xres = 1.0;
    m_yres = 1.0;
    m_unit = KoUnit::U_PT;
    m_dirty = false;
    m_undoHistory = 0;
    m_width = width;
    m_height = height;

#ifdef __BIG_ENDIAN__
    cmsHPROFILE hProfile = cmsCreate_sRGBProfile();
    m_private->bigEndianTransform = cmsCreateTransform(hProfile ,
                                              TYPE_ABGR_8,
                                              hProfile ,
                                              TYPE_RGBA_8,
                                              INTENT_PERCEPTUAL,
                                              0);
#endif

    connect(this, SIGNAL(sigSizeChanged(Q_INT32, Q_INT32)), SIGNAL(sigNonActiveLayersUpdated()));
}


void KisImage::resize(Q_INT32 w, Q_INT32 h, Q_INT32 x, Q_INT32 y, bool cropLayers)
{
    if (w != width() || h != height()) {
        if (m_adapter && m_adapter -> undo()) {
            if (cropLayers)
                m_adapter->beginMacro("Crop Image");
            else
                m_adapter -> beginMacro("Resize Image");

            m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));
        }

        m_width = w;
        m_height = h;

        if (cropLayers) {
            KisCropVisitor v(QRect(x, y, w, h));
            m_rootLayer->accept(v);

        }

        if (m_adapter && m_adapter -> undo()) {
            m_adapter -> endMacro();
        }

        emit sigSizeChanged(w, h);
    }
}

void KisImage::resize(const QRect& rc, bool cropLayers)
{
    resize(rc.width(), rc.height(), rc.x(), rc.y(), cropLayers);
}


void KisImage::scale(double sx, double sy, KisProgressDisplayInterface *progress, KisFilterStrategy *filterStrategy)
{
    if (nlayers() == 0) return; // Nothing to scale

    // New image size. XXX: Pass along to discourage rounding errors?
    Q_INT32 w, h;
    w = (Q_INT32)(( width() * sx) + 0.5);
    h = (Q_INT32)(( height() * sy) + 0.5);

    if (w != width() || h != height()) {

        if (m_adapter && m_adapter -> undo()) {
            m_adapter->beginMacro("Scale image");
        }

        KisScaleVisitor visitor (this, sx, sy, progress, filterStrategy);
        m_rootLayer->accept(visitor);

        if (m_adapter && m_adapter -> undo()) {
            m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));
        }

        m_width = w;
        m_height = h;

        if (m_adapter && m_adapter -> undo()) {
            m_adapter->endMacro();
        }
        emit sigSizeChanged(w, h);
        notify();

    }
}

void KisImage::rotate(double , KisProgressDisplayInterface *)
{
/*LAYERREMOVE
void KisImage::rotate(double angle, KisProgressDisplayInterface *m_progress)
{
    const double pi=3.1415926535897932385;

    if (m_layers.empty()) return; // Nothing to scale

    Q_INT32 w, h;
    w = (Q_INT32)(width()*QABS(cos(angle*pi/180)) + height()*QABS(sin(angle*pi/180)) + 0.5);
    h = (Q_INT32)(height()*QABS(cos(angle*pi/180)) + width()*QABS(sin(angle*pi/180)) + 0.5);

    Q_INT32 oldCentreToNewCentreXOffset = (w - width()) / 2;
    Q_INT32 oldCentreToNewCentreYOffset = (h - height()) / 2;

    m_adapter->beginMacro("Rotate image");

    vKisLayerSP_it it;
    for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
        KisLayerSP layer = (*it);

        KisTransaction * t = 0;
        if (undoAdapter() && m_adapter->undo()) {
            t = new KisTransaction("", layer.data());
            Q_CHECK_PTR(t);
        }

        layer -> rotate(angle, true, m_progress);

        if (t) {
            m_adapter->addCommand(t);
        }

        //XXX: This is very ugly.
        KNamedCommand *moveCommand = layer -> moveCommand(layer -> getX() + oldCentreToNewCentreXOffset,
                                                          layer -> getY() + oldCentreToNewCentreYOffset);
        if (undoAdapter() && m_adapter->undo()) {
            m_adapter->addCommand(moveCommand);
        } else {
            delete moveCommand;
        }
    }

    m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));

    m_width = w;
    m_height = h;

    undoAdapter()->endMacro();

    emit sigSizeChanged(KisImageSP(this), w, h);
    notify();
*/
}

void KisImage::shear(double , double , KisProgressDisplayInterface *)
{
/*LAYERREMOVE
void KisImage::shear(double angleX, double angleY, KisProgressDisplayInterface *m_progress)
{
    const double pi=3.1415926535897932385;

    if (m_layers.empty()) return; // Nothing to scale

    //new image size
    Q_INT32 w=width();
    Q_INT32 h=height();


    if(angleX != 0 || angleY != 0){
        double deltaY=height()*QABS(tan(angleX*pi/180)*tan(angleY*pi/180));
        w = (Q_INT32) ( width() + QABS(height()*tan(angleX*pi/180)) );
        //ugly fix for the problem of having two extra pixels if only a shear along one
        //axis is done. This has to be fixed in the cropping code in KisRotateVisitor!
        if (angleX == 0 || angleY == 0)
            h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180)) );
        else if (angleX > 0 && angleY > 0)
            h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180))- 2 * deltaY + 2 );
        else if (angleX < 0 && angleY < 0)
            h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180))- 2 * deltaY + 2 );
        else
            h = (Q_INT32) ( height() + QABS(w*tan(angleY*pi/180)) );
    }

    if (w != width() || h != height()) {

        m_adapter->beginMacro("Shear image");

        vKisLayerSP_it it;
        for ( it = m_layers.begin(); it != m_layers.end(); ++it ) {
            KisLayerSP layer = (*it);

            KisTransaction * t = 0;
            if (undoAdapter() && m_adapter->undo()) {
                t = new KisTransaction("", layer.data());
                Q_CHECK_PTR(t);
            }

            layer -> shear(angleX, angleY, m_progress);

            if (t) {
                m_adapter->addCommand(t);
            }

        }

        m_adapter->addCommand(new KisResizeImageCmd(m_adapter, this, w, h, width(), height()));

        m_width = w;
        m_height = h;

        undoAdapter()->endMacro();

        emit sigSizeChanged(KisImageSP(this), w, h);
        notify();
    }
*/
}

void KisImage::convertTo(KisColorSpace * dstColorSpace, Q_INT32 renderingIntent)
{
    if ( m_colorSpace == dstColorSpace )
    {
//         kdDebug(DBG_AREA_CORE) << "KisImage: NOT GOING TO CONVERT\n";
        return;
    }

    if (undoAdapter() && m_adapter->undo()) {
        m_adapter->beginMacro(i18n("Convert Image Type"));
    }

    setColorSpace(dstColorSpace);

    KisColorSpaceConvertVisitor visitor(dstColorSpace, renderingIntent);
    m_rootLayer->accept(visitor);

    if (undoAdapter() && m_adapter->undo()) {

        m_adapter->addCommand(new KisConvertImageTypeCmd(undoAdapter(), this,
                                                         m_colorSpace, dstColorSpace));
        m_adapter->endMacro();
    }
    emit sigLayerPropertiesChanged( m_activeLayer );
    emit sigNonActiveLayersUpdated(); // This makes sure the
                                      // thumbnails are updated
}

KisProfile *  KisImage::getProfile() const
{
    return colorSpace()->getProfile();
}

void KisImage::setProfile(const KisProfile * profile)
{
    KisColorSpace * dstSpace = KisMetaRegistry::instance()->csRegistry()->getColorSpace( colorSpace()->id(), profile);
    //convertTo( dstSpace ); // XXX: We shouldn't convert here -- if you want to convert, use the conversion function.
    setColorSpace(dstSpace);
    emit(sigProfileChanged(const_cast<KisProfile *>(profile)));
}

double KisImage::xRes()
{
    return m_xres;
}

double KisImage::yRes()
{
    return m_yres;
}

void KisImage::setResolution(double xres, double yres)
{
    m_xres = xres;
    m_yres = yres;
}

Q_INT32 KisImage::width() const
{
    return m_width;
}

Q_INT32 KisImage::height() const
{
    return m_height;
}

KisPaintDeviceSP KisImage::activeDevice()
{
    if (KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_activeLayer.data())) {
        return layer -> paintDevice();
    }
    else if (KisAdjustmentLayer* layer = dynamic_cast<KisAdjustmentLayer*>(m_activeLayer.data())) {
        if (layer->selection()) {
            return layer -> selection().data();
        }
    }
    return 0;
}

KisLayerSP KisImage::newLayer(const QString& name, Q_UINT8 opacity, const KisCompositeOp& compositeOp, KisColorSpace * colorstrategy)
{
    KisLayerSP layer;
    if (colorstrategy)
        layer = new KisPaintLayer(this, name, opacity, colorstrategy);
    else
        layer = new KisPaintLayer(this, name, opacity);
    Q_CHECK_PTR(layer);

    if (compositeOp.isValid())
        layer -> setCompositeOp(compositeOp);
    layer -> setVisible(true);

    if (m_activeLayer != 0) {
        addLayer(layer, m_activeLayer->parent().data(), m_activeLayer->nextSibling());
    }
    else {
        addLayer(layer, m_rootLayer, 0);
    }
    activate(layer);

    return layer;
}

void KisImage::setLayerProperties(KisLayerSP layer, Q_UINT8 opacity, const KisCompositeOp& compositeOp, const QString& name)
{
    if (layer && (layer->opacity() != opacity || layer->compositeOp() != compositeOp || layer->name() != name)) {
        if (m_adapter->undo()) {
            QString oldname = layer -> name();
            Q_INT32 oldopacity = layer -> opacity();
            KisCompositeOp oldCompositeOp = layer -> compositeOp();
            layer -> setName(name);
            layer -> setOpacity(opacity);
            layer -> setCompositeOp(compositeOp);
            m_adapter->addCommand(new LayerPropsCmd(layer, this, m_adapter, oldname, oldopacity, oldCompositeOp));
        } else {
            layer -> setName(name);
            layer -> setOpacity(opacity);
            layer -> setCompositeOp(compositeOp);
        }
    }
}

KisGroupLayerSP KisImage::rootLayer() const
{
    return m_rootLayer;
}

KisLayerSP KisImage::activeLayer() const
{
    return m_activeLayer;
}

KisPaintDeviceSP KisImage::projection() const
{
    return m_rootLayer->projection();
}

KisLayerSP KisImage::activate(KisLayerSP layer)
{
    if (layer != m_activeLayer) {
        if (m_activeLayer) m_activeLayer->deactivate();
        m_activeLayer = layer;
        if (m_activeLayer) m_activeLayer->activate();
        emit sigLayerActivated(m_activeLayer);
    }

    return layer;
}

KisLayerSP KisImage::findLayer(const QString& name) const
{
    return rootLayer() -> findLayer(name);
}

KisLayerSP KisImage::findLayer(int id) const
{
    return rootLayer() -> findLayer(id);
}


bool KisImage::addLayer(KisLayerSP layer, KisGroupLayerSP parent)
{
    return addLayer(layer, parent, parent->firstChild());
}

bool KisImage::addLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis)
{
    if (!parent)
        return false;

    const bool success = parent->addLayer(layer, aboveThis);
    if (success)
    {
        if (!layer->temporary() && m_adapter && m_adapter->undo()) {
            m_adapter->addCommand(new LayerAddCmd(m_adapter, this, layer));
        }
        KisPaintLayerSP player = dynamic_cast<KisPaintLayer*>(layer.data());
        if (player != 0) {

            QValueVector<KisPaintDeviceAction *> actions = KisMetaRegistry::instance() ->
                csRegistry() -> paintDeviceActionsFor(player -> paintDevice() -> colorSpace());
            for (uint i = 0; i < actions.count(); i++) {
                actions.at(i) -> act(player.data() -> paintDevice(), width(), height());
            }
        }

        if (!layer->temporary()) {
            emit sigLayerAdded(layer);
            activate(layer);
        }
        layer->setDirty(true);
    }

    return success;
}

bool KisImage::removeLayer(KisLayerSP layer)
{
    if (!layer || layer -> image() != this)
        return false;

    if (KisGroupLayerSP parent = layer -> parent()) {

        KisLayerSP wasAbove = layer -> nextSibling();
        KisLayerSP wasBelow = layer -> prevSibling();
        const bool wasActive = layer == activeLayer();
        const bool success = parent -> removeLayer(layer);
        if (success) {
            layer -> setImage(0);
            if (!layer->temporary() && m_adapter->undo()) {
                m_adapter->addCommand(new LayerRmCmd(m_adapter, this, layer, parent, wasAbove));
            }
            if (!layer->temporary()) {
                emit sigLayerRemoved(layer, parent, wasAbove);
                if (wasActive) {
                    if (wasBelow)
                        activate(wasBelow);
                    else if (wasAbove)
                        activate(wasAbove);
                    else if (parent != rootLayer())
                        activate(parent.data());
                    else
                        activate(rootLayer() -> firstChild());
                }
            }
        }
        return success;
    }

    return false;
}

bool KisImage::raiseLayer(KisLayerSP layer)
{
    if (!layer)
        return false;
    return moveLayer(layer, layer -> parent().data(), layer -> prevSibling());
}

bool KisImage::lowerLayer(KisLayerSP layer)
{
    if (!layer)
        return false;
    if (KisLayerSP next = layer -> nextSibling())
        return moveLayer(layer, layer -> parent().data(), next -> nextSibling());
    return false;
}

bool KisImage::toTop(KisLayerSP layer)
{
    if (!layer)
        return false;
    return moveLayer(layer, rootLayer(), rootLayer() -> firstChild());
}

bool KisImage::toBottom(KisLayerSP layer)
{
    if (!layer)
        return false;
    return moveLayer(layer, rootLayer(), 0);
}

bool KisImage::moveLayer(KisLayerSP layer, KisGroupLayerSP parent, KisLayerSP aboveThis)
{
    if (!parent)
        return false;

    KisGroupLayerSP wasParent = layer -> parent();
    KisLayerSP wasAbove = layer -> nextSibling();

    if (wasParent.data() == parent.data() && wasAbove.data() == aboveThis.data())
        return false;

    if (!wasParent -> removeLayer(layer))
        return false;

    const bool success = parent -> addLayer(layer, aboveThis);
    if (success)
    {
        if (m_adapter->undo())
            m_adapter->addCommand(new LayerMoveCmd(m_adapter, this, layer, wasParent, wasAbove));
        emit sigLayerMoved(layer, wasParent, wasAbove);
        notify();
    }
    else //we already removed the layer above, but re-adding it failed, so...
    {
        if (m_adapter->undo())
            m_adapter->addCommand(new LayerRmCmd(m_adapter, this, layer, wasParent, wasAbove));
        emit sigLayerRemoved(layer, wasParent, wasAbove);
        notify();
    }

    layer->setDirty(true);
    return success;
}

Q_INT32 KisImage::nlayers() const
{
    return rootLayer() -> numLayers() - 1;
}

Q_INT32 KisImage::nHiddenLayers() const
{
    return rootLayer() -> numLayers(KisLayer::Hidden);
}

void KisImage::flatten()
{
    KisGroupLayerSP oldRootLayer = m_rootLayer;

    KisPaintLayer *dst = new KisPaintLayer(this, nextLayerName(), OPACITY_OPAQUE, colorSpace());
    Q_CHECK_PTR(dst);

    QRect rc = mergedImage() -> extent();

    KisPainter gc(dst->paintDevice());
    gc.bitBlt(0, 0, COMPOSITE_COPY, mergedImage(), OPACITY_OPAQUE, rc.left(), rc.top(), rc.width(), rc.height());

    m_rootLayer = new KisGroupLayer(this, "", OPACITY_OPAQUE);

    blockSignals(true);
    addLayer(dst, m_rootLayer, 0);
    activate(dst);
    blockSignals(false);

    notifyLayersChanged();
    notify();

    if (m_adapter && m_adapter -> undo()) {
        m_adapter->addCommand(new KisChangeLayersCmd(m_adapter, this, oldRootLayer, m_rootLayer, i18n("Flatten Image")));
    }
}


void KisImage::mergeLayer(KisLayerSP /*l*/)
{
/*
    vKisLayerSP beforeLayers = m_layers;

    KisLayerSP dst = new KisLayer(this, l -> name(), OPACITY_OPAQUE);
    Q_CHECK_PTR(dst);

    KisFillPainter painter(dst.data());

    KisMerge<All, All> visitor(this);
    visitor(painter, layer(index(l) + 1));
    visitor(painter, l);

    int insertIndex = -1;

    if (visitor.insertMergedAboveLayer() != 0) {
        insertIndex = index(visitor.insertMergedAboveLayer());
    }

    add(dst, insertIndex);

    notifyLayersChanged();
    notify();

    if (m_adapter && m_adapter -> undo())
    {
        m_adapter->addCommand(new KisChangeLayersCmd(m_adapter, this, beforeLayers, m_layers, i18n("&Merge Layers")));
//XXX fix name after string freeze
    }
*/
}


void KisImage::enableUndo(KoCommandHistory *history)
{
    m_undoHistory = history;
}

void KisImage::setModified()
{
    emit sigImageModified();
}

void KisImage::renderToPainter(Q_INT32 x1,
                               Q_INT32 y1,
                               Q_INT32 x2,
                               Q_INT32 y2,
                               QPainter &painter,
                               KisProfile *  monitorProfile,
                               PaintFlags paintFlags,
                               float exposure)
{


    Q_INT32 w = x2 - x1 + 1;
    Q_INT32 h = y2 - y1 + 1;

    if (!m_renderinit) {
        QRect rc(x1, y1, w, h);
        rc &= bounds();
        updateProjection(rc);
    }

    QImage img = m_rootLayer->projection()->convertToQImage(monitorProfile, x1, y1, w, h, exposure);

#ifdef __BIG_ENDIAN__
        //cmsDoTransform(m_private->bigEndianTransform, img.bits(), img.bits(), w * h);
	uchar * data = img.bits();
	for (int i = 0; i < w * h; ++i) {
	    uchar r, g, b, a;
	    a = data[0];
	    b = data[1];
	    g = data[2];
	    r = data[3];
	    data[0] = r;
	    data[1] = g;
	    data[2] = b;
	    data[3] = a;
	    data += 4;
	}
#endif

    if (paintFlags & PAINT_BACKGROUND) {
        m_bkg -> paintBackground(img, x1, y1);
        img.setAlphaBuffer(false);
    }

    if (paintFlags & PAINT_SELECTION) {
        if (m_activeLayer != 0) {
            m_activeLayer -> paintSelection(img, x1, y1, w, h);
        }
    }

    if (paintFlags & PAINT_MASKINACTIVELAYERS) {
        if (m_activeLayer != 0) {
            m_activeLayer -> paintMaskInactiveLayers(img, x1, y1, w, h);
        }
    }

    painter.drawImage(x1, y1, img, 0, 0, w, h);
}

QImage KisImage::convertToQImage(Q_INT32 x1,
                                 Q_INT32 y1,
                                 Q_INT32 x2,
                                 Q_INT32 y2,
                                 KisProfile * profile,
                                 float exposure)
{
    Q_INT32 w = x2 - x1 + 1;
    Q_INT32 h = y2 - y1 + 1;

    QImage img = m_rootLayer->projection()->convertToQImage(profile, x1, y1, w, h, exposure);

    if (!img.isNull()) {

#ifdef __BIG_ENDIAN__
        //cmsDoTransform(m_private->bigEndianTransform, img.bits(), img.bits(), w * h);
        uchar * data = img.bits();
        for (int i = 0; i < w * h; ++i) {
            uchar r, g, b, a;
            a = data[0];
            b = data[1];
            g = data[2];
            r = data[3];
            data[0] = r;
            data[1] = g;
            data[2] = b;
            data[3] = a;
            data += 4;
        }        
#endif
        return img;
    }

    return QImage();
}

KisPaintDeviceSP KisImage::mergedImage()
{
    return m_rootLayer->projection();
}

KisColor KisImage::mergedPixel(Q_INT32 x, Q_INT32 y)
{
    return m_rootLayer->projection()->colorAt(x, y);
}

void KisImage::updateProjection(const QRect& rc)
{
    QRect rect = rc & QRect(0, 0, width(), height());

    KisMergeVisitor visitor(this, m_rootLayer->projection(), rc);
    m_rootLayer -> accept(visitor);

    m_renderinit = true;
}

void KisImage::notify()
{
    notify(QRect(0, 0, width(), height()));
}

void KisImage::notify(const QRect& rc)
{
    QRect rect = rc & QRect(0, 0, width(), height());
    if (m_activeLayer)
        m_activeLayer->setDirty(true); // Shouldn't do that here, but where we modify each layer.
    updateProjection(rect);

    if (rect.isValid()) {
        emit sigImageUpdated(rect);
    }
}

void KisImage::notifyLayersChanged()
{
    emit sigLayersChanged(rootLayer());
}

void KisImage::notifyPropertyChanged(KisLayerSP layer)
{
    emit sigLayerPropertiesChanged(layer);
}

void KisImage::notifyImageLoaded()
{
    emit sigNonActiveLayersUpdated();
}

QRect KisImage::bounds() const
{
    return QRect(0, 0, width(), height());
}


void KisImage::setUndoAdapter(KisUndoAdapter * adapter)
{
    m_adapter = adapter;
}


KisUndoAdapter* KisImage::undoAdapter() const
{
    return m_adapter;
}


//KisGuideMgr *KisImage::guides() const
//{
//    return const_cast<KisGuideMgr*>(&m_guides);
//}

void KisImage::slotSelectionChanged()
{
//     kdDebug(DBG_AREA_CORE) << "KisImage::slotSelectionChanged\n";
    emit sigActiveSelectionChanged(KisImageSP(this));
    notify();
}

void KisImage::slotSelectionChanged(const QRect& r)
{
//     kdDebug(DBG_AREA_CORE) << "KisImage::slotSelectionChanged rect\n";
    QRect r2(r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2);

    emit sigActiveSelectionChanged(KisImageSP(this));
    notify(r2);
}

KisColorSpace * KisImage::colorSpace() const
{
    return m_colorSpace;
}

void KisImage::setColorSpace(KisColorSpace * colorSpace)
{
    m_colorSpace = colorSpace;
    emit sigColorSpaceChanged(colorSpace);
}

void KisImage::setRootLayer(KisGroupLayerSP rootLayer)
{
    m_rootLayer = rootLayer;
}

void KisImage::addAnnotation(KisAnnotationSP annotation)
{
    // Find the icc annotation, if there is one
    vKisAnnotationSP_it it = m_annotations.begin();
    while (it != m_annotations.end()) {
        if ((*it) -> type() == annotation -> type()) {
            *it = annotation;
            return;
        }
        ++it;
    }
    m_annotations.push_back(annotation);
}

KisAnnotationSP KisImage::annotation(QString type)
{
    vKisAnnotationSP_it it = m_annotations.begin();
    while (it != m_annotations.end()) {
        if ((*it) -> type() == type) {
            return *it;
        }
        ++it;
    }
    return 0;
}

void KisImage::removeAnnotation(QString type)
{
    vKisAnnotationSP_it it = m_annotations.begin();
    while (it != m_annotations.end()) {
        if ((*it) -> type() == type) {
            m_annotations.erase(it);
            return;
        }
        ++it;
    }
}

vKisAnnotationSP_it KisImage::beginAnnotations()
{
    KisProfile * profile = colorSpace()->getProfile();
    KisAnnotationSP annotation;

    if (profile)
        annotation =  profile -> annotation();

    if (annotation)
         addAnnotation(annotation);
    else
        removeAnnotation("icc");

    return m_annotations.begin();
}

vKisAnnotationSP_it KisImage::endAnnotations()
{
    return m_annotations.end();
}

KisBackgroundSP KisImage::background() const
{
    return m_bkg;
}
#include "kis_image.moc"

