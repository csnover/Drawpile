// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/logindialog.h"
#include "desktop/dialogs/abusereport.h"
#include "desktop/dialogs/certificateview.h"
#include "libclient/net/login.h"
#include "libclient/net/loginsessions.h"
#include "libclient/parentalcontrols/parentalcontrols.h"

#include "libclient/utils/avatarlistmodel.h"
#include "libclient/utils/avatarlistmodeldelegate.h"
#include "libclient/utils/sessionfilterproxymodel.h"
#include "libclient/utils/usernamevalidator.h"
#include "libclient/utils/html.h"
#include "libclient/utils/icon.h"

#include "ui_logindialog.h"

#ifdef HAVE_QTKEYCHAIN
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6keychain/keychain.h>
#else
#include <qt5keychain/keychain.h>
#endif
#endif

#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QStyle>
#include <QItemSelectionModel>
#include <QAction>
#include <QPointer>
#include <memory>

namespace dialogs {

enum class Mode {
	loading,         // used whenever we're waiting for the server
	identity,        // ask user for username
	authenticate,    // ask user for password (for login)
	sessionlist,     // select session to join
	sessionpassword, // ask user for password (for session)
	catchup,         // logged in: catching up (dialog can be closed at this point)
	certChanged      // SSL certificate has changed (can be ignored)
};

struct LoginDialog::Private {
	Mode mode;

	QPointer<net::LoginHandler> loginHandler;
	AvatarListModel *avatars;
	SessionFilterProxyModel *sessions;
	const std::unique_ptr<Ui_LoginDialog> m_ui;

	QPushButton *okButton;
	QPushButton *reportButton;

	QUrl extauthurl;
	QSslCertificate oldCert, newCert;

	QMetaObject::Connection loginDestructConnection;

	Private(net::LoginHandler *login, LoginDialog *dlg)
		: mode(Mode::loading)
		, loginHandler(login)
		, m_ui(new Ui_LoginDialog)
	{
		Q_ASSERT(loginHandler);

		m_ui->setupUi(dlg);

		m_ui->serverTitle->setVisible(false);

		// Identity & authentication page
		m_ui->username->setValidator(new UsernameValidator(dlg));
		avatars = new AvatarListModel(dlg);
		avatars->loadAvatars(true);
		m_ui->avatarList->setModel(avatars);


#ifndef HAVE_QTKEYCHAIN
		m_ui->rememberPassword->setEnabled(false);
#endif

		{
			const auto iconSize = m_ui->username->sizeHint().height();
			m_ui->avatarIcon->setPixmap(icon::fromTheme("photo").pixmap(iconSize));
			m_ui->usernameIcon->setPixmap(icon::fromTheme("im-user").pixmap(iconSize));
			m_ui->passwordIcon->setPixmap(icon::fromTheme("object-locked").pixmap(iconSize));
		}

		// Session list page
		QObject::connect(m_ui->sessionList, &QTableView::doubleClicked, [this](const QModelIndex&) {
			if(okButton->isEnabled())
				okButton->click();
		});

		m_ui->showNsfw->setEnabled(parentalcontrols::level() == parentalcontrols::Level::Unrestricted);
		sessions = new SessionFilterProxyModel(dlg);
		sessions->setSortRole(net::LoginSessionModel::SortRole);
		sessions->setFilterCaseSensitivity(Qt::CaseInsensitive);
		sessions->setFilterKeyColumn(-1);

		connect(m_ui->showNsfw, &QAbstractButton::toggled, [this](bool show) {
			QSettings().setValue("history/filternsfw", show);
			sessions->setShowNsfw(show);
		});
		connect(m_ui->filter, &QLineEdit::textChanged,
		        sessions, &SessionFilterProxyModel::setFilterFixedString);

		m_ui->sessionList->setModel(sessions);
		m_ui->sessionList->sortByColumn(net::LoginSessionModel::StatusColumn, Qt::AscendingOrder);

		// Cert changed page
		{
			const auto iconSize = dlg->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, dlg);
			m_ui->warningIcon->setText(QString());
			m_ui->warningIcon->setPixmap(dlg->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(iconSize));
		}

		// Buttons
		okButton = m_ui->buttonBox->button(QDialogButtonBox::Ok);
		okButton->setDefault(true);

		reportButton = m_ui->buttonBox->addButton(LoginDialog::tr("Report..."), QDialogButtonBox::ActionRole);
		reportButton->setEnabled(false); // needs a selected session to be enabled

