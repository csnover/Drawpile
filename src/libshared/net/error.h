// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Drawpile contributors

#ifndef LIBSHARED_NET_ERROR_H
#define LIBSHARED_NET_ERROR_H

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QtGlobal>

class QJsonObject;

namespace protocol {

class Shutdown {
	Q_DECLARE_TR_FUNCTIONS(protocol::Shutdown)
public:
	enum Kind {
		Unknown,
		Logout,
		Session,
		Server,

		_Last
	};

	Shutdown(Kind kind) : m_kind(kind) {}

	static Shutdown fromJson(const QJsonObject &data);

	Kind kind() const { return m_kind; }

	QString message() const
	{
		switch(m_kind) {
		case Unknown: return tr("Unknown");
		case Logout: return tr("Logged out");
		case Session: return tr("Session terminated");
		case Server: return tr("Server shutting down");
		case _Last: {}
		}

		Q_UNREACHABLE();
	}

	QJsonObject toJson() const;

private:
	Kind m_kind;
};

class Error {
	Q_DECLARE_TR_FUNCTIONS(protocol::Error)

public:
	// These values are sent on the wire so should not be reordered
	enum Kind {
		Unknown,

		// Login
		BadPassword,
		BadUserId,
		BadUserIdOrName,
		UserNotFound,
		MissingUserIdOrName,
		ExpectedUsernamePassword,
		BadUsername,
		CannotExtAuthInternal,
		ExtAuthNotRequested,
		ExtAuthBadSignature,
		ExtAuthInvalidToken,
		BannedName,
		ExtAuth,
		NoExtAuth,
		ExtAuthOutgroup,
		ExtAuthBadResponse,
		NoGuest,
		UnauthorizedHost,
		BadAlias,
		NotFound,
		Banned,
		Closed,
		AuthOnly,
		NameInUse,
		NoTls,
		AlreadySecure,
		TlsRequired,
		UnparseableProtocol,
		InvalidUserId,
		ExpectedSessionId,
		IdInUse,
		BadProtocol,
		ServerFull,

		// Actions
		MissingOpword,
		EmptyOpword,
		CannotBanUser,
		CannotKickSelf,
		CannotKickModerators,
		CannotKickTrusted,
		MissingBanEntryId,
		MissingApiUrl,
		InvalidApiUrl,
		CannotReset,
		MissingUserIdBool,
		NotModerator,
		NotSessionOwner,
		NotSessionOwnerOrDeputy,
		UnknownCommand,
		InvalidMessage,
		DeoppedWhileResetting,
		HistoryLimitExceeded,

		_Last
	};

	Error(Kind kind, const QStringList &&args = {})
		: m_kind(kind)
		, m_args(args) {}

	static Error fromJson(const QJsonObject &data, const QString &fallback);

	// This is for backwards-compatibility with the existing wire protocol
	static Error fromCode(const QString &code)
	{
		if (code == "tlsRequired") return TlsRequired;
		if (code == "badUsername") return BadUsername;
		if (code == "badPassword") return BadPassword;
		if (code == "bannedName") return BannedName;
		if (code == "noExtAuth") return NoExtAuth;
		if (code == "extauthOutgroup") return ExtAuthOutgroup;
		if (code == "noGuest") return NoGuest;
		if (code == "unauthorizedHost") return UnauthorizedHost;
		if (code == "badAlias") return BadAlias;
		if (code == "notFound") return NotFound;
		if (code == "banned") return Banned;
		if (code == "closed") return Closed;
		if (code == "authOnly") return AuthOnly;
		if (code == "nameInUse") return NameInUse;
		if (code == "noTls") return NoTls;
		if (code == "alreadySecure") return AlreadySecure;
		if (code == "badProtocol") return BadProtocol;
		if (code == "idInUse") return IdInUse;
		return Unknown;
	}

	Kind kind() const { return m_kind; }

	const QStringList &args() const { return m_args; }

