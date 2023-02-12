// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TOOLS_TOOL_H
#define TOOLS_TOOL_H

#include "libclient/canvas/point.h"

#include <QCursor>

/**
 * @brief Tools
 *
 * Tools translate commands from the local user into messages that
 * can be sent over the network.
 * Read-only tools can access the canvas directly.
 */
namespace tools {

class ToolController;

/**
 * @brief Base class for all tools
 * Tool classes interpret mouse/pen commands into editing actions.
 */
class Tool
{
public:
	enum Type {
		FREEHAND, ERASER, LINE, RECTANGLE, ELLIPSE, BEZIER,
		FLOODFILL, ANNOTATION,
		PICKER, LASERPOINTER,
		SELECTION, POLYGONSELECTION,
		ZOOM, INSPECTOR,
		_LASTTOOL};

	Tool(ToolController &owner, Type type, const QCursor &cursor)
		: owner(owner), m_type(type), m_cursor(cursor)
		{}
	virtual ~Tool() {}

	Type type() const { return m_type; }
	const QCursor &cursor() const { return m_cursor; }

	/**
	 * @brief Start a new stroke
	 * @param point starting point
	 * @param right is the right mouse/pen button pressed instead of the left one
	 * @param zoom the current view zoom factor
	 */
	virtual void begin(const canvas::Point& point, bool right, float zoom) = 0;

	/**
	 * @brief Continue a stroke
	 * @param point new point
	 * @param constrain is the "constrain motion" button pressed
	 * @param cener is the "center on start point" button pressed
	 */
	virtual void motion(const canvas::Point& point, bool constrain, bool center) = 0;

	/**
	 * @brief Tool hovering over the canvas
	 * @param point tool position
	 */
	virtual void hover(const QPointF &point) { Q_UNUSED(point); }

	//! End stroke
	virtual void end() = 0;

	//! Finish and commit a multipart stroke
	virtual void finishMultipart() { }

	//! Cancel the current multipart stroke (if any)
	virtual void cancelMultipart() { }

	//! Undo the latest step of a multipart stroke. Undoing the first part should cancel the stroke
	virtual void undoMultipart() { }

	//! Is there a multipart stroke in progress at the moment?
	virtual bool isMultipart() const { return false; }

	//! Add an offset to this tool's current position (if active)
	virtual void offsetActiveTool(int x, int y) { Q_UNUSED(x) Q_UNUSED(y) /* most tools don't need to do anything here */ }

	//! Does this tool allow stroke smoothing to be used?
	virtual bool allowSmoothing() const { return false; }

protected:
	ToolController &owner;

private:
	const Type m_type;
	const QCursor m_cursor;
};

}

#endif