		resetMode(Mode::loading);
	}

	~Private() {}

	void resetMode(Mode mode);
	void setLoginMode(const QString &prompt);
};

void LoginDialog::Private::resetMode(Mode newMode)
{
	if(!loginHandler) {
		qWarning("LoginDialog::resetMode: login process already ended!");
		return;
	}

	mode = newMode;

	QWidget *page = nullptr;

	okButton->setVisible(true);
	reportButton->setVisible(false);

	switch(mode) {
	case Mode::loading:
		okButton->setVisible(false);
		m_ui->loginPromptLabel->setText(loginHandler->url().host());
		page = m_ui->loadingPage;
		break;
	case Mode::identity:
		m_ui->avatarList->setEnabled(true);
		m_ui->username->setEnabled(true);
		m_ui->username->setFocus();
		m_ui->password->setVisible(false);
		m_ui->passwordIcon->setVisible(false);
		m_ui->badPasswordLabel->setVisible(false);
		m_ui->rememberPassword->setVisible(false);
		page = m_ui->authPage;
		break;
	case Mode::authenticate:
		m_ui->avatarList->setEnabled(false);
		m_ui->username->setEnabled(false);
		m_ui->password->setVisible(true);
		m_ui->passwordIcon->setVisible(true);
		m_ui->badPasswordLabel->setVisible(false);
		m_ui->rememberPassword->setVisible(true);
		m_ui->password->setFocus();
		page = m_ui->authPage;
		break;
	case Mode::sessionlist:
		reportButton->setVisible(true);
		page = m_ui->listingPage;
		break;
	case Mode::sessionpassword:
		m_ui->sessionPassword->setFocus();
		page = m_ui->sessionPasswordPage;
		break;
	case Mode::catchup:
		okButton->setVisible(false);
		m_ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(LoginDialog::tr("Close"));
		page = m_ui->catchupPage;
		break;
	case Mode::certChanged:
		page = m_ui->certChangedPage;
		break;
	}

	Q_ASSERT(page);
	m_ui->pages->setCurrentWidget(page);
}

#ifdef HAVE_QTKEYCHAIN
static const QString KEYCHAIN_NAME = QStringLiteral("Drawpile");

static QString keychainSecretName(const QString &username, const QUrl &extAuthUrl, const QString &server)
{
	QString prefix, host;
	if(extAuthUrl.isValid()) {
		prefix = "ext:";
		host = extAuthUrl.host();
	} else {
		prefix = "srv:";
		host = server;
	}

	return prefix + username.toLower() + "@" + host;
}
#endif

LoginDialog::LoginDialog(net::LoginHandler *login, QWidget *parent)
	: QDialog(parent)
	, d(new Private(login, this))
{
	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(login->url().host());

	d->m_ui->avatarList->setItemDelegate(new AvatarItemDelegate(this));
	connect(d->m_ui->avatarList->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &deselected) {
		if (selected.isEmpty()) {
			d->m_ui->avatarList->selectionModel()->select(deselected, QItemSelectionModel::SelectCurrent);
		}
	});
	connect(d->m_ui->username, &QLineEdit::textChanged, d->avatars, &AvatarListModel::setDefaultAvatarUsername);
	connect(d->m_ui->username, &QLineEdit::textChanged, this, &LoginDialog::updateOkButtonEnabled);
	connect(d->m_ui->password, &QLineEdit::textChanged, this, &LoginDialog::updateOkButtonEnabled);
	connect(d->m_ui->sessionPassword, &QLineEdit::textChanged, this, &LoginDialog::updateOkButtonEnabled);
	connect(d->m_ui->sessionList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &LoginDialog::updateOkButtonEnabled);
	connect(d->m_ui->replaceCert, &QAbstractButton::toggled, this, &LoginDialog::updateOkButtonEnabled);

	connect(d->okButton, &QPushButton::clicked, this, &LoginDialog::onOkClicked);
	connect(d->reportButton, &QPushButton::clicked, this, &LoginDialog::onReportClicked);
	connect(this, &QDialog::rejected, login, &net::LoginHandler::cancelLogin);

	connect(d->m_ui->viewOldCert, &QPushButton::clicked, this, &LoginDialog::showOldCert);
	connect(d->m_ui->viewNewCert, &QPushButton::clicked, this, &LoginDialog::showNewCert);

	d->loginDestructConnection = connect(login, &net::LoginHandler::destroyed, this, &LoginDialog::deleteLater);

	connect(login, &net::LoginHandler::usernameNeeded, this, &LoginDialog::onUsernameNeeded);
	connect(login, &net::LoginHandler::loginNeeded, this, &LoginDialog::onLoginNeeded);
	connect(login, &net::LoginHandler::extAuthNeeded, this, &LoginDialog::onExtAuthNeeded);
	connect(login, &net::LoginHandler::sessionPasswordNeeded, this, &LoginDialog::onSessionPasswordNeeded);
	connect(login, &net::LoginHandler::loginOk, this, &LoginDialog::onLoginOk);
	connect(login, &net::LoginHandler::badLoginPassword, this, &LoginDialog::onBadLoginPassword);
	connect(login, &net::LoginHandler::extAuthComplete, this, &LoginDialog::onExtAuthComplete);
	connect(login, &net::LoginHandler::sessionChoiceNeeded, this, &LoginDialog::onSessionChoiceNeeded);
	connect(login, &net::LoginHandler::certificateCheckNeeded, this, &LoginDialog::onCertificateCheckNeeded);
	connect(login, &net::LoginHandler::serverTitleChanged, this, &LoginDialog::onServerTitleChanged);
}

