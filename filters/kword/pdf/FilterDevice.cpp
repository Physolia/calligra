/*
 * Copyright (c) 2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "FilterDevice.h"

#include <math.h>
#include <koFilterChain.h>
#include <kdebug.h>

#include "GfxState.h"
#include "Link.h"

#include "FilterPage.h"
#include "data.h"
#include "dialog.h"


namespace PDFImport
{

Device::Device(Data &data)
    : _data(data), _fillColor(Qt::white), _strokeColor(Qt::black)
{
    _pages.setAutoDelete(true);
}

Device::~Device()
{
    clear();
}

void Device::clear()
{
    _images.clear();
}

void Device::dumpPage(uint i)
{
    _data.startPage();
    _pages.at(i)->dump();
}

void Device::startPage(int, GfxState *)
{
    _pages.append( new Page(_data) );
}

void Device::endPage()
{
    if ( !_currentImage.image.isNull() ) addImage();
    _data.endPage( _pages.getLast()->rect() );
    clear();
    kdDebug(30516) << "-- end page --------------------------" << endl;
}

void Device::updateFont(GfxState *state)
{
    _pages.getLast()->updateFont(state);
}

void Device::beginString(GfxState *state, GString *)
{
    _pages.getLast()->beginString(state, state->getCurX(), state->getCurY());
}

void Device::endString(GfxState *)
{
    _pages.getLast()->endString();
}

void Device::drawChar(GfxState *state, double x, double y,
                            double dx, double dy, double, double,
                            CharCode, Unicode *u, int uLen)
{
    _pages.getLast()->addChar(state, x, y, dx, dy, u, uLen);
}

void Device::drawLink(::Link* link, Catalog *cat)
{
    double x1, y1, x2, y2, w;
    link->getBorder(&x1, &y1, &x2, &y2, &w);

    int ux1, uy1, ux2, uy2;
    cvtUserToDev(x1, y1, &ux1, &uy1);
    cvtUserToDev(x2, y2, &ux2, &uy2);

    DRect r;
    r.left = kMin(ux1, ux2);
    r.right = kMax(ux1, ux2);
    r.top = kMin(uy1, uy2);
    r.bottom = kMax(uy1, uy2);
    Link *l = new Link(r, *link->getAction(), *cat);
    _pages.getLast()->addLink(l);
}

void Device::addImage()
{
    if ( _currentImage.image.width()==0 || _currentImage.image.height()==0 ) {
        kdDebug(30516) << "image has null width or height !" << endl;
        _currentImage = Image();
        return;
    }

    // check if same image already put at same place (don't know why it
    // appends sometimes : related to KWord printing to pdf ?)
    ImageList::iterator it;
    for (it=_images.begin(); it!=_images.end(); ++it) {
        if ( (*it).rect==_currentImage.rect
             && (*it).image==_currentImage.image ) {
            kdDebug(30516) << "image already there !\n";
            _currentImage = Image();
            return;
        }
    }

    // add image
    QString name = QString("pictures/picture%1.png").arg(_data.imageIndex());
    QDomElement frameset = _data.pictureFrameset(_currentImage.rect);
    QDomElement picture = _data.createElement("PICTURE");
    picture.setAttribute("keepAspectRatio", "false");
    frameset.appendChild(picture);

    QDomElement key = _data.createElement("KEY");
    key.setAttribute("msec", 0);
    key.setAttribute("second", 0);
    key.setAttribute("minute", 0);
    key.setAttribute("hour", 0);
    key.setAttribute("day", 1);
    key.setAttribute("month", 1);
    key.setAttribute("year", 1970);
    key.setAttribute("filename", name);
    picture.appendChild(key);

    key = _data.createElement("KEY");
    key.setAttribute("msec", 0);
    key.setAttribute("second", 0);
    key.setAttribute("minute", 0);
    key.setAttribute("hour", 0);
    key.setAttribute("day", 1);
    key.setAttribute("month", 1);
    key.setAttribute("year", 1970);
    key.setAttribute("filename", name);
    key.setAttribute("name", name);
    _data.pictures().appendChild(key);

    KoStoreDevice *sd = _data.chain()->storageFile(name, KoStore::Write);
    QImageIO io(sd, "PNG");
    io.setImage(_currentImage.image);
    bool ok = io.write();
    Q_ASSERT(ok);
    sd->close();

    _images.append(_currentImage);
    _currentImage = Image();
}

void Device::computeGeometry(GfxState *state, Image &image)
{
    double xt, yt, wt, ht;
    state->transform(0, 0, &xt, &yt);
    state->transformDelta(1, 1, &wt, &ht);
    image.rect.left = xt + (wt>0 ? 0 : wt);
    image.rect.right = image.rect.left + fabs(wt);
    image.rect.top = yt + (ht>0 ? 0 : ht);
    image.rect.bottom = image.rect.top + fabs(ht);

    // #### TODO : take care of image transform (rotation,...)
}

uint Device::initImage(GfxState *state, int width, int height,
                             bool withMask)
{
    // get image geometry
    Image image;
    image.mask = withMask;
    computeGeometry(state, image);

    // check if new image
    if ( !_currentImage.image.isNull() &&
         (_currentImage.image.width()!=width
          || !equal(image.rect.left, _currentImage.rect.left)
          || !equal(image.rect.right, _currentImage.rect.right)
          || !equal(image.rect.top, _currentImage.rect.bottom)
          || !equal(image.mask, _currentImage.mask)) )
        addImage();

    uint offset =
        (_currentImage.image.isNull() ? 0 : _currentImage.image.height());
    image.image = QImage(width, offset + height, 32);
    image.image.setAlphaBuffer(withMask);
    if ( !_currentImage.image.isNull() ) { // copy previous
        for (int j=0; j<_currentImage.image.height(); j++) {
            QRgb *pix = (QRgb *)_currentImage.image.scanLine(j);
            QRgb *newPix = (QRgb *)image.image.scanLine(j);
            for (int i=0; i<width; i++) newPix[i] = pix[i];
        }
        _currentImage.image = image.image;
        _currentImage.rect.bottom = image.rect.bottom;
    } else _currentImage = image;
    return offset;
}

void Device::drawImage(GfxState *state, Object *, Stream *str,
                             int width, int height, GfxImageColorMap *colorMap,
                             int *maskColors, GBool inlineImg)
{
    kdDebug(30516) << "image kind=" << str->getKind()
                   << " inline=" << inlineImg
                   << " maskColors=" << (maskColors!=0) << endl;
    if ( !_data.options().importImages ) return;

    uint offset = initImage(state, width, height, maskColors!=0);

    // read pixels
    int nbComps = colorMap->getNumPixelComps();
    int nbBits = colorMap->getBits();
    ImageStream *istr = new ImageStream(str, width, nbComps, nbBits);
    istr->reset();
    for (int j=0; j<height; j++) {
        Guchar *p = istr->getLine();
        QRgb *pix = (QRgb *)_currentImage.image.scanLine(offset + j);
        for (int i=0; i<width; i++) {
            GfxRGB rgb;
            colorMap->getRGB(p, &rgb);
            int alpha = 255;
            if (maskColors) {
                for (int k=0; k<nbComps; k++)
                    if ( p[k]<maskColors[2*k] || p[k]>maskColors[2*k+1] ) {
                        alpha = 0;
                        break;
                    }
            }
            pix[i] = qRgba(qRound(rgb.r*255), qRound(rgb.g*255),
                           qRound(rgb.b*255), alpha);
            p += nbComps;
        }
    }
    delete istr;
}

void Device::drawImageMask(GfxState *state, Object *, Stream *str,
                                 int width, int height, GBool invert,
                                 GBool inlineImg)
{
    kdDebug(30516) << "image mask ! kind=" << str->getKind()
                   << "inline=" << inlineImg << endl;
    if ( !_data.options().importImages ) return;

    uint offset = initImage(state, width, height, true);

    // read pixels
    GfxRGB rgb;
    state->getFillRGB(&rgb);
    int red = qRound(rgb.r * 255);
    int green = qRound(rgb.g * 255);
    int blue = qRound(rgb.b * 255);

    ImageStream *istr = new ImageStream(str, width, 1, 1);
    str->reset();
    for (int j=0; j<height; j++) {
        Guchar *p = istr->getLine();
        QRgb *pix = (QRgb *)_currentImage.image.scanLine(offset + j);
        for (int i=0; i<width; i++)
            pix[i] = qRgba(red, green, blue, 255 * p[i]);
    }
    delete istr;

    if (invert) _currentImage.image.invertPixels();
}

void Device::updateAll(GfxState *state)
{
    updateFillColor(state);
    updateStrokeColor(state);
    updateFont(state);
}

void Device::updateFillColor(GfxState *state)
{
    GfxRGB rgb;
    state->getFillRGB(&rgb);
    _fillColor = toColor(rgb);
}

void Device::updateStrokeColor(GfxState *state)
{
    GfxRGB rgb;
    state->getStrokeRGB(&rgb);
    _strokeColor = toColor(rgb);
}

void Device::stroke(GfxState */*state*/)
{
//    kdDebug(30516) << "stroke" << endl;
//    DPathVector path = convertPath(state);
//    for (uint i=0; i<path.size(); i++) {
//        if ( path[i].isHorizontalSegment() ) {
//            kdDebug(30516) << "  horizontal segment" << endl;
            // #### FIXME correctly draw the line
//            if ( !_currentImage.image.isNull() ) addImage();
//            _currentImage.rect = path[i].boundingRect();
//            _currentImage.rect.bottom+=1;
//            _currentImage.image =
//                QImage(qRound(_currentImage.rect.width()),
//                       qRound(_currentImage.rect.height()), 32);
//            _currentImage.image.fill(_fillColor.pixel());
//            addImage();
//        } else if ( path[i].isVerticalSegment() ) {
//            kdDebug(30516) << "  vertical segment" << endl;
//        }
//    }
}

