// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/dialogs/joindialog.h"
#include "desktop/dialogs/addserverdialog.h"
#include "desktop/dialogs/settingsdialog/helpers.h"
#include "desktop/main.h"
#include "desktop/utils/mandatoryfields.h"
#include "libclient/utils/usernamevalidator.h"
#include "libclient/utils/listservermodel.h"
#include "libclient/utils/sessionfilterproxymodel.h"
#include "libclient/utils/images.h"
#include "libclient/utils/newversion.h"
#include "libshared/listings/announcementapi.h"
#include "libclient/contentfilter/contentfilter.h"

#ifdef HAVE_DNSSD
#include "libshared/listings/zeroconfdiscovery.h"
#endif

#include "ui_joindialog.h"

#include <QIcon>
#include <QPushButton>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QFileDialog>

namespace dialogs {

// Height below which the session listing widgets are hidden.
static const int COMPACT_MODE_THRESHOLD = 300;

// How often the listing view should be refreshed (in seconds)
static const int REFRESH_INTERVAL = 60;

JoinDialog::JoinDialog(const QUrl &url, QWidget *parent)
	: QDialog(parent), m_lastRefresh(0)
{
	m_ui = new Ui_JoinDialog;
	m_ui->setupUi(this);
	m_ui->buttons->button(QDialogButtonBox::Ok)->setText(tr("Join"));
	m_ui->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

	m_addServerButton = m_ui->buttons->addButton(tr("Add Server"), QDialogButtonBox::ActionRole);
	m_addServerButton->setIcon(QIcon::fromTheme("list-add"));
	connect(m_addServerButton, &QPushButton::clicked, this, &JoinDialog::addListServer);

	connect(m_ui->address, &QComboBox::editTextChanged, this, &JoinDialog::addressChanged);
	connect(m_ui->autoRecord, &QAbstractButton::clicked, this, &JoinDialog::recordingToggled);

	// New version notification (cached)
	m_ui->newVersionNotification->setVisible(NewVersionCheck::isThereANewSeries(dpApp().settings()));

	// Session listing
	if(contentfilter::level() != contentfilter::Level::Unrestricted)
		m_ui->showNsfw->setEnabled(false);

	m_sessions = new SessionListingModel(this);

#ifdef HAVE_DNSSD
	if(ZeroconfDiscovery::isAvailable()) {
		auto zeroconfDiscovery = new ZeroconfDiscovery(this);
		m_sessions->setMessage(tr("Nearby"), tr("Loading..."));
		connect(zeroconfDiscovery, &ZeroconfDiscovery::serverListUpdated, this, [this](const QVector<sessionlisting::Session> &servers) {
			m_sessions->setList(tr("Nearby"), servers);
		});
		zeroconfDiscovery->discover();
	}
#endif

	const auto servers = sessionlisting::ListServerModel::listServers(dpApp().settings().listServers(), true);
	for(const sessionlisting::ListServer &ls : servers) {
		if(ls.publicListings)
			m_sessions->setMessage(ls.name, tr("Loading..."));
	}
	m_ui->noListServersNotification->setVisible(servers.isEmpty());

	m_filteredSessions = new SessionFilterProxyModel(this);
	m_filteredSessions->setSourceModel(m_sessions);
	m_filteredSessions->setFilterCaseSensitivity(Qt::CaseInsensitive);
	m_filteredSessions->setSortCaseSensitivity(Qt::CaseInsensitive);
	m_filteredSessions->setFilterKeyColumn(-1);
	m_filteredSessions->setSortRole(Qt::UserRole);

	m_filteredSessions->setShowNsfw(false);
	m_filteredSessions->setShowPassworded(false);

	connect(m_ui->showPassworded, &QAbstractButton::toggled,
			m_filteredSessions, &SessionFilterProxyModel::setShowPassworded);
	connect(m_ui->showNsfw, &QAbstractButton::toggled,
			m_filteredSessions, &SessionFilterProxyModel::setShowNsfw);
	connect(m_ui->showClosed, &QAbstractButton::toggled,
			m_filteredSessions, &SessionFilterProxyModel::setShowClosed);
	connect(m_ui->filter, &QLineEdit::textChanged,
			m_filteredSessions, &SessionFilterProxyModel::setFilterFixedString);

	m_ui->listing->setModel(m_filteredSessions);
	m_ui->listing->expandAll();

	QHeaderView *header = m_ui->listing->header();
	header->setSectionResizeMode(
		SessionListingModel::Version, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(
		SessionListingModel::Title, QHeaderView::Stretch);
	header->setSectionResizeMode(
		SessionListingModel::Server, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(
		SessionListingModel::UserCount, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(
		SessionListingModel::Owner, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(
		SessionListingModel::Uptime, QHeaderView::ResizeToContents);

	connect(m_ui->listing, &QTreeView::clicked, this, [this](const QModelIndex &index) {
		// Set the server URL when clicking on an item
		if((index.flags() & Qt::ItemIsEnabled))
			m_ui->address->setCurrentText(index.data(SessionListingModel::UrlRole).value<QUrl>().toString());
	});

	connect(m_ui->listing, &QTreeView::doubleClicked, [this](const QModelIndex &index) {
		// Shortcut: double click to OK
		if((index.flags() & Qt::ItemIsEnabled) && m_ui->buttons->button(QDialogButtonBox::Ok)->isEnabled())
			accept();
	});

	new MandatoryFields(this, m_ui->buttons->button(QDialogButtonBox::Ok));

	restoreSettings();

	if(!url.isEmpty()) {
		m_ui->address->setCurrentText(url.toString());

		const QUrlQuery q(url);
		QUrl addListServer = q.queryItemValue("list-server");
		if(addListServer.isValid() && !addListServer.isEmpty()) {
			addListServerUrl(addListServer);
		}
	}

	// Periodically refresh the session listing
	auto refreshTimer = new QTimer(this);
	connect(refreshTimer, &QTimer::timeout,
			this, &JoinDialog::refreshListing);
	refreshTimer->setSingleShot(false);
	refreshTimer->start(1000 * (REFRESH_INTERVAL + 1));

	refreshListing();
}

JoinDialog::~JoinDialog()
{
	delete m_ui;
}

void JoinDialog::resizeEvent(QResizeEvent *event)
{
	QDialog::resizeEvent(event);
	bool show = false;
	bool change = false;
	if(height() < COMPACT_MODE_THRESHOLD && !m_ui->filter->isHidden()) {
		show = false;
		change = true;
	} else if(height() > COMPACT_MODE_THRESHOLD && m_ui->filter->isHidden()) {
		show = true;
		change = true;
	}

	if(change)
		setListingVisible(show);

	dpApp().settings().setLastJoinDialogSize(size());
}

void JoinDialog::setListingVisible(bool show)
{
	m_ui->filter->setVisible(show);
	m_ui->filterLabel->setVisible(show);
	m_ui->showPassworded->setVisible(show);
	m_ui->showNsfw->setVisible(show);
	m_ui->showClosed->setVisible(show);
	m_ui->listing->setVisible(show);
	m_ui->line->setVisible(show);

	if(show)
		refreshListing();
}

static QString cleanAddress(const QString &addr)
{
	if(addr.startsWith("drawpile://")) {
		QUrl url(addr);
		if(url.isValid()) {
			QString a = url.host();
			if(url.port()!=-1)
				a += ":" + QString::number(url.port());
			return a;
		}
	}
	return addr;
}

static bool isRoomcode(const QString &str) {
	// Roomcodes are always exactly 5 letters long
	if(str.length() != 5)
		return false;

	// And consist of characters in range A-Z
	for(int i=0;i<str.length();++i)
		if(str.at(i) < 'A' || str.at(i) > 'Z')
			return false;

	return true;
}

void JoinDialog::addressChanged(const QString &addr)
{
	m_addServerButton->setEnabled(!addr.isEmpty());

	if(isRoomcode(addr)) {
		// A room code was just entered. Trigger session URL query
		m_ui->address->setEditText(QString());
		m_ui->address->lineEdit()->setPlaceholderText(tr("Searching..."));
		m_ui->address->lineEdit()->setReadOnly(true);

		QStringList servers;
		for(const auto &s : sessionlisting::ListServerModel::listServers(dpApp().settings().listServers(), true)) {
			if(s.privateListings)
				servers << s.url;
		}
		resolveRoomcode(addr, servers);
	}
}

void JoinDialog::recordingToggled(bool checked)
{
	if(checked) {
		m_recordingFilename = QFileDialog::getSaveFileName(
			this,
			tr("Record"),
			m_recordingFilename,
			utils::fileFormatFilter(utils::FileFormatOption::SaveRecordings)
		);
		if(m_recordingFilename.isEmpty())
			m_ui->autoRecord->setChecked(false);
	}
}

QString JoinDialog::autoRecordFilename() const
{
	return m_ui->autoRecord->isChecked() ? m_recordingFilename : QString();
}

void JoinDialog::refreshListing()
{
	if(m_ui->listing->isHidden() || QDateTime::currentSecsSinceEpoch() - m_lastRefresh < REFRESH_INTERVAL)
		return;
	m_lastRefresh = QDateTime::currentSecsSinceEpoch();

	const auto listservers = sessionlisting::ListServerModel::listServers(dpApp().settings().listServers(), true);
	for(const sessionlisting::ListServer &ls : listservers) {
		if(!ls.publicListings)
			continue;

		const QUrl url = ls.url;
		if(!url.isValid()) {
			qWarning("Invalid list server URL: %s", qPrintable(ls.url));
			continue;
		}

		auto response = sessionlisting::getSessionList(url);

		connect(response, &sessionlisting::AnnouncementApiResponse::serverGone,
			[ls]() {
				qInfo() << "List server at" << ls.url << "is gone. Removing.";
				sessionlisting::ListServerModel servers(dpApp().settings(), true);
				if(servers.removeServer(ls.url))
					servers.submit();
			});
		connect(response, &sessionlisting::AnnouncementApiResponse::finished,
			this, [this, ls](const QVariant &result, const QString &message, const QString &error)
			{
				Q_UNUSED(message)
				if(error.isEmpty())
					m_sessions->setList(ls.name, result.value<QVector<sessionlisting::Session>>());
				else
					m_sessions->setMessage(ls.name, error);
			});
		connect(response, &sessionlisting::AnnouncementApiResponse::finished, response, &QObject::deleteLater);
	}
}

void JoinDialog::resolveRoomcode(const QString &roomcode, const QStringList &servers)
{
	if(servers.isEmpty()) {
		// Tried all the servers and didn't find the code
		m_ui->address->lineEdit()->setPlaceholderText(tr("Room code not found!"));
		QTimer::singleShot(1500, this, [this]() {
			m_ui->address->setEditText(QString());
			m_ui->address->lineEdit()->setReadOnly(false);
			m_ui->address->lineEdit()->setPlaceholderText(QString());
			m_ui->address->setFocus();
		});

		return;
	}

	const QUrl listServer = servers.first();
	qDebug() << "Querying join code" << roomcode << "at server:" << listServer;
	auto response = sessionlisting::queryRoomcode(listServer, roomcode);
	connect(response, &sessionlisting::AnnouncementApiResponse::finished,
		this, [this, roomcode, servers](const QVariant &result, const QString &message, const QString &error)
		{
			Q_UNUSED(message)
			if(!error.isEmpty()) {
				// Not found. Try the next server.
				resolveRoomcode(roomcode, servers.mid(1));
				return;
			}

			auto session = result.value<sessionlisting::Session>();

			QString url = "drawpile://" + session.host;
			if(session.port != 27750)
				url += QStringLiteral(":%1").arg(session.port);
			url += '/';
			url += session.id;
			m_ui->address->lineEdit()->setReadOnly(false);
			m_ui->address->lineEdit()->setPlaceholderText(QString());
			m_ui->address->setEditText(url);
			m_ui->address->setEnabled(true);
		}
	);
	connect(response, &sessionlisting::AnnouncementApiResponse::finished, response, &QObject::deleteLater);
}

void JoinDialog::restoreSettings()
{
	auto &settings = dpApp().settings();

	const QSize oldSize = settings.lastJoinDialogSize();
	if(oldSize.isValid()) {
		if(oldSize.height() < COMPACT_MODE_THRESHOLD)
			setListingVisible(false);
		resize(oldSize);
	}

	m_ui->address->insertItems(0, settings.recentHosts());

	settings.bindFilterLocked(m_ui->showPassworded);
	settings.bindFilterClosed(m_ui->showClosed);
	settings.bindFilterNsfm(m_ui->showNsfw);

	// Off by one because stored value uses bit 0 for sort order
	settings.bindJoinListSortColumn(m_ui->listing->header(), [=](int sortColumn) {
		m_ui->listing->sortByColumn(
			qAbs(sortColumn) - 1,
			sortColumn > 0 ? Qt::AscendingOrder : Qt::DescendingOrder
		);
	});
	connect(m_ui->listing->header(), &QHeaderView::sortIndicatorChanged, &settings, [&settings](int logicalIndex, Qt::SortOrder order) {
		settings.setJoinListSortColumn((logicalIndex + 1) * (order == Qt::AscendingOrder ? 1 : -1));
	});
}

void JoinDialog::rememberSettings() const
{
	auto &settings = dpApp().settings();
	// Move current item to the top of the list
	const QString current = cleanAddress(m_ui->address->currentText());
	int curindex = m_ui->address->findText(current);
	if(curindex>=0)
		m_ui->address->removeItem(curindex);

	QStringList hosts;
	auto max = settings.maxRecentFiles();
	if (max > 0) {
		hosts << current;
		--max;
		for (auto i = 0; max && i < m_ui->address->count(); ++i) {
			if (!m_ui->address->itemText(i).isEmpty()) {
				--max;
				hosts << m_ui->address->itemText(i);
			}
		}
	}
	settings.setRecentHosts(hosts);
}

QString JoinDialog::getAddress() const {
	return m_ui->address->currentText().trimmed();
}

QUrl JoinDialog::getUrl() const
{
	const QString address = getAddress();

	QString scheme;
	if(!address.startsWith("drawpile://"))
		scheme = "drawpile://";

	const QUrl url = QUrl(scheme + address, QUrl::TolerantMode);
	if(!url.isValid() || url.host().isEmpty())
		return QUrl();

	return url;
}

void JoinDialog::addListServer()
{
	// This is the "simplified" way of adding list servers:
	// The application will fetch the server's root page (http://DOMAIN/)
	// and see if there is a <meta name="drawpile:list-server"> tag.
	// If there is, it will follow it and add the list server.
	const auto urlString = m_ui->address->currentText().trimmed();
	auto url = QUrl::fromUserInput(urlString).adjusted(
		QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment
	);
	if (url.isValid() && url.scheme().startsWith("http")) {
		// Qt will default guessed user input to http, but it should really
		// default to https for privacy and security.
		if (!urlString.startsWith("http://", Qt::CaseInsensitive)) {
			url.setScheme("https");
		}
		addListServerUrl(url);
	} else {
		settingsdialog::execWarning(
			tr("Join a Session"),
			tr("'%1' is not a valid list server URL.").arg(urlString),
			this
		);
	}
}

void JoinDialog::addListServerUrl(const QUrl &url)
{
	m_addServerButton->setEnabled(false);

	auto *dlg = new AddServerDialog(this);

	connect(dlg, &QObject::destroyed, [this]() { m_addServerButton->setEnabled(true); });

	connect(dlg, &AddServerDialog::serverAdded, this, [this](const QString &name) {
		m_sessions->setMessage(name, tr("Loading..."));
		m_ui->noListServersNotification->hide();

		if(height() < COMPACT_MODE_THRESHOLD)
			resize(width(), COMPACT_MODE_THRESHOLD + 10);

		const auto index = m_sessions->index(m_sessions->rowCount()-1, 0);
		m_ui->listing->expandAll();
		m_ui->listing->scrollTo(index);

		m_lastRefresh = 0;
		refreshListing();
	});

	dlg->query(url);
}

}

