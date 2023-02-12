// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libserver/thinserverclient.h"
#include "libshared/net/messagequeue.h"
#include "libserver/thinsession.h"

namespace server {

ThinServerClient::ThinServerClient(QTcpSocket *socket, ServerLog *logger, QObject *parent)
	: Client(socket, logger, parent)
	, m_historyPosition(-1)
{
	connect(messageQueue(), &protocol::MessageQueue::allSent,
		this, &ThinServerClient::sendNextHistoryBatch);
}

void ThinServerClient::sendNextHistoryBatch()
{
	// Only enqueue messages for uploading when upload queue is empty
	// and session is in a normal running state.
	// (We'll get another messagesAvailable signal when ready)
	if(session() == nullptr || messageQueue()->isUploading() || session()->state() != Session::State::Running)
		return;

	protocol::MessageList batch;
	int batchLast;
	std::tie(batch, batchLast) = session()->history()->getBatch(m_historyPosition);
	m_historyPosition = batchLast;
	messageQueue()->send(batch);

	static_cast<ThinSession*>(session())->cleanupHistoryCache();
}

}
