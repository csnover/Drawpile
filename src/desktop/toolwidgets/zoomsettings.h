// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TOOLSETTINGS_ZOOM_H
#define TOOLSETTINGS_ZOOM_H

#include "desktop/toolwidgets/toolsettings.h"

namespace tools {

/**
 * @brief Zoom tool options
 */
class ZoomSettings final : public ToolSettings {
	Q_OBJECT
public:
	ZoomSettings(ToolController *ctrl, QObject *parent=nullptr);

	QString toolType() const override { return QStringLiteral("zoom"); }

	void setForeground(const QColor &color) override { Q_UNUSED(color); }

	int getSize() const override { return 0; }
	bool getSubpixelMode() const override { return false; }

signals:
	void resetZoom();
	void fitToWindow();

protected:
	QWidget *createUiWidget(QWidget *parent) override;
};

}

#endif
