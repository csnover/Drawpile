// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef ACCOUNTLISTPAGE_H
#define ACCOUNTLISTPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>
#include <memory>

namespace server {

struct JsonApiResult;

namespace gui {

class AccountListPage final : public QWidget
{
	Q_OBJECT
public:
	explicit AccountListPage(Server *server, QWidget *parent=nullptr);
	~AccountListPage() override;

private slots:
	void handleResponse(const QString &requestId, const JsonApiResult &result);

	void addNewAccount();
	void editSelectedAccount();
	void removeSelectedAccount();

private:
	void refreshPage();

	struct Private;
	const std::unique_ptr<Private> d;
};

class AccountListPageFactory final : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("accountlist"); }
	QString title() const override { return PageFactory::tr("Accounts"); }

	AccountListPage *makePage(Server *server) const override { return new AccountListPage(server); }
};

}
}

#endif
