// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>

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

#include <kdebug.h>

#include "KWEFStructures.h"
#include "KWEFBaseWorker.h"
#include "KWEFKWordLeader.h"

void KWEFBaseWorker::registerKWordLeader(KWEFKWordLeader* leader)
{
    m_kwordLeader=leader;
}

//
// At first, define all methods that do something real!
//

bool KWEFBaseWorker::doAbortFile(void)
{
    // Mostly, aborting is the same than closing the file!
    return doCloseFile();
}

bool KWEFBaseWorker::doFullDocument (const QValueList<ParaData>& paraList)
{
    if (!doOpenTextFrameSet())
        return false;
    if (!doFullAllParagraphs(paraList))
        return false;
    if (!doCloseTextFrameSet())
        return false;

    return true;
}

bool KWEFBaseWorker::doFullAllParagraphs (const QValueList<ParaData>& paraList)
{
    QValueList<ParaData>::ConstIterator it;
    for (it=paraList.begin();it!=paraList.end();it++)
    {
        if (!doFullParagraph((*it).text,(*it).layout,(*it).formattingList))
            return false;
    }
    return true;
}

bool KWEFBaseWorker::loadKoStoreFile(const QString& fileName, QByteArray& array)
{   // DEPRECATED: use loadSubFile
    return loadSubFile(fileName,array);
}

bool KWEFBaseWorker::loadSubFile(const QString& fileName, QByteArray& array)
// return value:
//   true if the file is not empty
//   false if the file is empty or if an error occured
{
    bool flag=false;
    if (m_kwordLeader)
    {
        flag=m_kwordLeader->loadSubFile(fileName,array);
    }
    else
    {
        kdWarning(30508) << "Leader is unknown! (KWEFBaseWorker::loadSubFile)" << endl;
    }
    return flag;
}

QIODevice* KWEFBaseWorker::getSubFileDevice(const QString& fileName)
{
    if (!m_kwordLeader)
    {
        kdWarning(30508) << "Leader is unknown! (KWEFBaseWorker::getSubFileDevice)" << endl;
        return NULL;
    }
    return m_kwordLeader->getSubFileDevice(fileName);
}

//
// Secondly, define all methods returning false
//

#define DO_FALSE_DEFINITION(string) \
    bool KWEFBaseWorker::string \
    {\
        kdWarning(30508) << "KWEFBaseWorker::" << #string << " was called (Worker not correctly defined?)" << endl; \
        return false;\
    }

DO_FALSE_DEFINITION (doOpenFile (const QString& , const QString& ))
DO_FALSE_DEFINITION (doCloseFile (void))
DO_FALSE_DEFINITION (doOpenDocument (void))
DO_FALSE_DEFINITION (doCloseDocument (void))

// The following is not generated by the leader
DO_FALSE_DEFINITION (doFullParagraph(const QString&, const LayoutData&, const ValueListFormatData&))

//
// Thirdly, define all methods returning true
//

#define DO_TRUE_DEFINITION(string) \
    bool KWEFBaseWorker::string \
    {\
        return true;\
    }

DO_TRUE_DEFINITION (doFullDocumentInfo (const KWEFDocumentInfo&))
DO_TRUE_DEFINITION (doFullPaperFormat (const int, const double, const double, const int))
DO_TRUE_DEFINITION (doFullPaperBorders (const double, const double, const double, const double))
DO_TRUE_DEFINITION (doOpenHead (void))
DO_TRUE_DEFINITION (doCloseHead (void))
DO_TRUE_DEFINITION (doOpenBody (void))
DO_TRUE_DEFINITION (doCloseBody (void))
DO_TRUE_DEFINITION (doOpenStyles (void))
DO_TRUE_DEFINITION (doCloseStyles (void))
DO_TRUE_DEFINITION (doFullDefineStyle (LayoutData&))
DO_TRUE_DEFINITION (doOpenSpellCheckIgnoreList (void))
DO_TRUE_DEFINITION (doCloseSpellCheckIgnoreList (void))
DO_TRUE_DEFINITION (doFullSpellCheckIgnoreWord (const QString&))

//  The following are not generated by the leader
DO_TRUE_DEFINITION (doOpenTextFrameSet (void))
DO_TRUE_DEFINITION (doCloseTextFrameSet (void))
