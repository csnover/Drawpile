// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef LASERTRAILITEM_H
#define LASERTRAILITEM_H

#include <QGraphicsItem>
#include <QPen>

namespace drawingboard {

class LaserTrailItem final : public QGraphicsItem
{
public:
	enum { Type = UserType + 13 };

	LaserTrailItem(uint8_t owner, int persistenceMs, const QColor &color, QGraphicsItem *parent=nullptr);

	QRectF boundingRect() const override;
	int type() const override { return Type; }

	bool animationStep(qreal dt);

	void addPoint(const QPointF &point);

	uint8_t owner() const { return m_owner; }

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
	bool m_blink;
	bool m_fadeout;
	uint8_t m_owner;
	QPen m_pen;
	QVector<QPointF> m_points;
	QRectF m_bounds;
	qint64 m_lastModified;
	qint64 m_persistence;
};

}

#endif // LASERTRAILITEM_H
