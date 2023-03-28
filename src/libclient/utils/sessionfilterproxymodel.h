// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SESSIONFILTERPROXYMODEL_H
#define SESSIONFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/**
 * A custom SortFilter proxy model that has special support for
 *  - SessionListingModel
 *  - LoginSessionModel
 *
 * Otherwise works like normal QSortFilterProxyModel
 */
class SessionFilterProxyModel final : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	SessionFilterProxyModel(QObject *parent=nullptr);

	bool showNsfw() const { return m_showNsfw; }
	bool showPassworded() const { return m_showPassworded; }

public slots:
	void setShowNsfw(bool show);
	void setShowPassworded(bool show);
	void setShowClosed(bool show);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
	bool m_showPassworded;
	bool m_showNsfw;
	bool m_showClosed;
};

#endif
