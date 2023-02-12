// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef LISTSERVERDELEGATE_H
#define LISTSERVERDELEGATE_H

#include <QItemDelegate>

namespace sessionlisting {

class ListServerDelegate : public QItemDelegate
{
public:
	ListServerDelegate(QObject *parent=0);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

}

#endif // LISTSERVERDELEGATE_H
