// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef NETSTATS_H
#define NETSTATS_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>

class Ui_NetStats;

namespace dialogs {

class NetStats : public DynamicUiWidget<QDialog, Ui_NetStats>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit NetStats(QWidget *parent = 0);
	~NetStats();

public slots:
	void setSentBytes(int bytes);
	void setRecvBytes(int bytes);
	void setCurrentLag(int lag);
	void setDisconnected();

private:
	QString formatKb(int bytes);

	Translator<int> m_recvLabelText;
	Translator<int> m_sentLabelText;
	Translator<int> m_lagLabelText;
};

}

#endif // NETSTATS_H
