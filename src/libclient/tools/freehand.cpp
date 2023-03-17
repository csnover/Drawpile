// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/canvas/canvasmodel.h"
#include "libclient/canvas/paintengine.h"
#include "libclient/net/client.h"
#include "libclient/net/envelopebuilder.h"

#include "libclient/tools/toolcontroller.h"
#include "libclient/tools/freehand.h"

#include "libshared/net/undo.h"
#include "rustpile/rustpile.h"

namespace tools {

Freehand::Freehand(ToolController &owner, bool isEraser)
	: Tool(owner, isEraser ? ERASER : FREEHAND, Qt::CrossCursor), m_drawing(false)
{
	m_brushengine = rustpile::brushengine_new();
}

Freehand::~Freehand()
{
	rustpile::brushengine_free(m_brushengine);
}

void Freehand::begin(const canvas::Point& point, bool right, float zoom)
{
	Q_UNUSED(zoom);
	Q_UNUSED(right);
	Q_ASSERT(!m_drawing);

	m_drawing = true;
	m_firstPoint = true;
	m_lastTimestamp = QDateTime::currentMSecsSinceEpoch();

	m_owner.setBrushEngineBrush(m_brushengine);

	// The pressure value of the first point is unreliable
	// because it is (or was?) possible to get a synthetic MousePress event
	// before the StylusPress event.
	m_start = point;
}

void Freehand::motion(const canvas::Point& point, bool constrain, bool center)
{
	Q_UNUSED(constrain);
	Q_UNUSED(center);
	if(!m_drawing)
		return;

	net::EnvelopeBuilder writer;

	if(m_firstPoint) {
		m_firstPoint = false;

		rustpile::write_undopoint(writer, m_owner.client()->myId());

		rustpile::brushengine_stroke_to(
			m_brushengine,
			m_start.x(),
			m_start.y(),
			qMin(m_start.pressure(), point.pressure()),
			0,
			m_owner.model()->paintEngine()->engine(),
			m_owner.activeLayer()
		);
	}

	qint64 now = QDateTime::currentMSecsSinceEpoch();
	qint64 deltaMsec = now - m_lastTimestamp;
	m_lastTimestamp = now;

	rustpile::brushengine_stroke_to(
		m_brushengine,
		point.x(),
		point.y(),
		point.pressure(),
		deltaMsec,
		m_owner.model()->paintEngine()->engine(),
		m_owner.activeLayer()
	);

	rustpile::brushengine_write_dabs(m_brushengine, m_owner.client()->myId(), writer);

	m_owner.client()->sendEnvelope(writer.toEnvelope());
}

void Freehand::end()
{
	if(m_drawing) {
		m_drawing = false;

		net::EnvelopeBuilder writer;

		if(m_firstPoint) {
			m_firstPoint = false;

			rustpile::write_undopoint(writer, m_owner.client()->myId());

			rustpile::brushengine_stroke_to(
				m_brushengine,
				m_start.x(),
				m_start.y(),
				m_start.pressure(),
				QDateTime::currentMSecsSinceEpoch(),
				nullptr,
				0
			);
		}

		rustpile::brushengine_end_stroke(m_brushengine);
		rustpile::brushengine_write_dabs(m_brushengine, m_owner.client()->myId(), writer);
		rustpile::write_penup(writer, m_owner.client()->myId());

		m_owner.client()->sendEnvelope(writer.toEnvelope());
	}
}

void Freehand::offsetActiveTool(int x, int y)
{
	if(m_drawing)
		rustpile::brushengine_add_offset(m_brushengine, x, y);
}

}
