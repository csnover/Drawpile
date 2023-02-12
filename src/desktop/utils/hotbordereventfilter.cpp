// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/utils/hotbordereventfilter.h"

#include <QMouseEvent>
#include <QWidget>

HotBorderEventFilter::HotBorderEventFilter(QObject *parent)
	: QObject(parent), m_hotBorder(false)
{
}

bool HotBorderEventFilter::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::HoverMove) {
		const QMouseEvent *e = static_cast<const QMouseEvent*>(event);

		// For some reason e->globalPos() does not always work. Window manager specific problem?
		const QPoint p = static_cast<QWidget*>(object)->mapToGlobal(e->pos());

		if(m_hotBorder) {
			if(p.y() > 30) {
				m_hotBorder = false;
				emit hotBorder(false);
			}
		} else {
			if(p.y() < 10) {
				m_hotBorder = true;
				emit hotBorder(true);
			}
		}
	}

	return false;
}
