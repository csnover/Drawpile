// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DP_NET_SERVER_H
#define DP_NET_SERVER_H

#include "libclient/net/envelope.h"

#include <QObject>

class QSslCertificate;

namespace net {

/**
 * \brief Abstract base class for servers interfaces
 */
class Server : public QObject {
	Q_OBJECT
public:
	enum Security {
		NO_SECURITY, // No secure connection
		NEW_HOST,    // Secure connection to a host we haven't seen before
		KNOWN_HOST,  // Secure connection whose certificate we have seen before
		TRUSTED_HOST // A host we have explicitly marked as trusted
	};

	Server(QObject *parent);

	/**
	 * \brief Send a message to the server
	 */
	virtual void sendEnvelope(const Envelope &e) = 0;

	/**
	 * @brief Log out from the server
	 */
	virtual void logout() = 0;

	/**
	 * @brief Is the user in a session
	 */
	virtual bool isLoggedIn() const = 0;

	/**
	 * @brief Return the number of bytes in the upload buffer
	 */
	virtual int uploadQueueBytes() const = 0;

	/**
	 * @brief Current security level
	 */
	virtual Security securityLevel() const = 0;

	/**
	 * @brief Get the server's SSL certificate (if any)
	 */
	virtual QSslCertificate hostCertificate() const = 0;

	/**
	 * @brief Does the server support persistent sessions?
	 */
	virtual bool supportsPersistence() const = 0;
	virtual bool supportsAbuseReports() const = 0;

signals:
	void envelopeReceived(const Envelope &envelope);
};

}

#endif
