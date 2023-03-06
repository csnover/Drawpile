// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SERVERLOGPAGE_H
#define SERVERLOGPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>

namespace  server {

struct JsonApiResult;

namespace gui {

class ServerLogPage : public QWidget
{
	Q_OBJECT
public:
	struct Private;

	explicit ServerLogPage(Server *server, QWidget *parent=nullptr);
	~ServerLogPage();

private slots:
	void handleResponse(const QString &requestId, const JsonApiResult &result);

private:
	void refreshPage();

	Private *d;
};

class ServerLogPageFactory : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("serverlog"); }
	QString title() const override { return PageFactory::tr("Server log"); }

	ServerLogPage *makePage(Server *server) const override { return new ServerLogPage(server); }
};

}
}

#endif
