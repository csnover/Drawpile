// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/dialogs/addserverdialog.h"
#include "desktop/dialogs/certificateview.h"
#include "desktop/dialogs/settingsdialog/helpers.h"
#include "desktop/dialogs/settingsdialog/servers.h"
#include "desktop/settings.h"
#include "desktop/utils/listserverdelegate.h"
#include "desktop/utils/sanerformlayout.h"
#include "desktop/widgets/groupedtoolbutton.h"
#include "libclient/utils/certificatestoremodel.h"
#include "libclient/utils/listservermodel.h"
#include "libshared/util/paths.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListView>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QSslCertificate>
#include <QStringList>
#include <QVBoxLayout>
#include <algorithm>
#include <optional>

namespace dialogs {
namespace settingsdialog {

Servers::Servers(desktop::settings::Settings &settings, QWidget *parent)
	: QWidget(parent)
{
	auto *layout = new QVBoxLayout(this);

	initListingServers(settings, layout);
	layout->addSpacing(6);
	initKnownHosts(layout);
}

void Servers::initKnownHosts(QVBoxLayout *form)
{
	auto *knownHostsLabel = new QLabel(tr("Known hosts:"));
	form->addWidget(knownHostsLabel);

	auto *knownHosts = new QListView;
	form->addWidget(knownHosts, 1);
	knownHostsLabel->setBuddy(knownHosts);
	knownHosts->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	knownHosts->setAlternatingRowColors(true);
	knownHosts->setFocusPolicy(Qt::StrongFocus);
	knownHosts->setSelectionMode(QAbstractItemView::ExtendedSelection);
	auto *knownHostsModel = new CertificateStoreModel(this);
	knownHosts->setModel(knownHostsModel);

	auto *actions = listActions(knownHosts,
		tr("Import trusted certificate…"),
		[=] { importCertificates(knownHostsModel); },

		tr("Remove selected hosts…"),
		makeDefaultDeleter(this, knownHosts,
			tr("Remove known hosts"),
			QT_TR_NOOP("Really remove %n known hosts?")
		)
	);

	actions->addStretch();

	auto *trustButton = new QPushButton(tr("Trust selected hosts"));
	trustButton->setAutoDefault(false);
	trustButton->setEnabled(knownHosts->selectionModel()->hasSelection());
	connect(knownHosts->selectionModel(), &QItemSelectionModel::selectionChanged, trustButton, [=] {
		trustButton->setEnabled(knownHosts->selectionModel()->hasSelection());
	});
	connect(trustButton, &QPushButton::clicked, knownHosts, [=] {
		trustCertificates(
			knownHostsModel,
			knownHosts->selectionModel()->selectedIndexes()
		);
	});
	actions->addWidget(trustButton);

	connect(knownHosts, &QListView::doubleClicked, this, [=](const QModelIndex &index) {
		viewCertificate(knownHostsModel, index);
	});

	form->addLayout(actions);
}

void Servers::initListingServers(desktop::settings::Settings &settings, QVBoxLayout *form)
{
	auto *serversLabel = new QLabel(tr("List servers:"));
	form->addWidget(serversLabel);

	auto *servers = new QListView;
	serversLabel->setBuddy(servers);
	auto *serversModel = new sessionlisting::ListServerModel(settings, true, this);
	servers->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	servers->setModel(serversModel);
	servers->setItemDelegate(new sessionlisting::ListServerDelegate(this));
	servers->setAlternatingRowColors(true);
	servers->setFocusPolicy(Qt::StrongFocus);
	servers->setSelectionMode(QAbstractItemView::ExtendedSelection);
	form->addWidget(servers, 1);
	form->addLayout(listActions(servers,
		tr("Add list servers…"),
		[=] { addListServer(serversModel); },

		tr("Remove selected list servers…"),
		makeDefaultDeleter(this, servers,
			tr("Remove list servers"),
			QT_TR_NOOP("Really remove %n list servers?")
		)
	));
}

void Servers::addListServer(sessionlisting::ListServerModel *model)
{
	auto urlString = QStringLiteral("https://");
	for (;;) {
		auto ok = true;
		urlString = QInputDialog::getText(
			this, tr("Add list server"), tr("Enter a list server URL:"),
			QLineEdit::Normal, urlString, &ok,
			Qt::Sheet, Qt::ImhUrlCharactersOnly
		).trimmed();
		if (!ok || urlString.isEmpty()) {
			return;
		}
		auto url = QUrl::fromUserInput(urlString);
		if (url.isValid() && url.scheme().startsWith("http")) {
			// Qt will default guessed user input to http, but it should really
			// default to https for privacy and security.
			if (!urlString.startsWith("http://", Qt::CaseInsensitive)) {
				url.setScheme("https");
			}

			auto *dialog = new AddServerDialog(this);
			dialog->setListServerModel(model);
			dialog->query(url);
			return;
		} else {
			const auto result = execWarning(
				tr("Add list server"),
				tr("'%1' is not a valid list server URL.").arg(urlString),
				this,
				QMessageBox::Retry | QMessageBox::Cancel,
				QMessageBox::Retry
			);
			if (result == QMessageBox::Cancel) {
				return;
			}
		}
	}
}

// TODO: Move to some utility module
static QStringList askForFiles(const QString &title, const QString &filter, const QString &action, QWidget *parent)
{
	QFileDialog dialog(parent, title, QString(), filter);
	dialog.setLabelText(QFileDialog::Accept, action);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setOption(QFileDialog::ReadOnly);
	dialog.setWindowModality(Qt::WindowModal);
	dialog.setSupportedSchemes({QStringLiteral("file")});
	if (dialog.exec() == QDialog::Accepted) {
		return dialog.selectedFiles();
	} else {
		return {};
	}
}

static bool askToContinue(const QString &title, const QString &message, QWidget *parent)
{
	QMessageBox box(
		QMessageBox::Warning,
		title,
		message,
		QMessageBox::Cancel,
		parent
	);
	const auto *ok = box.addButton(Servers::tr("Continue"), QMessageBox::AcceptRole);
	box.setDefaultButton(QMessageBox::Cancel);
	box.setWindowModality(Qt::WindowModal);
	box.exec();
	return box.clickedButton() == ok;
}

void Servers::importCertificates(CertificateStoreModel *model)
{
	const auto title = tr("Import trusted certificates");

	const auto paths = askForFiles(
		title,
		tr("Certificates (%1)").arg("*.pem *.crt *.cer") + ";;" +
		QApplication::tr("All files (*)"),
		tr("Import"),
		this
	);

	for (const auto &path : paths) {
		auto [ index, error ] = model->addCertificate(path, true);
		if (!error.isEmpty() && !askToContinue(title, error, this)) {
			model->revert();
			return;
		}
	}
	model->submit();
}

void Servers::trustCertificates(CertificateStoreModel *model, const QModelIndexList &indexes)
{
	for (const auto &index : indexes) {
		model->setData(index, true, CertificateStoreModel::TrustedRole);
	}
	if (!model->submit()) {
		execWarning(
			tr("Trust selected hosts"),
			tr("Could not save changes to known hosts: %1").arg(model->lastError()),
			this
		);
	}
}

void Servers::viewCertificate(CertificateStoreModel *model, const QModelIndex &index)
{
	const auto host = model->data(index, Qt::DisplayRole).toString();
	const auto cert = model->certificate(index);
	if (!cert) {
		return;
	}
	auto *cv = new CertificateView(host, *cert, this);
	cv->setWindowModality(Qt::WindowModal);
	cv->setAttribute(Qt::WA_DeleteOnClose);
	cv->show();
}

} // namespace settingsdialog
} // namespace dialogs
