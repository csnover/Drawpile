// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DP_NET_TCPSERVER_H
#define DP_NET_TCPSERVER_H

#include "libclient/net/server.h"
#include "libclient/net/messagequeue.h"

#include <QUrl>

class QSslSocket;

namespace net {

class LoginHandler;

class TcpServer : public Server
{
	Q_OBJECT
	friend class LoginHandler;
public:
	explicit TcpServer(QObject *parent=nullptr);

	void login(LoginHandler *login);
	void logout() override;

	void sendEnvelope(const Envelope &e) override;

	bool isLoggedIn() const override { return m_loginstate == nullptr; }

	int uploadQueueBytes() const override;

	void startTls();

	Security securityLevel() const override { return m_securityLevel; }
	QSslCertificate hostCertificate() const override;

	bool supportsPersistence() const override { return m_supportsPersistence; }
	bool supportsAbuseReports() const override { return m_supportsAbuseReports; }

signals:
	void loggedIn(const QUrl &url, uint8_t userid, bool join, bool auth, bool moderator, bool hasAutoreset);
	void loggingOut();
	void gracefullyDisconnecting(protocol::DisconnectExt::Reason reason, const QString &message);
	void serverDisconnected(const QString &message, const QString &errorcode, bool localDisconnect);

	void bytesReceived(int);
	void bytesSent(int);

	void lagMeasured(qint64 lag);

protected:
	void loginFailure(const QString &message, const QString &errorcode);
	void loginSuccess();

private slots:
	void handleMessage();
	void handleBadData(int len, int type, int contextId);
	void handleDisconnect();
	void handleSocketError();

private:
	QSslSocket *m_socket;
	MessageQueue *m_msgqueue;
	LoginHandler *m_loginstate;
	QString m_error, m_errorcode;
	Security m_securityLevel;
	bool m_localDisconnect;
	bool m_supportsPersistence;
	bool m_supportsAbuseReports;
};

}

#endif // TCPSERVER_H
