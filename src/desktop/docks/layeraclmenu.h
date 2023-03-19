// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef LAYERACLMENU_H
#define LAYERACLMENU_H

#include "rustpile/rustpile.h"

#include <QMenu>
#include <QVector>

class QAbstractItemModel;
class QAction;
class QActionGroup;
class QAbstractItemModel;
class QShowEvent;
class QWidget;

namespace docks {

class LayerAclMenu final : public QMenu
{
	Q_OBJECT
public:
	explicit LayerAclMenu(QWidget *parent=nullptr);

	void setUserList(QAbstractItemModel *model);
	void setAcl(bool lock, rustpile::Tier tier, const QVector<uint8_t> acl);
	void setCensored(bool censor);
	void setDefault(bool on);

signals:
	/**
	 * @brief Layer Access Control List changed
	 *
	 * This signal includes the new exclusive access list.
	 * The list is empty if all users have access.
	 *
	 * @param lock general layer lock
	 * @param ids list of user IDs.
	 */
	void layerAclChange(bool lock, rustpile::Tier tier, QVector<uint8_t> ids);

	/**
	 * @brief The censored checkbox was toggled
	 */
	void layerCensoredChange(bool censor);

	/**
	 * @brief The default checkbox was toggled
	 */
	void layerDefaultChange(bool on);

protected:
	void showEvent(QShowEvent *e) override;

private slots:
	void userClicked(QAction *useraction);
	void refreshParentalControls();

private:
	QAbstractItemModel *m_userlist;
	QVector<uint8_t> m_exclusives;
	QAction *m_lock;
	QAction *m_censored;
	QAction *m_default;
	QActionGroup *m_tiers;
	QActionGroup *m_users;
};

}

#endif // LAYERACLMENU_H
