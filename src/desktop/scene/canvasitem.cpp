// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/scene/canvasitem.h"

#include "libclient/canvas/paintengine.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace drawingboard {

/**
 * @param parent use another QGraphicsItem as a parent
 * @param scene the picture to which this layer belongs to
 */
CanvasItem::CanvasItem(QGraphicsItem *parent)
	: QGraphicsObject(parent), m_image(nullptr)
{
	setFlag(ItemUsesExtendedStyleOption);
}

void CanvasItem::setPaintEngine(canvas::PaintEngine *pe)
{
	m_image = pe;
	if(m_image) {
		connect(m_image, &canvas::PaintEngine::areaChanged, this, &CanvasItem::refreshImage);
		connect(m_image, &canvas::PaintEngine::resized, this, &CanvasItem::canvasResize);
	}
}

void CanvasItem::refreshImage(const QRect &area)
{
	update(area.adjusted(-2, -2, 2, 2));
}

void CanvasItem::canvasResize()
{
	prepareGeometryChange();
}

QRectF CanvasItem::boundingRect() const
{
	if(m_image)
		return QRectF(QPointF(), QSizeF(m_image->size()));
	return QRectF();
}

void CanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	 QWidget *)
{
	if(m_image) {
		const QRect exposed = option->exposedRect.adjusted(-1, -1, 1, 1).toAlignedRect();
		painter->drawPixmap(exposed, m_image->getPixmap(exposed), exposed);
	}
}

}
