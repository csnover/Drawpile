// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DP_NET_CTRL_H
#define DP_NET_CTRL_H

#include "libshared/net/chat.h"
#include "libshared/net/error.h"
#include "libshared/net/message.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <utility>

namespace protocol {

/**
 * @brief A command sent to the server using the Command message
 */
struct ServerCommand {
	QString cmd;
	QJsonArray args;
	QJsonObject kwargs;

	static ServerCommand fromJson(const QJsonDocument &doc);
	QJsonDocument toJson() const;
};

/**
 * @brief A reply or notification from the server received with the Command message
 */
struct ServerReply {
	enum {
		UNKNOWN,
		LOGIN,   // used during the login phase
		MESSAGE, // general chat type notification message
		ALERT,   // urgent notification message
		ERROR,   // error occurred
		RESULT,  // comand result
		LOG,     // server log message
		SESSIONCONF, // session configuration update
		SIZELIMITWARNING, // session history size nearing limit (deprecated)
		STATUS,  // Periodic status update
		RESET,   // session reset state
		CATCHUP,  // number of messages queued for upload (use for progress bars)
		RESETREQUEST // request client to perform a reset
	} type;
	QString message;
	QJsonObject reply;

	static ServerReply fromJson(const QJsonDocument &doc);
	QJsonDocument toJson() const;
};

/**
 * @brief Server command message
 *
 * This is a general purpose message for sending commands to the server
 * and receiving replies. This is used for (among other things):
 * - the login handshake
 * - setting session parameters (e.g. max user count and password)
 * - sending administration commands (e.g. kick user)
 */
class Command : public Message {
public:
	Command(uint8_t ctx, const QByteArray &msg) : Message(MSG_COMMAND, ctx), m_msg(msg) {}
	Command(uint8_t ctx, const QJsonDocument &doc) : Command(ctx, doc.toJson(QJsonDocument::Compact)) {}
	template<typename T> Command(uint8_t ctx, const T &t) : Command(ctx, t.toJson()) { }

	static Command *deserialize(uint8_t ctxid, const uchar *data, uint len);

	//! Convenience function: make an ERROR type reply message
	static MessagePtr error(Error::Kind kind, const QStringList &&args = {}) {
		return error(Error(kind, std::move(args)));
	}

	//! Convenience function: make an ERROR type reply message
	static MessagePtr error(const Error &message);

	//! Check is message payload is too big to be sent
	bool isOversize() const { return m_msg.length() > 0xffff; }

	QJsonDocument doc() const;
	ServerCommand cmd() const { return ServerCommand::fromJson(doc()); }
	ServerReply reply() const { return ServerReply::fromJson(doc()); }

	QString toString() const override;
	QString messageName() const override { return QStringLiteral("command"); }

protected:
	int payloadLength() const override;
	int serializePayload(uchar *data) const override;
	Kwargs kwargs() const override { return Kwargs(); }

private:
	QByteArray m_msg;
};

/**
 * @brief Disconnect notification
 *
 * This message is used when closing the connection gracefully. The message queue
 * will automatically close the socket after sending this message.
 */
class Disconnect : public Message {
public:
	// These values are sent on the wire so should not be reordered
	enum Reason {
		ERROR,    // client/server error
		KICK,     // user kicked by session operator
		SHUTDOWN, // client/server closed
		OTHER,    // other unspecified error

		_Last
	};

	Disconnect(uint8_t ctx, Reason reason, const QString &message) : Message(MSG_DISCONNECT, ctx),
		_reason(reason), _message(message.toUtf8()) { }

	static Disconnect *deserialize(uint8_t ctx, const uchar *data, uint len);

	/**
	 * Get the reason for the disconnection
	 */
	Reason reason() const { return _reason; }

	/**
	 * Get the disconnect message
	 *
	 * When reason is KICK, this is the name of the operator who kicked this user.
	 */
	QString message() const { return QString::fromUtf8(_message); }

	QString toString() const override;
	QString messageName() const override { return QStringLiteral("disconnect"); }

protected:
	int payloadLength() const override;
	int serializePayload(uchar *data) const override;
	Kwargs kwargs() const override { return Kwargs(); }

private:
	Reason _reason;
	QByteArray _message;
};

/**
 * @brief Ping message
 *
 * This is used for latency measurement as well as a keepalive. Normally, the client
 * should be the one to send the ping messages.
 *
 * The server should return with a Ping with the pong message setenv()
 */
class Ping : public Message {
public:
	Ping(uint8_t ctx, bool pong) : Message(MSG_PING, ctx), m_isPong(pong) { }

	static Ping *deserialize(uint8_t ctx, const uchar *data, int len);

	bool isPong() const { return m_isPong; }

	QString toString() const override;
	QString messageName() const override { return m_isPong ? QStringLiteral("pong") : QStringLiteral("ping"); }

protected:
	int payloadLength() const override;
	int serializePayload(uchar *data) const override;
	Kwargs kwargs() const override { return Kwargs(); }

private:
	bool m_isPong;
};

class DisconnectExt : public Message {
	Q_DECLARE_TR_FUNCTIONS(protocol::DisconnectExt)
public:
	using Reason = Disconnect::Reason;