	// This is for backwards-compatibility with the existing wire protocol
	const char *code() const {
		switch(m_kind) {
		case TlsRequired: return "tlsRequired";
		case UnparseableProtocol:
		case InvalidUserId:
		case ExpectedSessionId:
		case ExpectedUsernamePassword: return "syntax";
		case BadUsername: return "badUsername";
		case CannotExtAuthInternal:
		case ExtAuthBadSignature:
		case ExtAuthNotRequested:
		case ExtAuthInvalidToken: return "extAuthError";
		case BadPassword: return "badPassword";
		case BannedName: return "bannedName";
		case ExtAuthBadResponse:
		case ExtAuth: return "extauth";
		case NoExtAuth: return "noExtAuth";
		case ExtAuthOutgroup: return "extauthOutgroup";
		case NoGuest: return "noGuest";
		case UnauthorizedHost: return "unauthorizedHost";
		case BadAlias: return "badAlias";
		case NotFound: return "notFound";
		case Banned: return "banned";
		case ServerFull:
		case Closed: return "closed";
		case AuthOnly: return "authOnly";
		case NameInUse: return "nameInUse";
		case NoTls: return "noTls";
		case AlreadySecure: return "alreadySecure";
		case BadProtocol: return "badProtocol";
		case IdInUse: return "idInUse";
		default: return nullptr;
		}
	}

	QString message() const {
		switch(m_kind) {
		case Unknown: if (m_args.isEmpty()) {
			return tr("Unknown error");
		} else {
			return m_args.at(0);
		}
		case MissingOpword: return tr("Expected one argument: opword");
		case EmptyOpword: return tr("No opword set");
		case BadPassword: return tr("Incorrect password");
		case BadUserId: return tr("invalid user id: %1").arg(m_args.at(0));
		case BadUserIdOrName: return tr("invalid user ID or name");
		case UserNotFound: return tr("user not found");
		case MissingUserIdOrName: return tr("Expected one argument: user ID or name");
		case CannotBanUser: return tr("%1 cannot be banned.").arg(m_args.at(0));
		case CannotKickSelf: return tr("cannot kick self");
		case CannotKickModerators: return tr("cannot kick moderators");
		case CannotKickTrusted: return tr("cannot kick trusted users");
		case MissingBanEntryId: return tr("Expected one argument: ban entry ID");
		case MissingApiUrl: return tr("Expected one argument: API URL");
		case InvalidApiUrl: return tr("Invalid API URL");
		case CannotReset: return tr("Unable to reset in this state");
		case MissingUserIdBool: return tr("Expected two arguments: userId true/false");
		case NotModerator: return tr("%1: Not a moderator").arg(m_args.at(0));
		case NotSessionOwner: return tr("%1: Not a session owner").arg(m_args.at(0));
		case NotSessionOwnerOrDeputy: return tr("%1: Not a session owner or deputy").arg(m_args.at(0));
		case UnknownCommand: return tr("Unknown command: %1").arg(m_args.at(0));
		case TlsRequired: return tr("TLS required");
		case ExpectedUsernamePassword: return tr("Expected username and (optional) password");
		case BadUsername: return tr("Invalid username");
		case CannotExtAuthInternal: return tr("Cannot use extauth with an internal user account!");
		case ExtAuthNotRequested: return tr("Ext auth not requested!");
		case ExtAuthBadSignature: return tr("Ext auth token signature mismatch!");
		case ExtAuthInvalidToken: return tr("Ext auth token is invalid!");
		case BannedName: return tr("This username is banned");
		case ExtAuth: return tr("Received auth server reply in unexpected state");
		case NoExtAuth: return tr("Authentication server is unavailable!");
		case ExtAuthOutgroup: return tr("This username cannot log in to this server");
		case ExtAuthBadResponse: return tr("Unexpected ext-auth response: %1").arg(m_args.at(0));
		case NoGuest: return tr("Guest logins not allowed");
		case UnauthorizedHost: return tr("Hosting not authorized");
		case UnparseableProtocol: return tr("Unparseable protocol version");
		case InvalidUserId: return tr("Invalid user ID (must be in range 1-254)");
		case BadAlias: return tr("Invalid session alias");
		case ExpectedSessionId: return tr("Expected session ID");
		case NotFound: return tr("Session not found!");
		case Banned: return tr("You have been banned from this session");
		case Closed: return tr("This session is closed");
		case AuthOnly: return tr("This session does not allow guest logins");
		case NameInUse: return tr("This username is already in use");
		case NoTls: return tr("TLS not supported");
		case AlreadySecure: return tr("Connection already secured");
		case IdInUse: return tr("An internal server error occurred.");
		case BadProtocol: return tr("This server does not support this protocol version.");
		case ServerFull: return tr("This server is full.");
		case InvalidMessage: return tr("invalid message");
		case DeoppedWhileResetting: return tr("De-opped while resetting");
		case HistoryLimitExceeded: return tr("History limit exceeded");
		case _Last: {}
		}

		Q_UNREACHABLE();
	}

	QJsonObject toJson() const;

private:
	Kind m_kind;
	QStringList m_args;
};

} // namespace error

#endif
