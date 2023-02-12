// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

namespace net {

class Envelope;

/**
 * @brief A command sent to the server using the (Control) Command message
 */
struct ServerCommand {
	QString cmd;
	QJsonArray args;
	QJsonObject kwargs;

	Envelope toEnvelope() const;

	// Convenience functions_
	static Envelope make(const QString &cmd, const QJsonArray &args=QJsonArray(), const QJsonObject &kwargs=QJsonObject());

	//! Kick (and optionally ban) a user from the session
	static Envelope makeKick(int target, bool ban);

	//! Remove a ban entry
	static Envelope makeUnban(int entryId);

	//! (Un)mute a user
	static Envelope makeMute(int target, bool mute);

	//! Request the server to announce this session at a listing server
	static Envelope makeAnnounce(const QString &url, bool privateMode);

	//! Request the server to remove an announcement at the listing server
	static Envelope makeUnannounce(const QString &url);
};

/**
 * @brief A reply or notification from the server received with the Command message
 */
struct ServerReply {
	enum class ReplyType {
		Unknown,
		Login,   // used during the login phase
		Message, // general chat type notifcation message
		Alert,   // urgen notification message
		Error,   // error occurred
		Result,  // comand result
		Log,     // server log message
		SessionConf, // session configuration update
		SizeLimitWarning, // session history size nearing limit (deprecated)
		Status,  // Periodic status update
		Reset,   // session reset state
		Catchup,  // number of messages queued for upload (use for progress bars)
		ResetRequest, // request client to perform a reset
	} type;
	QString message;
	QJsonObject reply;

	static ServerReply fromEnvelope(const Envelope &e);
	static ServerReply fromJson(const QJsonDocument &doc);
	QJsonDocument toJson() const;
};

}
