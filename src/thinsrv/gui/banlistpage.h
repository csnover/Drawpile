// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef BANLISTPAGE_H
#define BANLISTPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>
#include <memory>

namespace server {

struct JsonApiResult;

namespace gui {

class BanListPage final : public QWidget
{
	Q_OBJECT
public:
	explicit BanListPage(Server *server, QWidget *parent=nullptr);
	~BanListPage() override;

private slots:
	void handleResponse(const QString &requestId, const JsonApiResult &result);

	void addNewBan();
	void removeSelectedBan();

private:
	void refreshPage();

	struct Private;
	const std::unique_ptr<Private> d;
};

class BanListPageFactory final : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("banlist"); }
	QString title() const override { return PageFactory::tr("IP bans"); }

	BanListPage *makePage(Server *server) const override { return new BanListPage(server); }
};

}
}

#endif
