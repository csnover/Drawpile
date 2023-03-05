// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/chat/chatwindow.h"
#include "desktop/utils/dynamicui.h"

#include <QVBoxLayout>

namespace widgets {

ChatWindow::ChatWindow(QWidget *content)
	: QWidget()
{
	AUTO_TR(this, setWindowTitle, tr("Chat"));
	setAttribute(Qt::WA_DeleteOnClose);
	auto *layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
	layout->addWidget(content);
}

void ChatWindow::closeEvent(QCloseEvent *event)
{
	emit closing();
	QWidget::closeEvent(event);
}

}
