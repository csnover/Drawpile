// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SESSIONLISTMODEL_H
#define SESSIONLISTMODEL_H

#include "thinsrv/gui/jsonlistmodel.h"

namespace server {
namespace gui {

class SessionListModel : public JsonListModel
{
	Q_OBJECT
public:
	explicit SessionListModel(QObject *parent=nullptr);

protected:
	QVariant getData(const QString &key, const QJsonObject &obj) const override;
};

}
}

#endif
