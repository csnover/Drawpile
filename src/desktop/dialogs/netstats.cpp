// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/netstats.h"
#include "ui_netstats.h"

namespace dialogs {

DP_DYNAMIC_DEFAULT_IMPL(NetStats)

NetStats::NetStats(QWidget *parent)
	: DynamicUiWidget(parent)
{
	m_recvLabelText = makeTranslator(this, [=](int bytes) {
		m_ui->recvLabel->setText(formatKb(bytes));
	}, 0);

	m_sentLabelText = makeTranslator(this, [=](int bytes) {
		m_ui->sentLabel->setText(formatKb(bytes));
	}, 0);

	m_lagLabelText = makeTranslator(this, [=](int lag) {
		m_ui->lagLabel->setText(
			lag == -1
			? tr("not connected")
			: tr("%L1ms").arg(lag)
		);
	}, -1);
}

NetStats::~NetStats() {}

void NetStats::setRecvBytes(int bytes)
{
	m_recvLabelText.args(bytes);
}

void NetStats::setSentBytes(int bytes)
{
	m_sentLabelText.args(bytes);
}

void NetStats::setCurrentLag(int lag)
{
	m_lagLabelText.args(lag);
}

void NetStats::setDisconnected()
{
	m_lagLabelText.args(-1);
}

QString NetStats::formatKb(int bytes)
{
	if(bytes < 1024)
		return tr("%L1b").arg(bytes);
	else if(bytes < 1024*1024)
		return tr("%L1KiB").arg(bytes / 1024);
	else
		return tr("%L1MiB").arg(float(bytes) / (1024*1024), 0, 'f', 1);
}

}
