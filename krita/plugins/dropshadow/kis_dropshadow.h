/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_DROPSHADOW_H_
#define _KIS_DROPSHADOW_H_

#include <kis_progress_subject.h>

class QColor;
class KisView;
class KisProgressDisplayInterface;

class KisDropshadow : public KisProgressSubject {

    Q_OBJECT

public:

    KisDropshadow(KisView * view);
    virtual ~KisDropshadow() {};

    void dropshadow(KisProgressDisplayInterface * progress, Q_INT32 xoffset, Q_INT32 yoffset, Q_INT32 blurradius, QColor color, Q_UINT8 opacity);

public: // Implement KisProgressSubject
        virtual void cancel() { m_cancelRequested = true; }

private:
    KisView * m_view;
    bool m_cancelRequested;

};

#endif
