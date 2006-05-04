/* This file is part of the KDE project
   Copyright (C) 2004 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004, 2006 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIQUERYVIEW_H
#define KEXIQUERYVIEW_H

#include <kexidatatable.h>

namespace KexiDB
{
	class QuerySchema;
}
class KexiMainWindow;

class KexiQueryView : public KexiDataTable
{
	Q_OBJECT

	public:
		KexiQueryView(KexiMainWindow *win, QWidget *parent, const char *name=0);
		~KexiQueryView();

	protected:
		virtual tristate afterSwitchFrom(int mode);

		virtual KexiDB::SchemaData* storeNewData(const KexiDB::SchemaData& sdata, bool &cancel);

		virtual tristate storeData(bool dontAsk = false);

		bool executeQuery(KexiDB::QuerySchema *query);

		class Private;
		Private *d;
};

#endif

