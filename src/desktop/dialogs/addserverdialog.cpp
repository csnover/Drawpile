// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/dialogs/addserverdialog.h"
#include "desktop/main.h"
#include "libshared/util/networkaccess.h"
#include "libclient/utils/listservermodel.h"

#include <QIcon>
#include <QPushButton>

namespace dialogs {

AddServerDialog::AddServerDialog(QWidget *parent)
	 : QMessageBox(
		QMessageBox::Icon::NoIcon,
		AddServerDialog::tr("Add Server"),
		QString(),
		QMessageBox::Cancel,
		parent)
	, m_servers(nullptr)
{
	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_DeleteOnClose);
}

void AddServerDialog::setListServerModel(sessionlisting::ListServerModel *model)
{
	m_servers = model;
}

void AddServerDialog::query(const QUrl &url)
{
	auto *response = sessionlisting::getApiInfo(url);
	connect(response, &sessionlisting::AnnouncementApiResponse::finished, this, [this, response](const QVariant &result, const QString&, const QString &error) {
		if(error.isEmpty()) {
			m_serverInfo = result.value<sessionlisting::ListServerInfo>();
			m_url = response->apiUrl();
			showSuccess();
		} else {
			showError(error);
		}
		response->deleteLater();
	});
}

void AddServerDialog::showError(const QString &errorMessage)
{
	setIcon(Icon::Warning);
	setText(errorMessage);
	show();
}

void AddServerDialog::showSuccess()
{
	setText(QStringLiteral("<b>%1</b>").arg(m_serverInfo.name.toHtmlEscaped()));
	setInformativeText(m_serverInfo.description.toHtmlEscaped());
	auto *btn = addButton(tr("Add"), AcceptRole);
	connect(btn, &QPushButton::clicked, this, &AddServerDialog::onAddClicked);
	show();

	if(m_serverInfo.faviconUrl == "drawpile") {
		const auto iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, this);
		const auto icon = QIcon(":/icons/drawpile.png").pixmap(iconSize);
		m_favicon = icon.toImage();
		setIconPixmap(icon);
	} else {
		const QUrl faviconUrl(m_serverInfo.faviconUrl);
		if(faviconUrl.isValid()) {
			auto *filedownload = new networkaccess::FileDownload(this);

			filedownload->setExpectedType("image/");

			connect(filedownload, &networkaccess::FileDownload::finished, this, [filedownload, this](const QString &errorMessage) {
				filedownload->deleteLater();
				if(!errorMessage.isEmpty()) {
					qWarning("Couldn't fetch favicon: %s", qPrintable(errorMessage));
					return;
				}

				if(!m_favicon.load(filedownload->file(), nullptr)) {
					qWarning("Couldn't load favicon.");
					return;
				}

				setIconPixmap(QPixmap::fromImage(m_favicon));
			});

			filedownload->start(faviconUrl);
		}
	}
}

void AddServerDialog::onAddClicked()
{
	sessionlisting::ListServerModel *listservers;
	if(m_servers) {
		listservers = m_servers;
	} else {
		listservers = new sessionlisting::ListServerModel(dpApp().settings(), true, this);
	}

	const auto url = m_url.toString();

	listservers->addServer(
		m_serverInfo.name,
		url,
		m_serverInfo.description,
		m_serverInfo.readOnly,
		m_serverInfo.publicListings,
		m_serverInfo.privateListings
	);

	if(!m_favicon.isNull()) {
		const auto iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, this);
		listservers->setFavicon(
			url,
			QIcon(":/icons/drawpile.png").pixmap(iconSize).toImage()
			);
	}

	listservers->submit();

	emit serverAdded(m_serverInfo.name);
}

}
