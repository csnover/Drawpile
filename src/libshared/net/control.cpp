// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libshared/net/chat.h"
#include "libshared/net/control.h"
#include "libshared/net/error.h"

#include <cstring>

#include <QtEndian>
#include <QDebug>

namespace protocol {

ServerCommand ServerCommand::fromJson(const QJsonDocument &doc)
{
	const QJsonObject o = doc.object();
	ServerCommand c;
	c.cmd = o.value("cmd").toString();
	c.args = o.value("args").toArray();
	c.kwargs = o.value("kwargs").toObject();
	return c;
}

QJsonDocument ServerCommand::toJson() const
{
	QJsonObject o;
	o["cmd"] = cmd;
	if(!args.isEmpty())
		o["args"] = args;
	if(!kwargs.isEmpty())
		o["kwargs"] = kwargs;
	return QJsonDocument(o);
}

ServerReply ServerReply::fromJson(const QJsonDocument &doc)
{
	const QJsonObject o = doc.object();
	ServerReply r;
	QString typestr = o.value("type").toString();

	if(typestr == "login")
		r.type = LOGIN;
	else if(typestr == "msg")
		r.type = MESSAGE;
	else if(typestr == "alert")
		r.type = ALERT;
	else if(typestr == "error")
		r.type = ERROR;
	else if(typestr == "result")
		r.type = RESULT;
	else if(typestr == "log")
		r.type = LOG;
	else if(typestr == "sessionconf")
		r.type = SESSIONCONF;
	else if(typestr == "sizelimit")
		r.type = SIZELIMITWARNING;
	else if(typestr == "status")
		r.type = STATUS;
	else if(typestr == "reset")
		r.type = RESET;
	else if(typestr == "autoreset")
		r.type = RESETREQUEST;
	else if(typestr == "catchup")
		r.type = CATCHUP;
	else
		r.type = UNKNOWN;

	r.message = o.value("message").toString();
	r.reply = o;
	return r;
}

QJsonDocument ServerReply::toJson() const
{
	QJsonObject o = reply;
	QString typestr;
	switch(type) {
	case UNKNOWN: break;
	case LOGIN: typestr=QStringLiteral("login"); break;
	case MESSAGE: typestr=QStringLiteral("msg"); break;
	case ALERT: typestr=QStringLiteral("alert"); break;
	case ERROR: typestr=QStringLiteral("error"); break;
	case RESULT: typestr=QStringLiteral("result"); break;
	case LOG: typestr=QStringLiteral("log"); break;
	case SESSIONCONF: typestr=QStringLiteral("sessionconf"); break;
	case SIZELIMITWARNING: typestr=QStringLiteral("sizelimit"); break;
	case STATUS: typestr=QStringLiteral("status"); break;
	case RESET: typestr=QStringLiteral("reset"); break;
	case CATCHUP: typestr=QStringLiteral("catchup"); break;
	case RESETREQUEST: typestr=QStringLiteral("autoreset"); break;
	}
	o["type"] = typestr;

	if(!o.contains("message"))
		o["message"] = message;
	return QJsonDocument(o);
}

MessagePtr Command::error(const Error &error)
{
	ServerReply sr;
	sr.type = ServerReply::ERROR;
	sr.message = error.message();
	sr.reply = {{
		{ "kind", int(error.kind()) },
		{ "args", QJsonArray::fromStringList(error.args()) }
	}};
	return MessagePtr(new Command(0, sr));
}

Command *Command::deserialize(uint8_t ctxid, const uchar *data, uint len)
{
	return new Command(ctxid, QByteArray(reinterpret_cast<const char*>(data), len));
}

int Command::serializePayload(uchar *data) const
{
	const int len = payloadLength();
	memcpy(data, m_msg.constData(), len);
	return len;
}

int Command::payloadLength() const
{
	return qMin(0xffff, m_msg.length());
}

QJsonDocument Command::doc() const
{
	QJsonParseError e;
	QJsonDocument d = QJsonDocument::fromJson(m_msg, &e);
	if(e.error != QJsonParseError::NoError) {
		qWarning() << "JSON parse error:" << e.errorString();
	}
	return d;
}

QString Command::toString() const
{
	return QStringLiteral("# Command");
}

Disconnect *Disconnect::deserialize(uint8_t ctx, const uchar *data, uint len)
{
	if(len<1)
		return nullptr;
	return new Disconnect(ctx, Reason(*data), QByteArray(reinterpret_cast<const char*>(data)+1, len-1));
}

int Disconnect::serializePayload(uchar *data) const
{
	uchar *ptr = data;
	*(ptr++) = _reason;
	memcpy(ptr, _message.constData(), _message.length());
	ptr += _message.length();
	return ptr - data;
}

int Disconnect::payloadLength() const
{
	return 1 + _message.length();
}

QString Disconnect::toString() const
{
	return QStringLiteral("# Disconnected: ") + message();
}

Ping *Ping::deserialize(uint8_t ctx, const uchar *data, int len)
{
	if(len!=1)
		return nullptr;
	return new Ping(ctx, *data);
}

int Ping::payloadLength() const
{
	return 1;
}

int Ping::serializePayload(uchar *data) const
{
	*data = m_isPong;
	return 1;
}

QString Ping::toString() const
{
	return QStringLiteral("# ") + messageName();
}

DisconnectExt *DisconnectExt::deserialize(uint8_t ctx, const uchar *data, uint len)
{
	QJsonParseError error;
	const auto doc = QJsonDocument::fromJson(QByteArray(reinterpret_cast<const char *>(data), len), &error).object();
	if (error.error != QJsonParseError::NoError) {
		qWarning() << "JSON parse error:" << error.errorString();
		return new DisconnectExt(Error::InvalidMessage);
	}

	int kind;
	if (!doc.contains("type") || !doc.contains("detail")) {
		kind = Reason::OTHER;
	} else {
		kind = doc["type"].toInt();
		if (kind < 0 || kind >= Reason::_Last) {
			kind = Reason::OTHER;
		}
	}

	switch (kind) {
	case Reason::KICK: {
		const auto actor = ChatActor::fromJson(doc["detail"].toObject());
		if (actor.kind() != ChatActor::Unknown) {
			return new DisconnectExt(actor, ctx);
		}
		break;
	}
	case Reason::ERROR:
		return new DisconnectExt(
			Error::fromJson(
				doc["detail"].toObject(),
				doc["message"].toString()
			),
			ctx
		);
	case Reason::SHUTDOWN: {
		const auto shutdown = Shutdown::fromJson(doc["detail"].toObject());
		if (shutdown.kind() != Shutdown::Unknown) {
			return new DisconnectExt(shutdown, ctx);
		}
		break;
	}
	default: {}
	}

	return new DisconnectExt(doc["message"].toString(), ctx);
}

QString DisconnectExt::toString() const
{
	return QStringLiteral("# Disconnected (structured): ") + message();
}

int DisconnectExt::payloadLength() const
{
	return QJsonDocument(toJson()).toJson(QJsonDocument::Compact).size();
}

int DisconnectExt::serializePayload(uchar *data) const
{
	const auto json = QJsonDocument(toJson()).toJson(QJsonDocument::Compact);
	memcpy(data, json.constData(), json.size());
	return json.size();
}

}
