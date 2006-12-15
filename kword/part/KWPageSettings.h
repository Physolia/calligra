/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KWPAGESETTINGS_H
#define KWPAGESETTINGS_H

#include "KWord.h"
#include "kword_export.h"

#include <KoPageLayout.h>

/**
 * This class holds all the settings for one document used in automatic placing of frames.
 * For documents that have a main text auto generated we have a lot of little options
 * to do that. This class wraps all these options.
 * Note that the margins are per page and stored in a KWPage instance, not here.
 */
class KWORD_TEST_EXPORT KWPageSettings {
public:
    /// constructor, initializing the data to some default values.
    KWPageSettings();
    /// copy constructor
    KWPageSettings(const KWPageSettings &other);

    /**
     * Return the current columns settings.
     */
    const KoColumns &columns() const { return m_columns; }
    /**
     * Set the new columns settings
     */
    void setColumns(const KoColumns &columns) { m_columns = columns; }

    /// Return the type of header the first page will get.
    KWord::HeaderFooterType firstHeader() const { return m_firstHeader; }
    /**
     * Set the type of header the first page will get.
     * @param p There are 2 ways to 'enable' the header, KWord::HFTypeEvenOdd will
     *   use the OddHeaders frameSet for the text and KWord::HFTypeUniform will use
     *   the FirstHeader frameset for the text.
     * This distinction is useful when reconfiguring a document without moving text
     */
    void setFirstHeaderPolicy(KWord::HeaderFooterType p) { m_firstHeader = p; }

    /// Return the type of footer the first page will get.
    KWord::HeaderFooterType firstFooter() const { return m_firstFooter; }
    /// Set the type of footer the first page will get.
    void setFirstFooterPolicy(KWord::HeaderFooterType p) { m_firstFooter = p; }

    /// Return the type of header all the pages, except the first page will get.
    KWord::HeaderFooterType headers() const { return m_headers; }
    /// set the type of header all the pages, except the first page will get.
    void setHeaderPolicy(KWord::HeaderFooterType p) { m_headers = p; }

    /// Return the type of footers all the pages, except the first page will get.
    KWord::HeaderFooterType footers() const { return m_footers; }
    /// Set the type of footers all the pages, except the first page will get.
    void setFooterPolicy(KWord::HeaderFooterType p) { m_footers = p; }

    /**
     * This is the main toggle for all automatically generated frames.
     * The generation and placing of the main text frame, as well as headers, footers,
     * end notes and footnotes for the main text flow is enabled as soon as this is on.
     * Turn it off and all the other settings on this class will be ignored.
     * @param on the big switch for auto-generated frames.
     */
    void setMainTextFrame(bool on) { m_mainFrame = on; }
    /**
     * Return if the main text frame, but also the headers/footers etc should be autogenerated.
     */
    bool hasMainTextFrame() const { return m_mainFrame; }

    /// return the distance between the main text and the header
    double headerDistance() const { return m_headerDistance; }
    /**
     * Set the distance between the main text and the header
     * @param distance the distance
     */
    void setHeaderDistance(double distance) { m_headerDistance = distance; }
    /// return the distance between the footer and the frame directly above that (footnote or main)
    double footerDistance() const { return m_footerDistance; }
    /**
     * Set the distance between the footer and the frame directly above that (footnote or main)
     * @param distance the distance
     */
    void setFooterDistance(double distance) { m_footerDistance = distance; }
    /// return the distance between the footnote and the main frame.
    double footnoteDistance() const { return m_footNoteDistance; }
    /**
     * Set the distance between the footnote and the main frame.
     * @param distance the distance
     */
    void setFootnoteDistance(double distance) { m_footNoteDistance = distance; }
    /// return the distance between the main text frame and the end notes frame.
    double endNoteDistance() const { return m_endNoteDistance; }
    /**
     * Set the distance between the main text frame and the end notes frame.
     * @param distance the distance
     */
    void setEndNoteDistance(double distance) { m_endNoteDistance = distance; }

    /// return the line length of the foot note separator line, in percent of the pagewidth
    int footNoteSeparatorLineLength() const { return m_footNoteSeparatorLineLength;}
    /// set the line length of the foot note separator line, in percent of the pagewidth
    void setFootNoteSeparatorLineLength( int length){  m_footNoteSeparatorLineLength = length;}

    /// return the thickness of the line (in pt) drawn above the foot notes
    double footNoteSeparatorLineWidth() const { return m_footNoteSeparatorLineWidth;}
    /// set the thickness of the line (in pt) drawn above the foot notes
    void setFootNoteSeparatorLineWidth( double width){  m_footNoteSeparatorLineWidth=width;}

    /// return the pen style used to draw the foot note separator line
    Qt::PenStyle footNoteSeparatorLineType() const { return m_footNoteSeparatorLineType;}
    /// set the pen style used to draw the foot note separator line
    void setFootNoteSeparatorLineType( Qt::PenStyle type) {m_footNoteSeparatorLineType = type;}

    /// return the position on the page for the foot note separator line
    KWord::FootNoteSeparatorLinePos footNoteSeparatorLinePosition() const {
        return m_footNoteSeparatorLinePos;
    }
    /// set the position on the page for the foot note separator line
    void setFootNoteSeparatorLinePosition(KWord::FootNoteSeparatorLinePos position) {
        m_footNoteSeparatorLinePos = position;
    }

    /// initialize to default settings
    void clear();

private:
    KoColumns m_columns;

    bool m_mainFrame;
    double m_headerDistance, m_footerDistance, m_footNoteDistance, m_endNoteDistance;
    KWord::HeaderFooterType m_firstHeader, m_firstFooter, m_headers, m_footers;

    double m_footNoteSeparatorLineWidth; ///< width of line; so more like 'thickness'
    int m_footNoteSeparatorLineLength; ///< It's a percentage of page.
    Qt::PenStyle m_footNoteSeparatorLineType; ///< foot note separate type
    KWord::FootNoteSeparatorLinePos m_footNoteSeparatorLinePos; ///< alignment in page
};

#endif