LoginDialog::~LoginDialog()
{}

void LoginDialog::updateOkButtonEnabled()
{
	bool enabled = false;
	switch(d->mode) {
	case Mode::loading:
	case Mode::catchup:
		break;
	case Mode::identity:
		enabled = UsernameValidator::isValid(d->m_ui->username->text());
		break;
	case Mode::authenticate:
		enabled = !d->m_ui->password->text().isEmpty();
		break;
	case Mode::sessionpassword:
		enabled = !d->m_ui->sessionPassword->text().isEmpty();
		break;
	case Mode::sessionlist: {
		QModelIndexList sel = d->m_ui->sessionList->selectionModel()->selectedIndexes();
		if(sel.isEmpty())
			enabled = false;
		else
			enabled = sel.first().data(net::LoginSessionModel::JoinableRole).toBool();

		d->reportButton->setEnabled(!sel.isEmpty() && d->loginHandler && d->loginHandler->supportsAbuseReports());
		break; }
	case Mode::certChanged:
		enabled = d->m_ui->replaceCert->isChecked();
		break;
	}

	d->okButton->setEnabled(enabled);
}

void LoginDialog::showOldCert()
{
	auto dlg = new CertificateView(d->loginHandler ? d->loginHandler->url().host() : QString(), d->oldCert);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();
}

void LoginDialog::showNewCert()
{
	auto dlg = new CertificateView(d->loginHandler ? d->loginHandler->url().host() : QString(), d->newCert);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->show();
}

void LoginDialog::onUsernameNeeded(bool canSelectAvatar)
{
	QSettings cfg;
	d->m_ui->username->setText(cfg.value("history/username").toString());
	if(canSelectAvatar && d->avatars->rowCount() > 1) {
		d->m_ui->avatarList->show();
		const QString avatar = cfg.value("history/avatar").toString();
		if(avatar.isEmpty())
			d->m_ui->avatarList->setCurrentIndex(d->avatars->index(0));
		else
			d->m_ui->avatarList->setCurrentIndex(d->avatars->getAvatar(avatar));
	} else {
		d->m_ui->avatarList->hide();
	}

	d->resetMode(Mode::identity);
	updateOkButtonEnabled();
}

void LoginDialog::Private::setLoginMode(const QString &prompt)
{
	m_ui->loginPromptLabel->setText(prompt);
	if(extauthurl.isValid())
		m_ui->loginPromptLabel->setStyleSheet(QStringLiteral(
			"background: #3498db;"
			"color: #fcfcfc;"
			"padding: 16px"
			));
	else
		m_ui->loginPromptLabel->setStyleSheet(QStringLiteral(
			"background: #fdbc4b;"
			"color: #31363b;"
			"padding: 16px"
			));

	resetMode(Mode::authenticate);

#ifdef HAVE_QTKEYCHAIN
	if(!loginHandler) {
		qWarning("LoginDialog::setLoginMode: login process already ended!");
		return;
	}

	auto *readJob = new QKeychain::ReadPasswordJob(KEYCHAIN_NAME);
	readJob->setInsecureFallback(QSettings().value("settings/insecurepasswordstorage", false).toBool());
	readJob->setKey(
		keychainSecretName(
			loginHandler->url().userName(),
			extauthurl,
			loginHandler->url().host()
		)
	);

	connect(readJob, &QKeychain::ReadPasswordJob::finished, okButton, [this, readJob]() {
		if(readJob->error() != QKeychain::NoError) {
			qWarning("Keychain error (key=%s): %s",
				qPrintable(readJob->key()),
				qPrintable(readJob->errorString())
			);
			return;
		}

		if(mode != Mode::authenticate) {
			// Unlikely, but...
			qWarning("Keychain returned too late!");
			return;
		}

		const QString password = readJob->textData();
		if(!password.isEmpty()) {
			m_ui->password->setText(password);
			okButton->click();
		}
	});

	readJob->start();
#endif
}

