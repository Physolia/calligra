/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "kis_pan_action.h"

#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QGesture>

#include <KLocalizedString>

#include <KoCanvasController.h>

#include <kis_canvas2.h>

#include "kis_input_manager.h"

class KisPanAction::Private
{
public:
    Private() : panDistance(10) { }

        QPointF averagePoint( QTouchEvent* event );

    const int panDistance;

    QPointF lastPosition;
};

KisPanAction::KisPanAction(KisInputManager *manager)
    : KisAbstractInputAction(manager), d(new Private)
{
    setName(i18n("Pan Canvas"));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Toggle Pan Mode"), PanToggleShortcut);
    shortcuts.insert(i18n("Pan Left"), PanLeftShortcut);
    shortcuts.insert(i18n("Pan Right"), PanRightShortcut);
    shortcuts.insert(i18n("Pan Up"), PanUpShortcut);
    shortcuts.insert(i18n("Pan Down"), PanDownShortcut);
    setShortcutIndexes(shortcuts);
}

KisPanAction::~KisPanAction()
{
    delete d;
}

void KisPanAction::activate()
{
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
}

void KisPanAction::deactivate()
{
    QApplication::restoreOverrideCursor();
}

void KisPanAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    switch (shortcut) {
        case PanToggleShortcut: {
            QTouchEvent *tevent = dynamic_cast<QTouchEvent*>(event);
            d->lastPosition = d->averagePoint(tevent);
            break;
        }
        case PanLeftShortcut:
            inputManager()->canvas()->canvasController()->pan(QPoint(d->panDistance, 0));
            break;
        case PanRightShortcut:
            inputManager()->canvas()->canvasController()->pan(QPoint(-d->panDistance, 0));
            break;
        case PanUpShortcut:
            inputManager()->canvas()->canvasController()->pan(QPoint(0, d->panDistance));
            break;
        case PanDownShortcut:
            inputManager()->canvas()->canvasController()->pan(QPoint(0, -d->panDistance));
            break;
    }
}

// <<<<<<< HEAD
// void KisPanAction::end()
// {
//     d->active = false;
//     QApplication::restoreOverrideCursor();
// }
//
void KisPanAction::inputEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Gesture: {
            QGestureEvent *gevent = static_cast<QGestureEvent*>(event);
            if (gevent->activeGestures().at(0)->gestureType() == Qt::PanGesture) {
                QPanGesture *pan = static_cast<QPanGesture*>(gevent->activeGestures().at(0));
                inputManager()->canvas()->canvasController()->pan(-pan->delta().toPoint() * 0.2);
            }
        }
        case QEvent::TouchUpdate: {
            QTouchEvent *tevent = static_cast<QTouchEvent*>(event);
            QPointF newPos = d->averagePoint(tevent);
            QPointF delta = newPos - d->lastPosition;
            inputManager()->canvas()->canvasController()->pan(-delta.toPoint());
            d->lastPosition = newPos;
        }
        default:
            break;
    }

    KisAbstractInputAction::inputEvent(event);
}

void KisPanAction::mouseMoved(const QPointF &lastPos, const QPointF &pos)
{
    QPointF relMovement = -(pos - lastPos);
    inputManager()->canvas()->canvasController()->pan(relMovement.toPoint());
}

QPointF KisPanAction::Private::averagePoint( QTouchEvent* event )
{
    QPointF result;
    int count = 0;

    foreach( QTouchEvent::TouchPoint point, event->touchPoints() ) {
        if( point.state() != Qt::TouchPointReleased ) {
            result += point.screenPos();
            count++;
        }
    }

    if( count > 0 ) {
        return result / count;
    } else {
        return QPointF();
    }
}
