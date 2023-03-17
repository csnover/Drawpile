// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DP_CANVASITEM_H
#define DP_CANVASITEM_H

#include <QGraphicsObject>
#include <QPointer>

namespace canvas {
	class PaintEngine;
}

namespace drawingboard {

/**
 * @brief A graphics item that draws a LayerStack
 */
class CanvasItem final : public QGraphicsObject
{
Q_OBJECT
public:
	CanvasItem(QGraphicsItem *parent=nullptr);
	void setPaintEngine(canvas::PaintEngine *pe);

	QRectF boundingRect() const override;

private slots:
	void refreshImage(const QRect &area);
	void canvasResize();

protected:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
	QPointer<canvas::PaintEngine> m_image;
};

}

#endif
