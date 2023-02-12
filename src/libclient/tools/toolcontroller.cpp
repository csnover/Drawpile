// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/tools/toolcontroller.h"

#include "libclient/tools/annotation.h"
#include "libclient/tools/freehand.h"
#include "libclient/tools/colorpicker.h"
#include "libclient/tools/laser.h"
#include "libclient/tools/selection.h"
#include "libclient/tools/shapetools.h"
#include "libclient/tools/beziertool.h"
#include "libclient/tools/floodfill.h"
#include "libclient/tools/zoom.h"
#include "libclient/tools/inspector.h"

#include "libclient/canvas/point.h"
#include "libclient/canvas/canvasmodel.h"
#include "libclient/canvas/paintengine.h"

namespace tools {

ToolController::ToolController(net::Client *client, QObject *parent)
	: QObject(parent),
	m_toolbox{},
	m_client(client), m_model(nullptr),
	m_activeTool(nullptr),
	m_prevShift(false), m_prevAlt(false),
	m_smoothing(0)
{
	Q_ASSERT(client);

	registerTool(new Freehand(*this, false));
	registerTool(new Freehand(*this, true)); // eraser is a specialized freehand tool
	registerTool(new ColorPicker(*this));
	registerTool(new Line(*this));
	registerTool(new Rectangle(*this));
	registerTool(new Ellipse(*this));
	registerTool(new BezierTool(*this));
	registerTool(new FloodFill(*this));
	registerTool(new Annotation(*this));
	registerTool(new LaserPointer(*this));
	registerTool(new RectangleSelection(*this));
	registerTool(new PolygonSelection(*this));
	registerTool(new ZoomTool(*this));
	registerTool(new Inspector(*this));

	m_activeTool = m_toolbox[Tool::FREEHAND];
	m_activeLayer = 0;
	m_activeAnnotation = 0;
}

void ToolController::registerTool(Tool *tool)
{
	Q_ASSERT(tool->type() >= 0 && tool->type() < Tool::_LASTTOOL);
	Q_ASSERT(m_toolbox[int(tool->type())] == nullptr);
	m_toolbox[tool->type()] = tool;
}

ToolController::~ToolController()
{
	for(Tool *t : m_toolbox)
		delete t;
}

Tool *ToolController::getTool(Tool::Type type)
{
	Q_ASSERT(type >= 0 && type < Tool::_LASTTOOL);
	Tool *t = m_toolbox[type];
	Q_ASSERT(t);
	return t;
}

void ToolController::setActiveTool(Tool::Type tool)
{
	if(activeTool() != tool) {
		m_activeTool->cancelMultipart();
		endDrawing();

		m_activeTool = getTool(tool);
		emit activeToolChanged(tool);
		emit toolCursorChanged(activeToolCursor());
	}
}

void ToolController::setActiveAnnotation(uint16_t id)
{
	if(m_activeAnnotation != id) {
		m_activeAnnotation = id;
		emit activeAnnotationChanged(id);
	}
}

Tool::Type ToolController::activeTool() const
{
	Q_ASSERT(m_activeTool);
	return m_activeTool->type();
}

QCursor ToolController::activeToolCursor() const
{
	Q_ASSERT(m_activeTool);
	return m_activeTool->cursor();
}

void ToolController::setActiveLayer(uint16_t id)
{
	if(m_activeLayer != id) {
		m_activeLayer = id;
		if(m_model) {
			m_model->paintEngine()->setViewLayer(id);
		}

		emit activeLayerChanged(id);
	}
}

void ToolController::setActiveBrush(const brushes::ActiveBrush &b)
{
	m_activebrush = b;
	emit activeBrushChanged(b);
}

void ToolController::setModel(canvas::CanvasModel *model)
{
	if(m_model != model) {
		m_model = model;
		connect(m_model->aclState(), &canvas::AclState::featureAccessChanged, this, &ToolController::onFeatureAccessChange);
	}
	emit modelChanged(model);
}

void ToolController::onFeatureAccessChange(canvas::Feature feature, bool canUse)
{
	if(feature == canvas::Feature::RegionMove) {
		static_cast<SelectionTool*>(getTool(Tool::SELECTION))->setTransformEnabled(canUse);
		static_cast<SelectionTool*>(getTool(Tool::POLYGONSELECTION))->setTransformEnabled(canUse);
	}
}

void ToolController::setSmoothing(int smoothing)
{
	if(m_smoothing != smoothing) {
		m_smoothing = smoothing;
		if(smoothing>0)
			m_smoother.setSmoothing(smoothing);
		emit smoothingChanged(smoothing);
	}
}

void ToolController::startDrawing(const QPointF &point, qreal pressure, bool right, float zoom)
{
	Q_ASSERT(m_activeTool);

	if(!m_model) {
		qWarning("ToolController::startDrawing: no model set!");
		return;
	}

	m_smoother.reset();
	m_activeTool->begin(canvas::Point(point, pressure), right, zoom);

#if 0 // FIXME
	if(!m_activeTool->isMultipart())
		m_model->stateTracker()->setLocalDrawingInProgress(true);
#endif

	if(!m_activebrush.isEraser())
		emit colorUsed(m_activebrush.qColor());
}

void ToolController::continueDrawing(const QPointF &point, qreal pressure, bool shift, bool alt)
{
	Q_ASSERT(m_activeTool);

	if(!m_model) {
		qWarning("ToolController::continueDrawing: no model set!");
		return;
	}

	if(m_smoothing>0 && m_activeTool->allowSmoothing()) {
		m_smoother.addPoint(canvas::Point(point, pressure));

		if(m_smoother.hasSmoothPoint()) {
			m_activeTool->motion(m_smoother.smoothPoint(), shift, alt);
		}

	} else {
		m_activeTool->motion(canvas::Point(point, pressure), shift, alt);
	}

	m_prevShift = shift;
	m_prevAlt = alt;
}

void ToolController::hoverDrawing(const QPointF &point)
{
	Q_ASSERT(m_activeTool);
	if(!m_model)
		return;

	m_activeTool->hover(point);
}

void ToolController::endDrawing()
{
	Q_ASSERT(m_activeTool);

	if(!m_model) {
		qWarning("ToolController::endDrawing: no model set!");
		return;
	}

	// Drain any remaining points from the smoothing buffer
	if(m_smoothing>0 && m_activeTool->allowSmoothing()) {
		if(m_smoother.hasSmoothPoint())
			m_smoother.removePoint();
		while(m_smoother.hasSmoothPoint()) {
			m_activeTool->motion(m_smoother.smoothPoint(),
				m_prevShift, m_prevAlt);
			m_smoother.removePoint();
		}
	}

	m_activeTool->end();
#if 0 // FIXME
	m_model->stateTracker()->setLocalDrawingInProgress(false);
#endif
}

bool ToolController::undoMultipartDrawing()
{
	Q_ASSERT(m_activeTool);

	if(!m_model) {
		qWarning("ToolController::undoMultipartDrawing: no model set!");
		return false;
	}

	if(!m_activeTool->isMultipart())
		return false;

	m_activeTool->undoMultipart();
	return true;
}

bool ToolController::isMultipartDrawing() const
{
	Q_ASSERT(m_activeTool);

	return m_activeTool->isMultipart();
}

void ToolController::finishMultipartDrawing()
{
	Q_ASSERT(m_activeTool);

	if(!m_model) {
		qWarning("ToolController::finishMultipartDrawing: no model set!");
		return;
	}

	if(m_model->aclState()->isLayerLocked(m_activeLayer)) {
		// It is possible for the active layer to become locked
		// before the user has finished multipart drawing.
		qWarning("Cannot finish multipart drawing: active layer is locked!");
		return;
	}

	m_smoother.reset();
	m_activeTool->finishMultipart();
}

void ToolController::cancelMultipartDrawing()
{
	Q_ASSERT(m_activeTool);

	if(!m_model) {
		qWarning("ToolController::cancelMultipartDrawing: no model set!");
		return;
	}

	m_smoother.reset();
	m_activeTool->cancelMultipart();
}

void ToolController::offsetActiveTool(int xOffset, int yOffset)
{
	Q_ASSERT(m_activeTool);
	m_activeTool->offsetActiveTool(xOffset, yOffset);
	m_smoother.addOffset(QPointF(xOffset, yOffset));
}

void ToolController::setBrushEngineBrush(rustpile::BrushEngine *be, bool freehand)
{
	activeBrush().setInBrushEngine(be, activeLayer(), freehand);
}

}
