/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
 
#include "kis_jpeg_converter.h"

#include <stdio.h>

#include <kapplication.h>
#include <koDocumentInfo.h>

#include <kio/netaccess.h>

#include <kis_abstract_colorspace.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_paint_layer.h>
#include <kis_meta_registry.h>
#include <kis_profile.h>

namespace {
    
    J_COLOR_SPACE getColorTypeforColorSpace( KisColorSpace * cs)
    {
        if ( cs->id() == KisID("GRAYA") || cs->id() == KisID("GRAYA16") )
        {
            return JCS_GRAYSCALE;
        }
        if ( cs->id() == KisID("RGBA") || cs->id() == KisID("RGBA16") )
        {
            return JCS_RGB;
        }
        if ( cs->id() == KisID("CMYK") || cs->id() == KisID("CMYK16") )
        {
            return JCS_CMYK;
        }
        kdDebug() << "Cannot export images in " + cs->id().name() + " yet.\n";
        return JCS_UNKNOWN;
    }

    QString getColorSpaceForColorType(J_COLOR_SPACE color_type) {
        if(color_type == JCS_GRAYSCALE)
        {
            return "GRAYA";
        } else if(color_type == JCS_RGB) {
            return "RGBA";
        } else if(color_type == JCS_CMYK) {
            return "CMYK";
        }
        return "";
    }

}

KisJPEGConverter::KisJPEGConverter(KisDoc *doc, KisUndoAdapter *adapter)
{
    m_doc = doc;
    m_adapter = adapter;
    m_job = 0;
    m_stop = false;
}

KisJPEGConverter::~KisJPEGConverter()
{
}

KisImageBuilder_Result KisJPEGConverter::decode(const KURL& uri)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    
    // open the file
    FILE *fp = fopen(uri.path().ascii(), "rb");
    if (!fp)
    {
        return (KisImageBuilder_RESULT_NOT_EXIST);
    }
    jpeg_stdio_src(&cinfo, fp);
    
    // read header
    jpeg_read_header(&cinfo, TRUE);
    
    // start reading
    jpeg_start_decompress(&cinfo);
    
    // Get the colorspace
    QString csName = getColorSpaceForColorType(cinfo.out_color_space);
    if(csName == "") {
        kdDebug() << "unsupported colorspace : " << cinfo.out_color_space << endl;
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    
    // TODO: add support for profile in jpeg
#if 0
    // Read image profile
    png_charp profile_name, profile_data;
    int compression_type;
    png_uint_32 proflen;
    
    png_get_iCCP(png_ptr, info_ptr, &profile_name, &compression_type, &profile_data, &proflen);
#endif
    KisProfile* profile = 0;
#if 0
    QByteArray profile_rawdata;
    // XXX: Hardcoded for icc type -- is that correct for us?
    if (QString::compare(profile_name, "icc") == 0) {
       
        profile_rawdata.resize(proflen);
        memcpy(profile_rawdata.data(), profile_data, proflen);

        cmsHPROFILE hProfile = cmsOpenProfileFromMem(profile_data, (DWORD)proflen);

        if (hProfile != (cmsHPROFILE) NULL) {
            profile = new KisProfile(hProfile, profile_rawdata);
            Q_CHECK_PTR(profile);
        }
    }
#endif

    // Retrieve a pointer to the colorspace
    KisColorSpace* cs;
    if (profile)
    {
        kdDebug() << "image has embedded profile: " << profile -> productName() << "\n";
        cs = KisMetaRegistry::instance()->csRegistry()->getColorSpace(csName, profile);
    }
    else
        cs = KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID(csName,""),"");

    if(cs == 0)
    {
        kdDebug() << "unknown colorspace" << endl;
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    
    // Creating the KisImageSP
    if( ! m_img) {
        m_img = new KisImage(m_doc->undoAdapter(),  cinfo.image_width,  cinfo.image_height, cs, "built image");
        Q_CHECK_PTR(m_img);
        if(profile)
        {
//             m_img -> addAnnotation( new KisAnnotation(profile_name, "", profile_rawdata) );
        }
    }

    KisPaintLayerSP layer = new KisPaintLayer(m_img, m_img -> nextLayerName(), Q_UINT8_MAX);
    m_img->addLayer(layer.data(), m_img->rootLayer(), 0);
    // Read data
    JSAMPROW row_pointer = new JSAMPLE[cinfo.image_width*cinfo.num_components];
        
    for (; cinfo.output_scanline < cinfo.image_height;) {
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, cinfo.output_scanline, cinfo.image_width, true);
        jpeg_read_scanlines(&cinfo, &row_pointer, 1);
        Q_UINT8 *src = row_pointer;
        switch(cinfo.out_color_space)
        {
            case JCS_GRAYSCALE:
                while (!it.isDone()) {
                    Q_UINT8 *d = it.rawData();
                    d[0] = *(src++);
                    d[1] = Q_UINT8_MAX;
                    ++it;
                }
                break;
            case JCS_RGB:
                while (!it.isDone()) {
                    Q_UINT8 *d = it.rawData();
                    d[2] = *(src++);
                    d[1] = *(src++);
                    d[0] = *(src++);
                    d[3] = Q_UINT8_MAX;
                    ++it;
                }
                break;
            case JCS_CMYK:
                while (!it.isDone()) {
                    Q_UINT8 *d = it.rawData();
                    d[0] = *(src++);
                    d[1] = *(src++);
                    d[2] = *(src++);
                    d[3] = *(src++);
                    d[4] = Q_UINT8_MAX;
                    ++it;
                }
                break;
            default:
                return KisImageBuilder_RESULT_UNSUPPORTED;
        }
    }
    

    
    // Finish decompression
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    return KisImageBuilder_RESULT_OK;
}

