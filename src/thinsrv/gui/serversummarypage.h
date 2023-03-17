// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SERVERSUMMARYPAGE_H
#define SERVERSUMMARYPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>
#include <memory>

namespace server {

struct JsonApiResult;

namespace gui {

class ServerSummaryPage final : public QWidget
{
	Q_OBJECT
public:
	struct Private;
	explicit ServerSummaryPage(Server *server, QWidget *parent=nullptr);
	~ServerSummaryPage() override;

private slots:
	void startOrStopServer();
	void handleResponse(const QString &requestId, const JsonApiResult &result);

	void saveSettings();

	void showSettingsDialog();

private:
	void refreshPage();

	const std::unique_ptr<Private> d;
};

class ServersummaryPageFactory final : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("summary:server"); }
	QString title() const override { return PageFactory::tr("Settings"); }
	ServerSummaryPage *makePage(Server *server) const override { return new ServerSummaryPage(server); }
};

}
}

#endif
