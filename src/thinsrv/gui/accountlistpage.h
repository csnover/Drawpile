// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef ACCOUNTLISTPAGE_H
#define ACCOUNTLISTPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>

namespace  server {

struct JsonApiResult;

namespace gui {

class AccountListPage : public QWidget
{
	Q_OBJECT
public:
	struct Private;

	explicit AccountListPage(Server *server, QWidget *parent=nullptr);
	~AccountListPage();

private slots:
	void handleResponse(const QString &requestId, const JsonApiResult &result);

	void addNewAccount();
	void editSelectedAccount();
	void removeSelectedAccount();

private:
	void refreshPage();

	Private *d;
};

class AccountListPageFactory : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("accountlist"); }
	QString title() const override { return QApplication::tr("Accounts"); }

	AccountListPage *makePage(Server *server) const override { return new AccountListPage(server); }
};

}
}

#endif
