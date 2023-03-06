// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef USERLISTPAGE_H
#define USERLISTPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>

namespace server {

struct JsonApiResult;

namespace gui {

class UserListPage : public QWidget
{
	Q_OBJECT
public:
	struct Private;
	explicit UserListPage(Server *server, QWidget *parent=nullptr);
	~UserListPage();

private slots:
	void handleResponse(const QString &requestId, const JsonApiResult &result);

private:
	void refreshPage();

	Private *d;
};

class UserListPageFactory : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("summary:users"); }
	QString title() const override { return PageFactory::tr("Users"); }
	UserListPage *makePage(Server *server) const override { return new UserListPage(server); }
};

}
}

#endif
