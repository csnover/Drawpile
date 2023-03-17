// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef USERITEMDELEGATE_H
#define USERITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QMenu>

class Document;

namespace net {
	class Envelope;
}

namespace widgets {

class UserItemDelegate : public QAbstractItemDelegate
{
	Q_OBJECT
public:
	UserItemDelegate(QObject *parent=nullptr);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

	void setDocument(Document *doc) { m_doc = doc; }

signals:
	void opCommand(const net::Envelope &msg);
	void requestPrivateChat(int userId);

private slots:
	void toggleOpMode(bool op);
	void toggleTrusted(bool trust);
	void toggleLock(bool lock);
	void toggleMute(bool mute);
	void kickUser();
	void banUser();
	void pmUser();
	void undoByUser();
	void redoByUser();

private:
	void showContextMenu(const QModelIndex &index, const QPoint &pos);

	QMenu m_userMenu;

	Document *m_doc;

	QIcon m_lockIcon;
	QIcon m_muteIcon;

	QAction *m_menuTitle;
	QAction *m_opAction;
	QAction *m_trustAction;
	QAction *m_lockAction;
	QAction *m_muteAction;
	QAction *m_kickAction;
	QAction *m_banAction;
	QAction *m_chatAction;
	QAction *m_undoAction;
	QAction *m_redoAction;

	int m_menuId;
};

}

#endif
