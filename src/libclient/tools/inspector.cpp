// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/canvas/canvasmodel.h"

#include "libclient/tools/toolcontroller.h"
#include "libclient/tools/inspector.h"

namespace tools {

Inspector::Inspector(ToolController &owner)
	: Tool(owner, INSPECTOR, QCursor(Qt::WhatsThisCursor))
{
}

void Inspector::begin(const canvas::Point& point, bool right, float zoom)
{
	Q_UNUSED(zoom);
	Q_UNUSED(right);
	m_owner.model()->inspectCanvas(point.x(), point.y());
}

void Inspector::motion(const canvas::Point& point, bool constrain, bool center)
{
	Q_UNUSED(point);
	Q_UNUSED(constrain);
	Q_UNUSED(center);
}

void Inspector::end()
{
	m_owner.model()->stopInspectingCanvas();
}

void Inspector::cancelMultipart()
{
	end();
}

}
