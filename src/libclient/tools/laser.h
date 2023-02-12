// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TOOLS_LASER_H
#define TOOLS_LASER_H

#include "libclient/tools/tool.h"

namespace tools {

class LaserPointer : public Tool {
public:
	LaserPointer(ToolController &owner);

	void begin(const canvas::Point& point, bool right, float zoom) override;
	void motion(const canvas::Point& point, bool constrain, bool center) override;
	void end() override;

	bool allowSmoothing() const override { return true; }

	void setPersistence(int p) { m_persistence = p; }

private:
	int m_persistence;
	bool m_drawing;
};

}

#endif
