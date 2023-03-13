// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libserver/opcommands.h"
#include "libserver/client.h"
#include "libserver/session.h"
#include "libserver/serverlog.h"
#include "libshared/net/chat.h"
#include "libshared/net/control.h"
#include "libshared/net/error.h"
#include "libshared/net/meta.h"
#include "libshared/util/passwordhash.h"

#include <QList>
#include <QStringList>
#include <QUrl>

namespace server {

using protocol::Error;
using protocol::SystemChat;

namespace {

typedef void (*SrvCommandFn)(Client *, const QJsonArray &, const QJsonObject &);

class SrvCommand {
public:
	enum Mode {
		NONOP,  // usable by all
		DEPUTY, // needs at least deputy privileges
		OP,     // needs operator privileges
		MOD     // needs moderator privileges
	};

	SrvCommand(const QString &name, SrvCommandFn fn, Mode mode=OP)
		: m_fn(fn), m_name(name), m_mode(mode)
	{}

	void call(Client *c, const QJsonArray &args, const QJsonObject &kwargs) const { m_fn(c, args, kwargs); }
	const QString &name() const { return m_name; }

	Mode mode() const { return m_mode; }

private:
	SrvCommandFn m_fn;
	QString m_name;
	Mode m_mode;
};

struct SrvCommandSet {
	QList<SrvCommand> commands;

