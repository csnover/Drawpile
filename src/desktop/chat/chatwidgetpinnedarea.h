// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CHATWIDGETPINNEDAREA_H
#define CHATWIDGETPINNEDAREA_H

#include <QLabel>

namespace widgets {

class ChatWidgetPinnedArea : public QLabel
{
	Q_OBJECT
public:
	explicit ChatWidgetPinnedArea(QWidget *parent = nullptr);
	void setPinText(const QString &);

protected:
	void mouseDoubleClickEvent(QMouseEvent *event);
};

}

#endif // CHATWIDGETPINNEDAREA_H
