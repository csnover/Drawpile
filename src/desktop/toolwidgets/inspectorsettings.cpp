// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/toolwidgets/inspectorsettings.h"
#include "desktop/utils/dynamicui.h"
#include "libclient/tools/toolcontroller.h"
#include "libclient/canvas/userlist.h"

#include "ui_inspectorsettings.h"

namespace tools {

InspectorSettings::InspectorSettings(ToolController *ctrl, QObject *parent)
	: ToolSettings(ctrl, parent), m_ui(nullptr), m_userlist(nullptr)
{
}

InspectorSettings::~InspectorSettings()
{}

QWidget *InspectorSettings::createUiWidget(QWidget *parent)
{
	auto widget = new QWidget(parent);
	m_ui.reset(new Ui_InspectorSettings);
	m_ui->setupUi(widget);
	makeTranslator(widget, [=] {
		m_ui->retranslateUi(widget);
	});

	return widget;
}

void InspectorSettings::onCanvasInspected(int lastEditedBy)
{

	if(m_userlist) {
		const canvas::User u = m_userlist->getUserById(lastEditedBy);

		m_ui->lblAvatar->setPixmap(u.avatar);
		m_ui->lblUsername->setText(u.name);

	} else {
		m_ui->lblUsername->setText(tr("User %1").arg(lastEditedBy));
	}
}

}
