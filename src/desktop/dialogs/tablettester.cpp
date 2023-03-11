// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/tablettester.h"
#include "desktop/utils/dynamicui.h"
#include "ui_tablettest.h"

#include "desktop/main.h"

namespace dialogs {

DP_DYNAMIC_DEFAULT_IMPL(TabletTestDialog)

TabletTestDialog::TabletTestDialog(QWidget *parent)
	: DynamicUiWidget(parent)
{
	connect(static_cast<DrawpileApp*>(qApp), &DrawpileApp::eraserNear, this, [this](bool near) {
		QString msg;
		if(near)
			msg = tr("Eraser brought near");
		else
			msg = tr("Eraser taken away");

		m_ui->logView->appendPlainText(msg);
	});
}

TabletTestDialog::~TabletTestDialog()
{}

}