KisImageBuilder_Result KisJPEGConverter::buildImage(const KURL& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, false, qApp -> mainWidget())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp -> mainWidget())) {
        result = decode(tmpFile);
        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageSP KisJPEGConverter::image()
{
    return m_img;
}


KisImageBuilder_Result KisJPEGConverter::buildFile(const KURL& uri, KisPaintLayerSP layer, vKisAnnotationSP_it annotationsStart, vKisAnnotationSP_it annotationsEnd, KisJPEGOptions options)
{
    if (!layer)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageSP img = layer -> image();
    if (!img)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;
    // Open file for writing
    FILE *fp = fopen(uri.path().ascii(), "wb");
    if (!fp)
    {
        return (KisImageBuilder_RESULT_FAILURE);
    }
    uint height = img->height();
    uint width = img->width();
    // Initialize structure
    struct jpeg_compress_struct cinfo;
    jpeg_create_compress(&cinfo);
    // Initialize error output
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    // Initialize output stream
    jpeg_stdio_dest(&cinfo, fp);
    
    cinfo.image_width = width;  // image width and height, in pixels
    cinfo.image_height = height;
    cinfo.input_components = img->colorSpace()->nColorChannels(); // number of color channels per pixel */
    J_COLOR_SPACE color_type = getColorTypeforColorSpace(img->colorSpace());
    if(color_type == JCS_UNKNOWN)
    {
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    cinfo.in_color_space = color_type;   // colorspace of input image

    
    // Set default compression parameters
    jpeg_set_defaults(&cinfo);
    // Customize them
    jpeg_set_quality(&cinfo, options.quality, true);
    
    if(options.progressive)
    {
        jpeg_simple_progression (&cinfo);
    }
    
    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Save annotation
#if 0 // FIXME using iccjpeg.* for profile and marking for other annotations
    vKisAnnotationSP_it it = annotationsStart;
    while(it != annotationsEnd) {
        if (!(*it) || (*it) -> type() == QString()) {
            kdDebug() << "Warning: empty annotation" << endl;
            ++it;
            continue;
        }

        kdDebug() << "Trying to store annotation of type " << (*it) -> type() << " of size " << (*it) -> annotation() . size() << endl;

        if ((*it) -> type().startsWith("krita_attribute:")) { // Attribute
            // FIXME: it should be possible to save krita_attributes in the "CHUNKs"
            kdDebug() << "can't save this annotation : " << (*it) -> type() << endl;
        } else { // Profile
            char* name = new char[(*it)->type().length()+1];
            strcpy(name, (*it)->type().ascii());
            png_set_iCCP(png_ptr, info_ptr, name, PNG_COMPRESSION_TYPE_BASE, (char*)(*it)->annotation().data(), (*it) -> annotation() . size());
        }
        ++it;
    }
#endif

    JSAMPROW row_pointer = new JSAMPLE[width*cinfo.input_components];
    int color_nb_bits = 8 * layer->paintDevice()->pixelSize() / layer->paintDevice()->nChannels();
    
    for (; cinfo.next_scanline < height;) {
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, cinfo.next_scanline, width, false);
        Q_UINT8 *dst = row_pointer;
        switch(color_type)
        {
            case JCS_GRAYSCALE:
                if(color_nb_bits == 16)
                {
                    while (!it.isDone()) {
                        const Q_UINT16 *d = reinterpret_cast<const Q_UINT16 *>(it.rawData());
                        *(dst++) = d[0] / Q_UINT8_MAX;
                        ++it;
                    }
                } else {
                    while (!it.isDone()) {
                        const Q_UINT8 *d = it.rawData();
                        *(dst++) = d[0];
                        ++it;
                    }
                }
                break;
            case JCS_RGB:
                if(color_nb_bits == 16)
                {
                    while (!it.isDone()) {
                        const Q_UINT16 *d = reinterpret_cast<const Q_UINT16 *>(it.rawData());
                        *(dst++) = d[2] / Q_UINT8_MAX;
                        *(dst++) = d[1] / Q_UINT8_MAX;
                        *(dst++) = d[0] / Q_UINT8_MAX;
                        ++it;
                    }
                } else {
                    while (!it.isDone()) {
                        const Q_UINT8 *d = it.rawData();
                        *(dst++) = d[2];
                        *(dst++) = d[1];
                        *(dst++) = d[0];
                        ++it;
                    }
                }
                break;
            case JCS_CMYK:
                if(color_nb_bits == 16)
                {
                    while (!it.isDone()) {
                        const Q_UINT16 *d = reinterpret_cast<const Q_UINT16 *>(it.rawData());
                        *(dst++) = d[0] / Q_UINT8_MAX;
                        *(dst++) = d[1] / Q_UINT8_MAX;
                        *(dst++) = d[2] / Q_UINT8_MAX;
                        *(dst++) = d[3] / Q_UINT8_MAX;
                        ++it;
                    }
                } else {
                    while (!it.isDone()) {
                        const Q_UINT8 *d = it.rawData();
                        *(dst++) = d[0];
                        *(dst++) = d[1];
                        *(dst++) = d[2];
                        *(dst++) = d[3];
                        ++it;
                    }
                }
                break;
            default:
                return KisImageBuilder_RESULT_UNSUPPORTED;
        }
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }
    

    // Writting is over
    jpeg_finish_compress(&cinfo);
    fclose(fp);
    
    delete row_pointer;
    // Free memory
    jpeg_destroy_compress(&cinfo);
    
    return KisImageBuilder_RESULT_OK;
}


void KisJPEGConverter::cancel()
{
    m_stop = true;
}

#include "kis_jpeg_converter.moc"

