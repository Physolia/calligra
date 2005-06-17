/* This file is part of the KDE project
 Copyright (C) 2003 Alexander Dymo <adymo@mksat.net>

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
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
*/
#ifndef COMMDEFS_H
#define COMMDEFS_H

#include <qapplication.h>
#include <qcanvas.h>
#include <qstringlist.h>

namespace Kudesigner
{

enum RttiValues { Rtti_Box = 1700, Rtti_Section, Rtti_Band,
                  Rtti_KugarTemplate = 1800, Rtti_ReportHeader, Rtti_PageHeader, Rtti_DetailHeader,
                  Rtti_Detail, Rtti_DetailFooter, Rtti_PageFooter, Rtti_ReportFooter,
                  Rtti_TextBox = 2000, Rtti_ReportItem, Rtti_Label, Rtti_Field,
                  Rtti_Special, Rtti_Calculated, Rtti_Line};

QString insertItemActionName( int rtti );

class Config
{
public:
    static int holderSize()
    {
        return m_holderSize;
    }
    static void setHolderSize( int holderSize )
    {
        m_holderSize = holderSize;
    }
    static int gridSize();
    static void setGridSize( int gridSize );

private:
    static int m_holderSize;
    static int m_gridSize;
};

}

//Holder size for report items. TODO: configurable
const int HolderSize = 6;

const int DefaultItemWidth = 50;
const int DefaultItemHeight = 20;

#endif
