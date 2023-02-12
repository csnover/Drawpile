// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef JSONLISTMODEL_H
#define JSONLISTMODEL_H

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QVector>

namespace server {
namespace gui {

/**
 * @brief A generic table model for JSON objects
 */
class JsonListModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	struct Column {
		QString key;
		QString title;
	};
	JsonListModel(const QVector<Column> &columns, QObject *parent=nullptr);

	void setList(const QJsonArray &list);
	QJsonObject objectAt(int row) const { return m_list.at(row).toObject(); }

	int rowCount(const QModelIndex &parent=QModelIndex()) const override;
	int columnCount(const QModelIndex &parent=QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
	virtual QVariant getData(const QString &key, const QJsonObject &obj) const;
	virtual Qt::ItemFlags getFlags(const QJsonObject &obj) const;
	virtual int getId(const QJsonObject &obj) const;

	QJsonArray m_list;
	QVector<Column> m_columns;
};

}
}

#endif
