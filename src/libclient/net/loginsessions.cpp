// SPDX-License-Identifier: GPL-3.0-or-later

#include "libclient/net/loginsessions.h"
#include "libclient/contentfilter/contentfilter.h"

#include <QDebug>
#include <QIcon>
#include <QPixmap>

namespace net {

LoginSessionModel::LoginSessionModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_moderatorMode(false)
{
}

void LoginSessionModel::setModeratorMode(bool mod)
{
	if(mod != m_moderatorMode) {
		beginResetModel();
		m_moderatorMode = mod;
		endResetModel();
	}
}

int LoginSessionModel::rowCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : m_sessions.size();
}

int LoginSessionModel::columnCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : ColumnCount;
}

QVariant LoginSessionModel::data(const QModelIndex &index, int role) const
{
	if(index.row() < 0 || index.row() >= m_sessions.size()) {
		return QVariant{};
	}

	const LoginSession &ls = m_sessions.at(index.row());

	switch(role) {
	case SortRole:
		if (index.column() == Version) {
			return ls.protocol.asInteger();
		}
		[[fallthrough]];
	case Qt::DisplayRole:
		switch(index.column()) {
		case Title: {
			QString title = ls.title.isEmpty() ? tr("(untitled)") : ls.title;
			if(ls.alias.isEmpty()) {
				return title;
			} else {
				return QStringLiteral("%1 [%2]").arg(title).arg(ls.alias);
			}
		}
		case Owner:
			return ls.founder;
		case UserCount:
			return ls.userCount;
		default:
			return QVariant{};
		}
	case Qt::DecorationRole:
		switch(index.column()) {
		case Version:
			if(ls.protocol.isCurrent()) {
				return QIcon::fromTheme("state-ok");
			} else if(ls.protocol.isPastCompatible()) {
				return QIcon::fromTheme("state-warning");
			} else {
				return QIcon::fromTheme("state-error");
			}
		case Title:
			if(ls.closed) {
				return QIcon::fromTheme("im-ban-user");
			} else if(ls.needPassword) {
				return QIcon::fromTheme("object-locked");
			} else if (isNsfm(ls)) {
				return QIcon(":/icons/censored.svg");
			} else {
				QPixmap pm(64, 64);
				pm.fill(Qt::transparent);
				return QIcon(pm);
			}
		default:
			return QVariant{};
		}
	case Qt::ToolTipRole:
		switch(index.column()) {
		case Version: {
			const auto version = ls.protocol.versionName();
			if (ls.protocol.isCurrent()) {
				return tr("Compatible");
			} else if (ls.protocol.isPastCompatible()) {
				return tr("Requires compatibility mode (%1)").arg(version);
			} else if (ls.protocol.isFuture()) {
				return tr("Requires newer client (%1)").arg(version);
			} else {
				return tr("Incompatible (%1)")
					.arg(version.isEmpty() ? tr("unknown version") : version);
			}
		}
		case Title:
			if(ls.closed) {
				return tr("Closed (new logins blocked)");
			} else if(ls.needPassword) {
				return tr("Session password required");
			} else {

			}
			return isNsfm(ls) ? tr("Not safe for me") : QVariant{};
		default:
			return QVariant{};
		}
	case Qt::TextAlignmentRole:
		return index.column() == UserCount ? Qt::AlignCenter : QVariant{};
	case IdRole: return ls.id;
	case IdAliasRole: return ls.alias;
	case AliasOrIdRole: return ls.idOrAlias();
	case UserCountRole: return ls.userCount;
	case TitleRole: return ls.title;
	case FounderRole: return ls.founder;
	case NeedPasswordRole: return ls.needPassword;
	case PersistentRole: return ls.persistent;
	case ClosedRole: return ls.closed;
	case IncompatibleRole: return !ls.protocol.isCompatible();
	case JoinableRole: return (!ls.closed || m_moderatorMode) && ls.protocol.isCompatible();
	case NsfmRole: return isNsfm(ls);
	case CompatibilityModeRole: return ls.protocol.isPastCompatible();
	default: return QVariant{};
	}
}

Qt::ItemFlags LoginSessionModel::flags(const QModelIndex &index) const
{
	if(index.row()<0 || index.row() >= m_sessions.size())
		return Qt::NoItemFlags;

	const LoginSession &ls = m_sessions.at(index.row());
	if(!ls.protocol.isCompatible() || (ls.closed && !m_moderatorMode))
		return Qt::NoItemFlags;
	else
		return QAbstractTableModel::flags(index);
}

QVariant LoginSessionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role != Qt::DisplayRole || orientation != Qt::Horizontal) {
		return QVariant{};
	}

	switch(section) {
	case Title: return tr("Title");
	case Owner: return tr("Started by");
	case UserCount: return tr("Users");
	default: return QVariant{};
	}
}

void LoginSessionModel::updateSession(const LoginSession &session)
{
	// If the session is already listed, update it in place
	for(int i=0;i<m_sessions.size();++i) {
		if(m_sessions.at(i).isIdOrAlias(session.idOrAlias())) {
			m_sessions[i] = session;
			emit dataChanged(index(i, 0), index(i, columnCount()));
			return;
		}
	}

	// Add a new session to the end of the list
	beginInsertRows(QModelIndex(), m_sessions.size(), m_sessions.size());
	m_sessions << session;
	endInsertRows();
}

void LoginSessionModel::removeSession(const QString &id)
{
	for(int i=0;i<m_sessions.size();++i) {
		if(m_sessions.at(i).isIdOrAlias(id)) {
			beginRemoveRows(QModelIndex(), i, i);
			m_sessions.removeAt(i);
			endRemoveRows();
			return;
		}
	}
}

bool LoginSessionModel::isNsfm(const LoginSession &session) const
{
	return (contentfilter::useAdvisoryTag() ? session.nsfm : false)
		|| contentfilter::isNsfmTitle(session.title);
}

} // namespace net
