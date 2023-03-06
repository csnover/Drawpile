// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/toolwidgets/toolsettings.h"
#include "libclient/tools/toolproperties.h"
#include "libclient/tools/toolcontroller.h"

namespace tools {

QWidget *ToolSettings::createUi(QWidget *parent)
{
	Q_ASSERT(!m_widget);
	m_widget = createUiWidget(parent);
	return m_widget;
}

void ToolSettings::pushSettings()
{
	// Default implementation has no settings
}

ToolProperties ToolSettings::saveToolSettings()
{
	return ToolProperties();
}

void ToolSettings::restoreToolSettings(const ToolProperties &)
{
}

}