	DisconnectExt(const ChatActor &actor, uint8_t ctx = 0)
		: Message(MSG_DISCONNECT_EXT, ctx)
		, m_reason(Reason::KICK)
		, m_actor(actor) {}

	DisconnectExt(const ChatActor::Kind &actor, uint8_t ctx = 0)
		: Message(MSG_DISCONNECT_EXT, ctx)
		, m_reason(Reason::KICK)
		, m_actor(actor) {}

	DisconnectExt(const Error &error, uint8_t ctx = 0)
		: Message(MSG_DISCONNECT_EXT, ctx)
		, m_reason(Reason::ERROR)
		, m_error(error) {}

	DisconnectExt(const Error::Kind &error, uint8_t ctx = 0)
		: Message(MSG_DISCONNECT_EXT, ctx)
		, m_reason(Reason::ERROR)
		, m_error(error) {}

	DisconnectExt(const Shutdown &shutdown, uint8_t ctx = 0)
		: Message(MSG_DISCONNECT_EXT, ctx)
		, m_reason(Reason::SHUTDOWN)
		, m_shutdown(shutdown) {}

	DisconnectExt(const Shutdown::Kind &shutdown, uint8_t ctx = 0)
		: Message(MSG_DISCONNECT_EXT, ctx)
		, m_reason(Reason::SHUTDOWN)
		, m_shutdown(shutdown) {}

	DisconnectExt(const QString &message, uint8_t ctx = 0)
		: Message(MSG_DISCONNECT_EXT, ctx)
		, m_reason(Reason::OTHER)
		, m_raw(message) {}

	DisconnectExt(const DisconnectExt &other)
		: Message(other)
		, m_reason(other.m_reason)
	{
		switch (m_reason) {
		case Reason::KICK: new (&m_actor) auto(other.m_actor); break;
		case Reason::ERROR: new (&m_error) auto(other.m_error); break;
		case Reason::SHUTDOWN: new (&m_shutdown) auto(other.m_shutdown); break;
		case Reason::OTHER: new (&m_raw) auto(other.m_raw); break;
		case Reason::_Last: Q_UNREACHABLE(); break;
		}
	}

	DisconnectExt(DisconnectExt &&other)
		: Message(other)
		, m_reason(other.m_reason)
	{
		switch (m_reason) {
		case Reason::KICK: new (&m_actor) auto(std::move(other.m_actor)); break;
		case Reason::ERROR: new (&m_error) auto(std::move(other.m_error)); break;
		case Reason::SHUTDOWN: new (&m_shutdown) auto(std::move(other.m_shutdown)); break;
		case Reason::OTHER: new (&m_raw) auto(std::move(other.m_raw)); break;
		case Reason::_Last: Q_UNREACHABLE(); break;
		}
	}

	~DisconnectExt()
	{
		switch(m_reason) {
		case Reason::KICK: m_actor.~ChatActor(); break;
		case Reason::ERROR: m_error.~Error(); break;
		case Reason::SHUTDOWN: m_shutdown.~Shutdown(); break;
		case Reason::OTHER: m_raw.~QString(); break;
		case Reason::_Last: Q_UNREACHABLE();
		}
	}

	Disconnect *toDisconnect() const
	{
		return new Disconnect(contextId(), m_reason, message());
	}

	static DisconnectExt *deserialize(uint8_t ctx, const uchar *data, uint len);

	Reason reason() const { return m_reason; }

	QString message() const
	{
		switch (m_reason) {
		case Reason::KICK:
			return tr("Kicked by %1").arg(m_actor.name());
		case Reason::ERROR:
			return tr("Server error: %1").arg(m_error.message());
		case Reason::SHUTDOWN:
			return tr("Closing session: %1").arg(m_shutdown.message());
		case Reason::OTHER:
			return tr("Unknown error: %1").arg(m_raw);
		case Reason::_Last: {}
		}

		Q_UNREACHABLE();
	}

	QJsonObject toJson() const
	{
		QJsonObject detail;
		switch(m_reason) {
		case Reason::KICK: detail = m_actor.toJson(); break;
		case Reason::ERROR: detail = m_error.toJson(); break;
		case Reason::SHUTDOWN: detail = m_shutdown.toJson(); break;
		case Reason::OTHER: break;
		case Reason::_Last: Q_UNREACHABLE(); break;
		}

		return {{
			{ "type", m_reason },
			// Message included for forward-compatibility
			{ "message", message() },
			{ "detail", detail }
		}};
	}

	QString toString() const override;
	QString messageName() const override { return QStringLiteral("disconnectext"); }

protected:
	int payloadLength() const override;
	int serializePayload(uchar *data) const override;
	Kwargs kwargs() const override { return Kwargs(); }

private:
	Reason m_reason;
	union {
		ChatActor m_actor;
		Error m_error;
		Shutdown m_shutdown;
		QString m_raw;
	};
};

}

#endif
