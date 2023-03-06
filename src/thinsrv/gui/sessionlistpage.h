// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SESSIONLISTPAGE_H
#define SESSIONLISTPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>

namespace  server {

struct JsonApiResult;

namespace gui {

class SessionListModel;

class SessionListPage : public QWidget
{
	Q_OBJECT
public:
	struct Private;

	explicit SessionListPage(Server *server, QWidget *parent=nullptr);
	~SessionListPage();

private slots:
	void sendMessageToAll();

private:
	Private *d;
};

class SessionListPageFactory : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("sessionlist"); }
	QString title() const override { return PageFactory::tr("Sessions"); }

	SessionListPage *makePage(Server *server) const override { return new SessionListPage(server); }
};

}
}

#endif