	SrvCommandSet();
};

const SrvCommandSet COMMANDS;

void readyToAutoReset(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(args);
	Q_UNUSED(kwargs);
	client->session()->readyToAutoReset(client->id());
}

void initBegin(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(args);
	Q_UNUSED(kwargs);
	client->session()->handleInitBegin(client->id());
}

void initComplete(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(args);
	Q_UNUSED(kwargs);
	client->session()->handleInitComplete(client->id());
}

void initCancel(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(args);
	Q_UNUSED(kwargs);
	client->session()->handleInitCancel(client->id());
}

void sessionConf(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(args);
	client->session()->setSessionConfig(kwargs, client);
}

void opWord(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(kwargs);
	if(args.size() != 1)
		throw Error(Error::MissingOpword);

	const QByteArray opwordHash = client->session()->history()->opwordHash();
	if(opwordHash.isEmpty())
		throw Error(Error::EmptyOpword);

	if(passwordhash::check(args.at(0).toString(), opwordHash)) {
		client->session()->changeOpStatus(client->id(), true, protocol::ChatActor::Password);

	} else {
		throw Error(Error::BadPassword);
	}
}

Client *_getClient(Session *session, const QJsonValue &idOrName)
{
	Client *c = nullptr;
	if(idOrName.isDouble()) {
		// ID number
		const int id = idOrName.toInt();
		if(id<1 || id > 254)
			throw Error(Error::BadUserId, { QString::number(id) });
		c = session->getClientById(id);

	} else if(idOrName.isString()){
		// Username
		c = session->getClientByUsername(idOrName.toString());
	} else {
		throw Error(Error::BadUserIdOrName);
	}
	if(!c)
		throw Error(Error::UserNotFound);

	return c;
}

void kickUser(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	if(args.size()!=1)
		throw Error(Error::MissingUserIdOrName);

	const bool ban = kwargs["ban"].toBool();

	if(ban && client->session()->hasPastClientWithId(args.at(0).toInt())) {
		// Retroactive ban
		const auto target = client->session()->getPastClientById(args.at(0).toInt());
		if(target.isBannable) {
			client->session()->addBan(target, client->username());
			client->session()->sysToAll(SystemChat(
				SystemChat::UserBanned,
				client->username(),
				{ target.username }),
				false
			);
		} else {
			throw Error(Error::CannotBanUser, { target.username });
		}
		return;
	}

	Client *target = _getClient(client->session(), args.at(0));
	if(target == client)
		throw Error(Error::CannotKickSelf);

	if(target->isModerator())
		throw Error(Error::CannotKickModerators);

	if(client->isDeputy()) {
		if(target->isOperator() || target->isTrusted())
			throw Error(Error::CannotKickTrusted);
	}

	if(ban) {
		client->session()->addBan(target, client->username());
	}
	client->session()->sysToAll(SystemChat(ban
		? SystemChat::UserBanned
		: SystemChat::UserKicked,
		client->username(),
		{ target->username() }),
		false
	);

	target->disconnectClient(protocol::ChatActor(client->username()));
}

void removeBan(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(kwargs);
	if(args.size()!=1)
		throw Error(Error::MissingBanEntryId);

	client->session()->removeBan(args.at(0).toInt(), client->username());
}

void killSession(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(args);
	Q_UNUSED(kwargs);

	client->session()->sysToAll(SystemChat(
		SystemChat::SessionShutDown,
		client->username()),
		true
	);
	client->session()->killSession();
}

void announceSession(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	if(args.size()!=1)
		throw Error(Error::MissingApiUrl);

	QUrl apiUrl { args.at(0).toString() };
	if(!apiUrl.isValid())
		throw Error(Error::InvalidApiUrl);

	client->session()->makeAnnouncement(apiUrl, kwargs["private"].toBool());
}

void unlistSession(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(kwargs);
	if(args.size() != 1)
		throw Error(Error::MissingApiUrl);

	client->session()->unlistAnnouncement(args.at(0).toString());
}

void resetSession(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(args);
	Q_UNUSED(kwargs);

	if(client->session()->state() != Session::State::Running)
		throw Error(Error::CannotReset);

	client->session()->resetSession(client->id());
}

void setMute(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(kwargs);

	if(args.size() != 2)
		throw Error(Error::MissingUserIdBool);

	Client *c = _getClient(client->session(), args.at(0));

	const bool m = args.at(1).toBool();
	if(c->isMuted() != m) {
		c->setMuted(m);
		client->session()->sendUpdatedMuteList();
		if(m)
			c->log(Log().about(Log::Level::Info, Log::Topic::Mute).message(Client::tr("Muted by %1").arg(client->username())));
		else
			c->log(Log().about(Log::Level::Info, Log::Topic::Unmute).message(Client::tr("Unmuted by %1").arg(client->username())));
	}
}

void reportAbuse(Client *client, const QJsonArray &args, const QJsonObject &kwargs)
{
	Q_UNUSED(args);

	const int user = kwargs["user"].toInt();
	const QString reason = kwargs["reason"].toString();

	client->session()->sendAbuseReport(client, user, reason);
}

SrvCommandSet::SrvCommandSet()
{
	commands
		<< SrvCommand("ready-to-autoreset", readyToAutoReset)
		<< SrvCommand("init-begin", initBegin)
		<< SrvCommand("init-complete", initComplete)
		<< SrvCommand("init-cancel", initCancel)
		<< SrvCommand("sessionconf", sessionConf)
		<< SrvCommand("kick-user", kickUser, SrvCommand::DEPUTY)
		<< SrvCommand("gain-op", opWord, SrvCommand::NONOP)

		<< SrvCommand("reset-session", resetSession)
		<< SrvCommand("kill-session", killSession, SrvCommand::MOD)

		<< SrvCommand("announce-session", announceSession)
		<< SrvCommand("unlist-session", unlistSession)

		<< SrvCommand("remove-ban", removeBan)
		<< SrvCommand("mute", setMute)

		<< SrvCommand("report", reportAbuse, SrvCommand::NONOP)
	;
}

} // end of anonymous namespace

void handleClientServerCommand(Client *client, const QString &command, const QJsonArray &args, const QJsonObject &kwargs)
{
	for(const SrvCommand &c : COMMANDS.commands) {
		if(c.name() == command) {
			if(c.mode() == SrvCommand::MOD && !client->isModerator()) {
				client->sendDirectMessage(protocol::Command::error(Error::NotModerator, { command }));
				return;
			}
			else if(c.mode() == SrvCommand::OP && !client->isOperator()) {
				client->sendDirectMessage(protocol::Command::error(Error::NotSessionOwner, { command }));
				return;
			}
			else if(c.mode() == SrvCommand::DEPUTY && !client->isOperator() && !client->isDeputy()) {
				client->sendDirectMessage(protocol::Command::error(Error::NotSessionOwnerOrDeputy, { command }));
				return;
			}

			try {
				c.call(client, args, kwargs);
			} catch(const Error &err) {
				client->sendDirectMessage(protocol::Command::error(err));
			}
			return;
		}
	}

	client->sendDirectMessage(
		protocol::Command::error(Error::UnknownCommand, { command })
	);
}

}
