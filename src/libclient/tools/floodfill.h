// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TOOLS_FLOODFILL_H
#define TOOLS_FLOODFILL_H

#include "libclient/tools/tool.h"

namespace tools {

class FloodFill final : public Tool
{
public:
	FloodFill(ToolController &owner);

	void begin(const canvas::Point& point, bool right, float zoom) override;
	void motion(const canvas::Point& point, bool constrain, bool center) override;
	void end() override;

	void setTolerance(qreal tolerance) { m_tolerance = tolerance; }
	void setExpansion(int expansion) { m_expansion = expansion; }
	void setSizeLimit(unsigned int limit) { m_sizelimit = qMax(100u, limit); }
	void setSampleMerged(bool sm) { m_sampleMerged = sm; }
	void setUnderFill(bool uf) { m_underFill = uf; }
	void setEraseMode(bool erase) { m_eraseMode = erase; }

private:
	qreal m_tolerance;
	int m_expansion;
	unsigned int m_sizelimit;
	bool m_sampleMerged;
	bool m_underFill;
	bool m_eraseMode;
};

}

#endif
