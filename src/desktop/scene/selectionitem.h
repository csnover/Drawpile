// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SELECTIONITEM_H
#define SELECTIONITEM_H

#include "libclient/canvas/selection.h"

#include <QGraphicsObject>

namespace drawingboard {

class SelectionItem : public QGraphicsObject
{
public:
	enum { Type= UserType + 11 };

	SelectionItem(canvas::Selection *selection, QGraphicsItem *parent=0);

	QRectF boundingRect() const;
	int type() const { return Type; }

	void marchingAnts(double dt);

private slots:
	void onShapeChanged();
	void onAdjustmentModeChanged();

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *options, QWidget *);

private:
	QPolygonF m_shape;
	canvas::Selection *m_selection;
	qreal m_marchingants;
};

}

#endif // SELECTIONITEM_H
