// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <memory>

namespace net { class Envelope; }
namespace canvas { class UserListModel; }

namespace widgets {

/**
 * @brief Chat window
 *
 * A widget for chatting with other users
 */
class ChatWidget : public QWidget
{
	Q_OBJECT
public:
	explicit ChatWidget(QWidget *parent=nullptr);
	~ChatWidget();

	void focusInput();
	void setAttached(bool isAttached);
	void setUserList(canvas::UserListModel *userlist);

public slots:
	/**
	 * @brief Set default message preservation mode
	 *
	 * This sets a visual cue that informs the user whether chat messages are preserved
	 * in the session history or not
	 *
	 */
	void setPreserveMode(bool preservechat);

	//! Display a received message
	void receiveMessage(int sender, int recipient, uint8_t tflags, uint8_t oflags, const QString &message);

	//! Display a system message
	void systemMessage(const QString& message, bool isAlert=false);

	//! Set the message pinned to the top of the chat box
	void setPinnedMessage(const QString &message);

	void userJoined(int id, const QString &name);
	void userParted(int id);
	void kicked(const QString &kickedBy);

	//! Empty the chat box
	void clear();

	//! Initialize the chat box for a new server
	void loggedIn(int myId);

	//! Open a private chat view with this user
	void openPrivateChat(int userId);

private slots:
	void sendMessage(const QString &msg);
	void chatTabSelected(int index);
	void chatTabClosed(int index);
	void showChatContextMenu(const QPoint &pos);
	void setCompactMode(bool compact);

signals:
	void message(const net::Envelope &msg);
	void detachRequested();
	void expandRequested();

private:
	struct Private;
	const std::unique_ptr<Private> d;
};

}

#endif
