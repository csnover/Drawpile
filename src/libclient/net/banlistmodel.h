// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef BANLISTMODEL_H
#define BANLISTMODEL_H

#include <QAbstractTableModel>

class QJsonArray;

namespace net {

struct BanlistEntry {
	int id;
	QString username;
	QString ip;
	QString bannedBy;
};

/**
 * @brief A representation of the serverside banlist
 *
 * This is just for showing the list to the user
 */
class BanlistModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit BanlistModel(QObject *parent=nullptr);

	int rowCount(const QModelIndex &parent=QModelIndex()) const;
	int columnCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;

	//! Replace banlist content
	void updateBans(const QJsonArray &banlist);

	void clear();

private:
	QList<BanlistEntry> m_banlist;
};

}

#endif // BANLISTMODEL_H