void LoginDialog::onLoginNeeded(const QString &forUsername, const QString &prompt)
{
	if(!forUsername.isEmpty())
		d->m_ui->username->setText(forUsername);

	d->extauthurl = QUrl();
	d->setLoginMode(prompt);
}

void LoginDialog::onExtAuthNeeded(const QString &forUsername, const QUrl &url)
{
	Q_ASSERT(url.isValid());

	if(!forUsername.isEmpty())
		d->m_ui->username->setText(forUsername);

	QString prompt = tr("Log in with %1 credentials").arg("<i>" + url.host() + "</i>");
	if(url.scheme() != "https")
		prompt += " (INSECURE CONNECTION!)";

	d->extauthurl = url;
	d->setLoginMode(prompt);
}

void LoginDialog::onExtAuthComplete(bool success)
{
	if(!success) {
		onBadLoginPassword();
	}

	// If success == true, onLoginOk is called too
}

void LoginDialog::onLoginOk()
{
	if(d->m_ui->rememberPassword->isChecked()) {
#ifdef HAVE_QTKEYCHAIN
		auto *writeJob = new QKeychain::WritePasswordJob(KEYCHAIN_NAME);
		writeJob->setInsecureFallback(QSettings().value("settings/insecurepasswordstorage", false).toBool());
		writeJob->setKey(
			keychainSecretName(
				d->loginHandler->url().userName(),
				d->extauthurl,
				d->loginHandler->url().host()
			)
		);
		writeJob->setTextData(d->m_ui->password->text());
		writeJob->start();
#endif
	}
}

void LoginDialog::onBadLoginPassword()
{
	d->resetMode(Mode::authenticate);
	d->m_ui->password->setText(QString());

#ifdef HAVE_QTKEYCHAIN
	auto *deleteJob = new QKeychain::DeletePasswordJob(KEYCHAIN_NAME);
	deleteJob->setInsecureFallback(QSettings().value("settings/insecurepasswordstorage", false).toBool());
	deleteJob->setKey(
		keychainSecretName(
			d->loginHandler->url().userName(),
			d->extauthurl,
			d->loginHandler->url().host()
		)
	);
	deleteJob->start();
#endif

	d->m_ui->badPasswordLabel->show();
	QTimer::singleShot(2000, d->m_ui->badPasswordLabel, &QLabel::hide);
}

