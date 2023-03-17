// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TOOLS_FREEHAND_H
#define TOOLS_FREEHAND_H

#include "libclient/tools/tool.h"

namespace rustpile {
	struct PaintEngineBrush;
}

namespace tools {

//! Freehand brush tool
class Freehand final : public Tool
{
public:
	Freehand(ToolController &owner, bool isEraser);
	~Freehand() override;

	void begin(const canvas::Point& point, bool right, float zoom) override;
	void motion(const canvas::Point& point, bool constrain, bool center) override;
	void end() override;

	bool allowSmoothing() const override { return true; }

	void offsetActiveTool(int x, int y) override;

private:
	rustpile::BrushEngine *m_brushengine;
	bool m_drawing;
	bool m_firstPoint;
	qint64 m_lastTimestamp;
	canvas::Point m_start;
};

}

#endif
