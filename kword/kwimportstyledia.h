/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __KWImportStyleDia__
#define __KWImportStyleDia__

#include <kdialogbase.h>
#include <qstringlist.h>
#include <kwframestyle.h>
#include <kwtablestyle.h>
class QLineEdit;
class QListBox;
class KWDocument;
class QPushButton;

class KWImportFrameTableStyleDia : public KDialogBase
{
    Q_OBJECT
public:
    enum StyleType { frameStyle, TableStyle};
    KWImportFrameTableStyleDia( KWDocument *_doc, const QStringList & _list, StyleType _type , QWidget *parent, const char *name );
    ~KWImportFrameTableStyleDia();
    QPtrList<KWFrameStyle> listOfFrameStyleImported()const { return m_frameStyleList;}
    QPtrList<KWTableStyle> listOfTableStyleImported()const { return m_tableStyleList;}

protected slots:
    virtual void slotOk();

protected:
    QString generateStyleName( const QString & templateName );
    void loadFile();
    void initList();
    QListBox *m_listStyleName;
    KWDocument *m_doc;
    QPtrList<KWFrameStyle> m_frameStyleList;
    QPtrList<KWTableStyle> m_tableStyleList;
    QStringList m_list;
    StyleType m_typeStyle;
};

#endif
