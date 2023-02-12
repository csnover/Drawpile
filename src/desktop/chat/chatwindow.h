// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>

namespace widgets {

class ChatWindow : public QWidget
{
	Q_OBJECT
public:
	ChatWindow(QWidget *content);

signals:
	void closing();

protected:
	void closeEvent(QCloseEvent *event) override;
};

}

#endif // CHATWINDOW_H
