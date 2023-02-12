// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TOOLSETTINGS_FILL_H
#define TOOLSETTINGS_FILL_H

#include "desktop/toolwidgets/toolsettings.h"

class Ui_FillSettings;

namespace tools {

/**
 * @brief Settings for the flood fill tool
 */
class FillSettings : public ToolSettings {
	Q_OBJECT
public:
	FillSettings(ToolController *ctrl, QObject *parent=nullptr);
	~FillSettings();

	QString toolType() const override { return QStringLiteral("fill"); }

	void quickAdjust1(qreal adjustment) override;
	void setForeground(const QColor &color) override;

	int getSize() const override { return 0; }
	bool getSubpixelMode() const override { return false; }

	ToolProperties saveToolSettings() override;
	void restoreToolSettings(const ToolProperties &cfg) override;

public slots:
	void pushSettings() override;
	void toggleEraserMode() override;

protected:
	QWidget *createUiWidget(QWidget *parent) override;

private:
	Ui_FillSettings * _ui;
	qreal m_quickAdjust1 = 0.0;
};

}

#endif
