// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef NETSTATUS_H
#define NETSTATUS_H

#include "desktop/utils/dynamicui.h"
#include "libclient/net/server.h"

#include <QWidget>
#include <QUrl>
#include <QPointer>
#include <QSslCertificate>

class QLabel;
class QTimer;
class QProgressBar;

namespace dialogs {
	class NetStats;
}

namespace widgets {

class PopupMessage;

/**
 * @brief Network connection status widget
 * This widget displays the current status of the connection with the server
 * and the address of the host.
 */
class NetStatus : public QWidget
{
Q_OBJECT
public:
	NetStatus(QWidget *parent);

	void setSecurityLevel(net::Server::Security level, const QSslCertificate &certificate);

public slots:
	void connectingToHost(const QString& address, int port);
	void loggedIn(const QUrl &sessionUrl);
	void hostDisconnecting();
	void hostDisconnected();

	void setRoomcode(const QString &roomcode);

	void bytesReceived(int count);
	void bytesSent(int count);

	void lagMeasured(qint64 lag);

	//! Update the download progress bar with message catchup progress (0-100)
	void setCatchupProgress(int progress);

	//! Update the download progress bar with file download progress (0-total)
	void setDownloadProgress(qint64 received, qint64 total);

	//! Download over, hide the progress bar
	void hideDownloadProgress();

	//! This user was kicked off the session
	void kicked(const QString& user);

	void copyAddress();
	void copyUrl();

private slots:
	void discoverAddress();
	void externalIpDiscovered(const QString &ip);
	void showCertificate();
	void showNetStats();

private:
	void showCGNAlert();
	void message(const QString &msg);
	void updateLabel();

	enum { NotConnected, Connecting, LoggedIn, Disconnecting } m_state;

	QString fullAddress() const;

	QPointer<dialogs::NetStats> _netstats;
	QProgressBar *m_download;

	QLabel *m_label, *m_security;
	PopupMessage *m_popup;
	QString m_address;
	QString m_roomcode;
	int m_port;
	QUrl m_sessionUrl;

	bool m_hideServer;

	QAction *_copyaction;
	QAction *_urlaction;
	QAction *_discoverIp;

	quint64 _sentbytes, _recvbytes, _lag;

	QScopedPointer<const QSslCertificate> m_certificate;
	Translator<net::Server::Security> m_securityState;
	Translator<> m_labelText;
};

}

#endif