void LoginDialog::onSessionChoiceNeeded(net::LoginSessionModel *sessions)
{
	if(d->m_ui->showNsfw->isEnabled())
		d->m_ui->showNsfw->setChecked(QSettings().value("history/filternsfw").toBool());

	d->sessions->setSourceModel(sessions);

	QHeaderView *header = d->m_ui->sessionList->horizontalHeader();
	header->setSectionResizeMode(net::LoginSessionModel::StatusColumn, QHeaderView::Fixed);
	header->setSectionResizeMode(net::LoginSessionModel::TitleColumn, QHeaderView::Stretch);
	header->setSectionResizeMode(net::LoginSessionModel::FounderColumn, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(net::LoginSessionModel::UserCountColumn, QHeaderView::ResizeToContents);

	auto *sl = d->m_ui->sessionList;
	const auto iconSize = sl->style()->pixelMetric(QStyle::PM_ListViewIconSize, nullptr, sl) + sl->style()->pixelMetric(QStyle::PM_HeaderMargin, nullptr, sl) * 2;
	header->resizeSection(net::LoginSessionModel::StatusColumn, iconSize);

	d->resetMode(Mode::sessionlist);
	updateOkButtonEnabled();
}

void LoginDialog::onSessionPasswordNeeded()
{
	d->m_ui->sessionPassword->setText(QString());
	d->resetMode(Mode::sessionpassword);
}

void LoginDialog::onCertificateCheckNeeded(const QSslCertificate &newCert, const QSslCertificate &oldCert)
{
	d->oldCert = oldCert;
	d->newCert = newCert;
	d->m_ui->replaceCert->setChecked(false);
	d->okButton->setEnabled(false);
	d->resetMode(Mode::certChanged);
}

void LoginDialog::onLoginDone(bool join)
{
	if(join) {
		// Show catchup progress page when joining
		// Login process is now complete and the login handler will
		// self-destruct. But we can keep the login dialog open and show
		// the catchup progress bar until fully caught up or the user
		// manually closes the dialog.
		if(d->loginHandler)
			disconnect(d->loginDestructConnection);
		d->resetMode(Mode::catchup);
	}

}

void LoginDialog::onServerTitleChanged(const QString &title)
{
	d->m_ui->serverTitle->setText(htmlutils::newlineToBr(htmlutils::linkify(title.toHtmlEscaped())));
	d->m_ui->serverTitle->setHidden(title.isEmpty());
}

void LoginDialog::onOkClicked()
{
	if(!d->loginHandler) {
		qWarning("LoginDialog::onOkClicked: login process already ended!");
		return;
	}

	const Mode mode = d->mode;
	d->resetMode(Mode::loading);

	switch(mode) {
	case Mode::loading:
	case Mode::catchup:
		// No OK button in these modes
		qWarning("OK button click in wrong mode!");
		break;
	case Mode::identity: {
		const QPixmap avatar = d->m_ui->avatarList->currentIndex().data(Qt::DecorationRole).value<QPixmap>();
		const QString avatarFile = avatar.isNull() ? QString() : d->m_ui->avatarList->currentIndex().data(AvatarListModel::FilenameRole).toString();

		QSettings cfg;
		cfg.setValue("history/username", d->m_ui->username->text());
		cfg.setValue("history/avatar", avatarFile);

		if(!avatar.isNull())
			d->loginHandler->selectAvatar(avatar.toImage());

		d->loginHandler->selectIdentity(d->m_ui->username->text(), QString());
		break; }
	case Mode::authenticate:
		if(d->extauthurl.isValid()) {
			d->loginHandler->requestExtAuth(d->m_ui->username->text(), d->m_ui->password->text());
		} else {
			d->loginHandler->selectIdentity(d->m_ui->username->text(), d->m_ui->password->text());
		}
		break;
	case Mode::sessionlist: {
		if(d->m_ui->sessionList->selectionModel()->selectedIndexes().isEmpty()) {
			qWarning("Ok clicked but no session selected!");
			return;
		}

		const QModelIndex i = d->m_ui->sessionList->selectionModel()->selectedIndexes().first();
		d->loginHandler->joinSelectedSession(
			i.data(net::LoginSessionModel::AliasOrIdRole).toString(),
			i.data(net::LoginSessionModel::NeedPasswordRole).toBool()
		);
		break;
		}
	case Mode::sessionpassword:
		d->loginHandler->sendSessionPassword(d->m_ui->sessionPassword->text());
		break;
	case Mode::certChanged:
		d->loginHandler->acceptServerCertificate();
		break;
	}
}

void LoginDialog::onReportClicked()
{
	if(d->m_ui->sessionList->selectionModel()->selectedIndexes().isEmpty()) {
		qWarning("Cannot open report dialog: no session selected!");
		return;
	}

	const QModelIndex idx = d->m_ui->sessionList->selectionModel()->selectedIndexes().first();

	AbuseReportDialog *reportDlg = new AbuseReportDialog(this);
	reportDlg->setAttribute(Qt::WA_DeleteOnClose);

	const QString sessionId = idx.data(net::LoginSessionModel::IdRole).toString();
	const QString sessionAlias = idx.data(net::LoginSessionModel::IdAliasRole).toString();
	const QString sessionTitle = idx.data(net::LoginSessionModel::TitleRole).toString();

	reportDlg->setSessionInfo(sessionId, sessionAlias, sessionTitle);

	connect(reportDlg, &AbuseReportDialog::accepted, this, [this, sessionId, reportDlg]() {
		if(!d->loginHandler) {
			qWarning("LoginDialog abuse report: login process already ended!");
			return;
		}
		d->loginHandler->reportSession(sessionId, reportDlg->message());
	});

	reportDlg->show();
}

void LoginDialog::catchupProgress(int value)
{
	d->m_ui->progressBar->setMaximum(100);
	d->m_ui->progressBar->setValue(value);
	if(d->mode == Mode::catchup && value >= 100)
		this->deleteLater();
}

}
