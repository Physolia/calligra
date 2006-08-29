/* This file is part of the KDE project
   Copyright (C) 2001 Laurent Montel <montel@kde.org>
             (C) 2000 Torben Weis <weis@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_LOCALE_H
#define KSPREAD_LOCALE_H

#ifdef __APPLE__
/* FIXME: the proper fix is to rename Locale.h, it conflicts
with <locale.h> on case-insensitive filesystems but for
now I just want to make it work */

#include_next <locale.h>
#endif

#include <klocale.h>
#include <KoXmlReader.h>

class QDomElement;
class QDomDocument;

namespace KSpread
{

class Locale : public KLocale
{
public:
    Locale();

    void load( const KoXmlElement& element );
    QDomElement save( QDomDocument& doc ) const;
    void defaultSystemConfig();
};

} //namespace KSpread

#endif
