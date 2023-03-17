// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/canvas/canvasmodel.h"
#include "libclient/canvas/paintengine.h"

#include "libclient/net/client.h"
#include "libclient/net/envelopebuilder.h"

#include "libclient/tools/toolcontroller.h"
#include "libclient/tools/shapetools.h"
#include "libclient/tools/utils.h"

#include "rustpile/rustpile.h"

#include <QPixmap>

namespace tools {

void ShapeTool::begin(const canvas::Point& point, bool right, float zoom)
{
	Q_UNUSED(zoom);
	Q_UNUSED(right);
	Q_ASSERT(!m_drawing);

	m_start = point;
	m_p1 = point;
	m_p2 = point;
	m_drawing = true;

	updatePreview();
}

void ShapeTool::motion(const canvas::Point& point, bool constrain, bool center)
{
	if(!m_drawing)
		return;

	if(constrain)
		m_p2 = constraints::square(m_start, point);
	else
		m_p2 = point;

	if(center)
		m_p1 = m_start - (m_p2 - m_start);
	else
		m_p1 = m_start;

	updatePreview();
}

void ShapeTool::cancelMultipart()
{
	rustpile::paintengine_remove_preview(m_owner.model()->paintEngine()->engine(), m_owner.activeLayer());
	m_drawing = false;
}

void ShapeTool::end()
{
	if(!m_drawing)
		return;

	m_drawing = false;

	const uint8_t contextId = m_owner.client()->myId();
	auto engine = m_owner.model()->paintEngine()->engine();

	auto brushengine = rustpile::brushengine_new();

	m_owner.setBrushEngineBrush(brushengine, false);

	const auto pv = pointVector();
	for(const auto &p : pv) {
		rustpile::brushengine_stroke_to(brushengine, p.x(), p.y(), p.pressure(), 10, engine, m_owner.activeLayer());
	}
	rustpile::brushengine_end_stroke(brushengine);

	net::EnvelopeBuilder writer;
	rustpile::write_undopoint(writer, contextId);
	rustpile::brushengine_write_dabs(brushengine, contextId, writer);
	rustpile::write_penup(writer, contextId);

	rustpile::brushengine_free(brushengine);

	rustpile::paintengine_remove_preview(engine, m_owner.activeLayer());
	m_owner.client()->sendEnvelope(writer.toEnvelope());
}

void ShapeTool::updatePreview()
{
	auto brushengine = rustpile::brushengine_new();

	m_owner.setBrushEngineBrush(brushengine, false);

	auto engine = m_owner.model()->paintEngine()->engine();

	const canvas::PointVector pv = pointVector();
	Q_ASSERT(pv.size()>1);

	for(const auto &p : pv) {
		rustpile::brushengine_stroke_to(brushengine, p.x(), p.y(), p.pressure(), 10, engine, m_owner.activeLayer());
	}
	rustpile::brushengine_end_stroke(brushengine);

	rustpile::paintengine_preview_brush(engine, m_owner.activeLayer(), brushengine);
	rustpile::brushengine_free(brushengine);
}

Line::Line(ToolController &owner)
	: ShapeTool(owner, LINE, QCursor(QPixmap(":cursors/line.png"), 1, 1))
{
}

void Line::motion(const canvas::Point& point, bool constrain, bool center)
{
	if(constrain)
		m_p2 = constraints::angle(m_start, point);
	else
		m_p2 = point;

	if(center)
		m_p1 = m_start - (m_p2 - m_start);
	else
		m_p1 = m_start;

	updatePreview();
}

canvas::PointVector Line::pointVector() const
{
	canvas::PointVector pv;
	pv.reserve(2);
	pv << canvas::Point(m_p1, 1) << canvas::Point(m_p2, 1);
	return pv;
}

Rectangle::Rectangle(ToolController &owner)
	: ShapeTool(owner, RECTANGLE, QCursor(QPixmap(":cursors/rectangle.png"), 1, 1))
{
}

canvas::PointVector Rectangle::pointVector() const
{
	canvas::PointVector pv;
	pv.reserve(5);
	pv << canvas::Point(m_p1, 1);
	pv << canvas::Point(m_p1.x(), m_p2.y(), 1);
	pv << canvas::Point(m_p2, 1);
	pv << canvas::Point(m_p2.x(), m_p1.y(), 1);
	pv << canvas::Point(m_p1.x(), m_p1.y(), 1);
	return pv;
}

Ellipse::Ellipse(ToolController &owner)
	: ShapeTool(owner, ELLIPSE, QCursor(QPixmap(":cursors/ellipse.png"), 1, 1))
{
}

canvas::PointVector Ellipse::pointVector() const
{
	const auto r = rect();
	const qreal a = r.width() / 2.0;
	const qreal b = r.height() / 2.0;
	const qreal cx = r.x() + a;
	const qreal cy = r.y() + b;

	canvas::PointVector pv;

	// TODO smart step size selection
	for(qreal t=0;t<2*M_PI;t+=M_PI/20) {
		pv << canvas::Point(cx + a*cos(t), cy + b*sin(t), 1.0);
	}
	pv << canvas::Point(cx+a, cy, 1);
	return pv;
}

}
