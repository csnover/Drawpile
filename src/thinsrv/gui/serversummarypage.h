// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SERVERSUMMARYPAGE_H
#define SERVERSUMMARYPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>

namespace server {

struct JsonApiResult;

namespace gui {

class ServerSummaryPage : public QWidget
{
	Q_OBJECT
public:
	struct Private;
	explicit ServerSummaryPage(Server *server, QWidget *parent=nullptr);
	~ServerSummaryPage();

private slots:
	void startOrStopServer();
	void handleResponse(const QString &requestId, const JsonApiResult &result);

	void saveSettings();

	void showSettingsDialog();

private:
	void refreshPage();

	Private *d;
};

class ServersummaryPageFactory : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("summary:server"); }
	QString title() const override { return QApplication::tr("Settings"); }
	ServerSummaryPage *makePage(Server *server) const override { return new ServerSummaryPage(server); }
};

}
}

#endif
