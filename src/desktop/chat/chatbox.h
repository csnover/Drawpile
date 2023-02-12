// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CHATBOX_H
#define CHATBOX_H

#include <QWidget>

namespace net { class Envelope; }
namespace canvas {
	class CanvasModel;
}

class QListView;

class Document;

namespace widgets {

class ChatWidget;
class UserItemDelegate;

/**
 * Chat box with user list
 */
class ChatBox : public QWidget
{
	Q_OBJECT
public:
	explicit ChatBox(Document *doc, QWidget *parent=nullptr);

	//! Focus the text input widget
	void focusInput();

private slots:
	void onCanvasChanged(canvas::CanvasModel *canvas);
	void onServerLogin();
	void detachFromParent();
	void reattachToParent();

signals:
	//! User has written a new message
	void message(const net::Envelope &msg);

	//! The chatbox was either expanded or collapsed
	void expandedChanged(bool isExpanded);

	//! Request that the chatbox be expanded
	void expandPlease();

	//! Detached chat box should be re-attached and reparented (or it will be destroyed)
	void reattachNowPlease();

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	enum class State {
		Expanded,
		Collapsed,
		Detached
	};

	ChatWidget *m_chatWidget;
	UserItemDelegate *m_userItemDelegate;
	QListView *m_userList;

	State m_state = State::Expanded;
};

}

#endif
