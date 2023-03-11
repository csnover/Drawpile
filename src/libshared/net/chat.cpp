#include "libshared/net/chat.h"

#include <QJsonArray>
#include <QJsonObject>

namespace protocol {

ChatActor ChatActor::fromJson(const QJsonObject &data)
{
	if (!data.contains("kind"))
		return { Unknown, data["name"].toString() };

	auto kind = data["kind"].toInt();
	if (kind < 0 || kind >= _Last) {
		kind = Unknown;
	}

	return {
		static_cast<ChatActor::Kind>(kind),
		data["name"].toString()
	};
}

QJsonObject ChatActor::toJson() const
{
	return {{
		{ "kind", int(m_kind) },
		{ "name", m_name }
	}};
}

SystemChat SystemChat::fromJson(const QJsonObject &data, const QString &fallback)
{
	if (!data.contains("kind"))
		return { Unknown, ChatActor::Unknown, { fallback } };

	auto kind = data["kind"].toInt();
	if (kind < 0 || kind >= _Last) {
		kind = Unknown;
	}

	const auto actor = ChatActor::fromJson(data["actor"].toObject());

	return {
		static_cast<SystemChat::Kind>(kind),
		actor,
		data["args"].toVariant().toStringList()
	};
}

QJsonObject SystemChat::toJson() const
{
	return {{
		{ "kind", int(m_kind) },
		{ "actor", m_actor.toJson() },
		{ "args", QJsonArray::fromStringList(m_args) }
	}};
}

} // namespace protocol
