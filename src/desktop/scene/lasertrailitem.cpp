// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/scene/lasertrailitem.h"

#include <QApplication>
#include <QPainter>
#include <QDateTime>

namespace drawingboard {

LaserTrailItem::LaserTrailItem(uint8_t owner, int persistenceMs, const QColor &color, QGraphicsItem *parent)
	: QGraphicsItem(parent), m_blink(false), m_fadeout(false), m_owner(owner), m_lastModified(0), m_persistence(persistenceMs)
{
	m_pen.setWidth(qApp->devicePixelRatio() * 3);
	m_pen.setCapStyle(Qt::RoundCap);
	m_pen.setCosmetic(true);
	m_pen.setColor(color);
}

QRectF LaserTrailItem::boundingRect() const
{
	return m_bounds;
}

void LaserTrailItem::addPoint(const QPointF &point)
{
	prepareGeometryChange();
	m_points.append(point);
	if(m_points.length()==1) {
		m_bounds = QRectF{point, QSizeF{1, 1}};
	} else {
		m_bounds = m_bounds.united(QRectF{point, QSizeF{1, 1}});
	}
	m_lastModified = QDateTime::currentMSecsSinceEpoch();
}

void LaserTrailItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	if(m_blink) {
		QPen pen = m_pen;
		pen.setWidth(pen.width() + 1);
		painter->setPen(pen);
	} else {
		painter->setPen(m_pen);
	}

	painter->drawPolyline(m_points.constData(), m_points.size());

	painter->restore();
}

bool LaserTrailItem::animationStep(qreal dt)
{
	m_blink = !m_blink;

	bool retain = true;
	if(m_fadeout) {
		setOpacity(qMax(0.0, opacity() - dt));
		retain = opacity() > 0.0;
	} else if(m_lastModified < QDateTime::currentMSecsSinceEpoch() - m_persistence) {
		m_fadeout = true;
	}

	update(boundingRect());

	return retain;
}

}
