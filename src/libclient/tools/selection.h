// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TOOLS_SELECTION_H
#define TOOLS_SELECTION_H

#include "libclient/canvas/selection.h"
#include "libclient/tools/tool.h"

class QImage;
class QPolygon;

namespace tools {

/**
 * @brief Base class for selection tool
 *
 * These are used for selecting regions for copying & pasting, as well
 * as filling regions with solid color.
 */
class SelectionTool : public Tool {
public:
	SelectionTool(ToolController &owner, Type type, QCursor cursor)
		: Tool(owner, type,  cursor), m_allowTransform(true) { }

	void begin(const canvas::Point& point, bool right, float zoom) override;
	void motion(const canvas::Point& point, bool constrain, bool center) override;
	void end() override;

	void finishMultipart() override;
	void cancelMultipart() override;
	void undoMultipart() override;
	bool isMultipart() const override;

	//! Start a layer region move operation
	void startMove();

	//! Allow selection moving and resizing
	void setTransformEnabled(bool enable) { m_allowTransform = enable; }

	static QImage transformSelectionImage(const QImage &source, const QPolygon &target, QPoint *offset);
	static QImage shapeMask(const QColor &color, const QPolygonF &selection, QRect *maskBounds);

protected:
	virtual void initSelection(canvas::Selection *selection) = 0;
	virtual void newSelectionMotion(const canvas::Point &point, bool constrain, bool center) = 0;

	QPointF m_start, m_p1, m_end;
	canvas::Selection::Handle m_handle;

private:
	bool m_allowTransform;
};

class RectangleSelection : public SelectionTool {
public:
	RectangleSelection(ToolController &owner);

protected:
	void initSelection(canvas::Selection *selection);
	void newSelectionMotion(const canvas::Point &point, bool constrain, bool center);
};

class PolygonSelection : public SelectionTool {
public:
	PolygonSelection(ToolController &owner);

protected:
	void initSelection(canvas::Selection *selection);
	void newSelectionMotion(const canvas::Point &point, bool constrain, bool center);
};

}

#endif
