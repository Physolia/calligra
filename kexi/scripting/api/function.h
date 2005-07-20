/***************************************************************************
 * function.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ***************************************************************************/

#ifndef KROSS_API_FUNCTION_H
#define KROSS_API_FUNCTION_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qobject.h>
#include <klocale.h>
#include <kdebug.h>

#include "object.h"
//#include "classbase.h"
#include "list.h"
//#include "exception.h"
#include "argument.h"

namespace Kross { namespace Api {

    /**
     * Base class for a callable function. A function always
     * implements the \a Object::call() method to handle
     * the call. For example the \a ClassFunction class
     * is used to implement methods in classes.
     */
    class Function : public Object
    {
        public:
            Function(const QString& name, ArgumentList arglist, const QString& documentation);
            virtual ~Function();

            virtual const QString getClassName() const;
            virtual const QString getDescription() const;

            virtual Object::Ptr call(const QString& name, KSharedPtr<List> arguments) = 0;

        protected:
            /// List of arguments this function supports.
            ArgumentList m_arglist;
            /// Some documentation to describe the function.
            QString m_documentation;
    };

}}

#endif

