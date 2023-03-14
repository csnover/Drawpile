// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/widgets/brushpreview.h"

#include <QPaintEvent>
#include <QPainter>
#include <QEvent>

namespace widgets {

BrushPreview::BrushPreview(QWidget *parent, Qt::WindowFlags f)
	: QFrame(parent,f)
{
	setAttribute(Qt::WA_NoSystemBackground);
	setMinimumSize(32,32);

	updateBackground();
}

BrushPreview::~BrushPreview() {
#ifndef DESIGNER_PLUGIN
	rustpile::brushpreview_free(m_previewcanvas);
#endif
}

void BrushPreview::setBrush(const brushes::ActiveBrush &brush)
{
	m_brush = brush;
	m_needUpdate = true;
	update();
}

void BrushPreview::setPreviewShape(rustpile::BrushPreviewShape shape)
{
	if(m_shape != shape) {
		m_shape = shape;
		m_needUpdate = true;
		update();
	}
}

void BrushPreview::setFloodFillTolerance(int tolerance)
{
	if(m_fillTolerance != tolerance) {
		m_fillTolerance = tolerance;
		m_needUpdate = true;
		update();
	}
}

void BrushPreview::setFloodFillExpansion(int expansion)
{
	if(m_fillExpansion != expansion) {
		m_fillExpansion = expansion;
		m_needUpdate = true;
		update();
	}
}

void BrushPreview::setUnderFill(bool underfill)
{
	if(m_underFill != underfill) {
		m_underFill = underfill;
		m_needUpdate = true;
		update();
	}
}

void BrushPreview::resizeEvent(QResizeEvent *)
{
	m_needUpdate = true;
}

void BrushPreview::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::PaletteChange) {
		updateBackground();
	}

	m_needUpdate = true;
	update();
}

void BrushPreview::paintEvent(QPaintEvent *event)
{
	if(m_needUpdate)
		updatePreview();

	QPainter painter(this);
#ifdef DESIGNER_PLUGIN
	Q_UNUSED(event)
	painter.drawTiledPixmap(0, 0, width(), height(), m_background);
#else
	painter.drawPixmap(event->rect(), m_preview, event->rect());
#endif
}

void BrushPreview::updateBackground()
{
	constexpr int w = 16;
	const auto dark = palette().color(QPalette::Window).lightness() < 128;
	m_background = QPixmap(w*2, w*2);
	m_background.fill(dark ? QColor(153, 153, 153) : QColor(204, 204, 204));
	const auto alt = dark ? QColor(102, 102, 102) : Qt::white;
	QPainter p(&m_background);
	p.fillRect(0, 0, w, w, alt);
	p.fillRect(w, w, w, w, alt);
}

void BrushPreview::updatePreview()
{
#ifndef DESIGNER_PLUGIN
	const QSize size = contentsRect().size();

	if(size != m_preview.size()) {
		rustpile::brushpreview_free(m_previewcanvas);
		m_previewcanvas = rustpile::brushpreview_new(size.width(), size.height());
		m_preview = QPixmap(size);
	}

	m_brush.renderPreview(m_previewcanvas, m_shape);
	if(m_shape == rustpile::BrushPreviewShape::FloodFill || m_shape == rustpile::BrushPreviewShape::FloodErase) {
		rustpile::Color color = m_brush.color();
		if(m_shape == rustpile::BrushPreviewShape::FloodErase) {
			color.a = 0;
		}
		rustpile::brushpreview_floodfill(m_previewcanvas, &color, m_fillTolerance / 255.0, m_fillExpansion, m_underFill);
	}

	QPainter p(&m_preview);
	p.drawTiledPixmap(0, 0, m_preview.width(), m_preview.height(), m_background);
	rustpile::brushpreview_paint(m_previewcanvas, &p, [](void *p, int x, int y, const uchar *pixels) {
		const QImage img(pixels, 64, 64, 64*4, QImage::Format_ARGB32_Premultiplied);
		static_cast<QPainter*>(p)->drawImage(x, y, img);
	});

#endif
	m_needUpdate=false;
}

void BrushPreview::mouseDoubleClickEvent(QMouseEvent*)
{
	emit requestColorChange();
}

}
