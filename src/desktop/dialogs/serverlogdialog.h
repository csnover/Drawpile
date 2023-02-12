// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SERVERLOGDIALOG_H
#define SERVERLOGDIALOG_H

#include <QDialog>

class Ui_ServerLogDialog;

class QSortFilterProxyModel;
class QItemSelection;
class QAbstractItemModel;

namespace canvas {
	class UserListModel;
}

namespace net {
	class Envelope;
}

namespace dialogs {

class ServerLogDialog : public QDialog
{
	Q_OBJECT
public:
	ServerLogDialog(QWidget *parent=nullptr);
	~ServerLogDialog();

	void setModel(QAbstractItemModel *model);
	void setUserList(canvas::UserListModel *userlist);

public slots:
	void setOperatorMode(bool op);

signals:
	void inspectModeChanged(int contextId);
	void inspectModeStopped();
	void opCommand(const net::Envelope &msg);

protected:
	void hideEvent(QHideEvent *event) override;

private slots:
	void userSelected(const QItemSelection &selected);

	void setInspectMode(bool inspect);
	void kickSelected();
	void banSelected();
	void undoSelected();
	void redoSelected();

private:
	Ui_ServerLogDialog *m_ui;
	QSortFilterProxyModel *m_eventlogProxy;

	QSortFilterProxyModel *m_userlistProxy;
	canvas::UserListModel *m_userlist;

	bool m_opMode;

	uint8_t selectedUserId() const;
};

}

#endif
