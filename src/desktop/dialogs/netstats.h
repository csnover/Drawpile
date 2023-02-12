// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef NETSTATS_H
#define NETSTATS_H

#include <QDialog>

class Ui_NetStats;

namespace dialogs {

class NetStats : public QDialog
{
	Q_OBJECT
public:
	explicit NetStats(QWidget *parent = 0);

public slots:
	void setSentBytes(int bytes);
	void setRecvBytes(int bytes);
	void setCurrentLag(int lag);
	void setDisconnected();

private:
	Ui_NetStats *_ui;
};

}

#endif // NETSTATS_H
