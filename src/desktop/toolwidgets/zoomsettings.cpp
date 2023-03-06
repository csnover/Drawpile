// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/toolwidgets/zoomsettings.h"
#include "desktop/utils/dynamicui.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

namespace tools {

ZoomSettings::ZoomSettings(ToolController *ctrl, QObject *parent)
	: ToolSettings(ctrl, parent)
{
}

QWidget *ZoomSettings::createUiWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	auto layout = new QVBoxLayout(widget);

	auto *resetButton = new QPushButton(widget);
	AUTO_TR(resetButton, setText, tr("Normal Size"));
	auto *fitButton = new QPushButton(widget);
	AUTO_TR(fitButton, setText, tr("Fit To Window"));

	connect(resetButton, &QPushButton::clicked, this, &ZoomSettings::resetZoom);
	connect(fitButton, &QPushButton::clicked, this, &ZoomSettings::fitToWindow);

	layout->addWidget(resetButton);
	layout->addWidget(fitButton);
	layout->addStretch(1);

	return widget;
}

}
