#include "libshared/net/error.h"

#include <QJsonArray>
#include <QJsonObject>

namespace protocol {

Error Error::fromJson(const QJsonObject &data, const QString &fallback)
{
	if (!data.contains("kind"))
		return { Unknown, { fallback } };

	auto kind = data["kind"].toInt();
	if (kind < 0 || kind >= _Last) {
		kind = Unknown;
	}

	return {
		static_cast<Error::Kind>(kind),
		data["args"].toVariant().toStringList()
	};
}

QJsonObject Error::toJson() const
{
	return {{
		{ "kind", int(m_kind) },
		{ "args", QJsonArray::fromStringList(m_args) },
		// For backwards-compatibility with existing wire protocol
		{ "code", code() }
	}};
}

Shutdown Shutdown::fromJson(const QJsonObject &data)
{
	if (!data.contains("kind"))
		return { Unknown };

	auto kind = data["kind"].toInt();
	if (kind < 0 || kind >= _Last)
		return { Unknown };

	return { static_cast<Shutdown::Kind>(kind) };
}

QJsonObject Shutdown::toJson() const
{
	return {{
		{ "kind", int(m_kind) }
	}};
}

} // namespace protocol
