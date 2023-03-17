// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SERVERLOGPAGE_H
#define SERVERLOGPAGE_H

#include "thinsrv/gui/pagefactory.h"

#include <QWidget>
#include <QApplication>
#include <memory>

namespace server {

struct JsonApiResult;

namespace gui {

class ServerLogPage final : public QWidget
{
	Q_OBJECT
public:
	explicit ServerLogPage(Server *server, QWidget *parent=nullptr);
	~ServerLogPage() override;

private slots:
	void handleResponse(const QString &requestId, const JsonApiResult &result);

private:
	void refreshPage();

	struct Private;
	const std::unique_ptr<Private> d;
};

class ServerLogPageFactory final : public PageFactory
{
public:
	QString pageId() const override { return QStringLiteral("serverlog"); }
	QString title() const override { return PageFactory::tr("Server log"); }

	ServerLogPage *makePage(Server *server) const override { return new ServerLogPage(server); }
};

}
}

#endif
