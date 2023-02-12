// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include "thinsrv/gui/jsonlistmodel.h"

namespace server {
namespace gui {

class UserListModel : public JsonListModel
{
	Q_OBJECT
public:
	explicit UserListModel(QObject *parent=nullptr);

protected:
	QVariant getData(const QString &key, const QJsonObject &obj) const override;
	Qt::ItemFlags getFlags(const QJsonObject &obj) const override;
};

}
}

#endif
