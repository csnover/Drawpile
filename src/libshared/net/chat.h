// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Drawpile contributors

#ifndef LIBSHARED_NET_CHAT_H
#define LIBSHARED_NET_CHAT_H

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QtGlobal>

class QJsonObject;

namespace protocol {

class ChatActor {
	Q_DECLARE_TR_FUNCTIONS(protocol::ChatActor)
public:
	// These values are sent on the wire so should not be reordered
	enum Kind {
		Unknown,
		User,
		Server,
		ServerAdmin,
		Password,
		_Last
	};

	ChatActor(Kind kind)
		: m_kind(kind)
		, m_name(QString()) {}

	ChatActor(const QString &name)
		: m_kind(Kind::User)
		, m_name(name) {}

	ChatActor(Kind kind, const QString &name)
		: m_kind(kind)
		, m_name(name) {}

	static ChatActor fromJson(const QJsonObject &data);

	Kind kind() const { return m_kind; }

	QString name() const
	{
		switch(m_kind) {
		case Unknown: return tr("unknown");
		case User: return m_name;
		case Server: return tr("the server");
		case ServerAdmin: return tr("the server admin");
		case Password: return tr("password");
		case _Last: {}
		}

		Q_ASSERT(false);
		return QString();
	}

	QJsonObject toJson() const;

private:
	Kind m_kind;
	QString m_name;
};

class SystemChat {
	Q_DECLARE_TR_FUNCTIONS(protocol::SystemChat)
public:
	// These values are sent on the wire so should not be reordered
	enum Kind {
		Unknown,
		ResetFailed,
		PreparingReset,
		ResetCancelled,
		UserBanned,
		UserKicked,
		SessionShutDown,
		OpAdded,
		OpRemoved,
		TrustedAdded,
		TrustedRemoved,
		HistoryLimit,
		_Last
	};

	SystemChat(Kind kind, ChatActor actor = ChatActor::User, const QStringList &&args = {})
		: m_kind(kind)
		, m_actor(actor)
		, m_args(args)
	{}

	static SystemChat fromJson(const QJsonObject &data, const QString &fallback);

	QString message() const
	{
		switch (m_kind) {
		case Unknown: if (m_args.isEmpty()) {
			return tr("Unknown message received from server");
		} else {
			return m_args.at(0);
		}
		case ResetFailed: return tr("Session reset failed!");
		case ResetCancelled: return tr("Session reset cancelled.");
		case PreparingReset: return tr("Preparing for session reset!");
		case UserBanned: return tr("%1 banned by %2")
			.arg(m_args.at(0))
			.arg(m_actor.name());
		case UserKicked: return tr("%1 kicked by %2")
			.arg(m_args.at(0))
			.arg(m_actor.name());
		case SessionShutDown: return tr("Session shut down by moderator (%1)")
			.arg(m_actor.name());
		case TrustedAdded: return tr("%1 trusted by %2")
			.arg(m_args.at(0))
			.arg(m_actor.name());
		case TrustedRemoved: return tr("%1 untrusted by %2")
			.arg(m_args.at(0))
			.arg(m_actor.name());
		case OpAdded: return tr("%1 made operator by %2")
			.arg(m_args.at(0))
			.arg(m_actor.name());
		case OpRemoved: return tr("%1 deopped by %2")
			.arg(m_args.at(0))
			.arg(m_actor.name());
		case HistoryLimit: return tr("History size limit reached! Session must be reset to continue.");
		case _Last: {}
		}

		Q_ASSERT(false);
		return QString();
	}

	Kind kind() const { return m_kind; }
	const ChatActor &actor() const { return m_actor; }
	const QStringList &args() const { return m_args; }

	QJsonObject toJson() const;

private:
	Kind m_kind;
	ChatActor m_actor;
	QStringList m_args;
};

}

#endif