void Device::fill(GfxState */*state*/)
{
//    kdDebug(30516) << "fill" << endl;
//    doFill(state);
}

void Device::eoFill(GfxState */*state*/)
{
//    kdDebug(30516) << "eoFill" << endl;
//    convertPath(state);
//    doFill(state);
}

void Device::doFill(const DPathVector &path)
{
    for (uint i=0; i<path.size(); i++) {
        if ( path[i].isSegment() ) continue;
        if ( path[i].isRectangle() ) {
            kdDebug(30516) << "fill rectangle" << endl;
            if ( !_currentImage.image.isNull() ) addImage();
            _currentImage.rect = path[i].boundingRect();
            _currentImage.image =
                QImage(qRound(_currentImage.rect.width()),
                       qRound(_currentImage.rect.height()), 32);
            _currentImage.image.fill(_fillColor.pixel());
            addImage();
        }
    }
}

DPathVector Device::convertPath(GfxState *state)
{
    GfxPath *path = state->getPath();
    uint nbPaths = path->getNumSubpaths();
    DPathVector vector;
    for (uint i=0; i<nbPaths; i++) {
        GfxSubpath *spath = path->getSubpath(i);
        uint nbPoints = spath->getNumPoints();
        DPath dpath;
        for (uint k=0; k<nbPoints; k++) {
            if ( k>=1 && spath->getCurve(k) ) {
                kdDebug(30516) << "    bezier curve : ignore !" << endl;
                dpath = DPath();
                break;
            } else {
                DPoint dpoint;
                state->transform(spath->getX(k), spath->getY(k),
                                 &dpoint.x, &dpoint.y);
                dpath.push_back(dpoint);
            }
        }
        if ( dpath.size()!=0 ) vector.push_back(dpath);
    }
    return vector;
}

}; // namespace
