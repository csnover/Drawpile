// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/chat/useritemdelegate.h"
#include "libclient/canvas/userlist.h"
#include "libclient/canvas/canvasmodel.h"
#include "libclient/utils/icon.h"
#include "libclient/net/servercmd.h"
#include "libclient/net/envelopebuilder.h"
#include "libclient/document.h"
#include "desktop/utils/actionbuilder.h"
#include "desktop/utils/qtguicompat.h"

#include <QPainter>
#include <QModelIndex>
#include <QMouseEvent>
#include <QMenu>

namespace widgets {

static const int MARGIN = 4;
static const int PADDING = 8;
static const int AVATAR_SIZE = 32;
static const int STATUS_OVERLAY_SIZE = 16;
static const int BUTTON_WIDTH = 16;

UserItemDelegate::UserItemDelegate(QObject *parent)
	: QAbstractItemDelegate(parent), m_doc(nullptr)
{
	MenuBuilder(&m_userMenu, tr)
		.action([=](ActionBuilder action) {
			m_menuTitle = action.text(QT_TR_NOOP("User"))
				.separator(true);
		})
		.action([=](ActionBuilder action) {
			m_opAction = action.text(QT_TR_NOOP("Operator"))
				.checkable()
				.onTriggered(this, &UserItemDelegate::toggleOpMode);
		})
		.action([=](ActionBuilder action) {
			m_trustAction = action.text(QT_TR_NOOP("Trusted"))
				.checkable()
				.onTriggered(this, &UserItemDelegate::toggleTrusted);
		})
		.separator()
		.action([=](ActionBuilder action) {
			m_lockAction = action.text(QT_TR_NOOP("Lock"))
				.checkable()
				.onTriggered(this, &UserItemDelegate::toggleLock);
		})
		.action([=](ActionBuilder action) {
			m_muteAction = action.text(QT_TR_NOOP("Mute"))
				.checkable()
				.onTriggered(this, &UserItemDelegate::toggleMute);
		})
		.separator()
		.action([=](ActionBuilder action) {
			m_undoAction = action.text(QT_TR_NOOP("Undo"))
				.onTriggered(this, &UserItemDelegate::undoByUser);
		})
		.action([=](ActionBuilder action) {
			m_redoAction = action.text(QT_TR_NOOP("Redo"))
				.onTriggered(this, &UserItemDelegate::redoByUser);
		})
		.separator()
		.action([=](ActionBuilder action) {
			m_kickAction = action.text(QT_TR_NOOP("Kick"))
				.onTriggered(this, &UserItemDelegate::kickUser);
		})
		.action([=](ActionBuilder action) {
			m_banAction = action.text(QT_TR_NOOP("Kick && Ban"))
				.onTriggered(this, &UserItemDelegate::banUser);
		})
		.separator()
		.action([=](ActionBuilder action) {
			m_chatAction = action.text(QT_TR_NOOP("Private Message"))
				.onTriggered(this, &UserItemDelegate::pmUser);
		});

	m_lockIcon = icon::fromTheme("object-locked");
	m_muteIcon = icon::fromTheme("irc-unvoice");
}

QSize UserItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);

	return QSize(AVATAR_SIZE*4, AVATAR_SIZE+2*MARGIN);
}

void UserItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();
	painter->fillRect(option.rect, option.backgroundBrush);
	painter->setRenderHint(QPainter::Antialiasing);

	// Draw avatar
	const QRect avatarRect(
		option.rect.x() + MARGIN,
		option.rect.y() + MARGIN,
		AVATAR_SIZE,
		AVATAR_SIZE
		);
	painter->drawPixmap(
		avatarRect,
		index.data(canvas::UserListModel::AvatarRole).value<QPixmap>()
		);

	// Draw status overlay
	const bool isLocked = index.data(canvas::UserListModel::IsLockedRole).toBool();
	if(isLocked || index.data(canvas::UserListModel::IsMutedRole).toBool()) {
		const QRect statusOverlayRect(
			avatarRect.right() - STATUS_OVERLAY_SIZE,
			avatarRect.bottom() - STATUS_OVERLAY_SIZE,
			STATUS_OVERLAY_SIZE,
			STATUS_OVERLAY_SIZE
			);

		painter->setBrush(option.palette.color(QPalette::AlternateBase));
		painter->setPen(QPen(option.palette.color(QPalette::Base), 2));
		painter->drawEllipse(statusOverlayRect.adjusted(-2, -2, 2, 2));
		if(isLocked)
			m_lockIcon.paint(painter, statusOverlayRect);
		else
			m_muteIcon.paint(painter, statusOverlayRect);
	}

	// Draw username
	const QRect usernameRect(
		avatarRect.right() + PADDING,
		avatarRect.y(),
		option.rect.width() - avatarRect.right() - PADDING - BUTTON_WIDTH - MARGIN,
		option.rect.height() - MARGIN*2
		);
	QFont font = option.font;
	font.setPixelSize(16);
	font.setWeight(QFont::Light);
	painter->setPen(option.palette.text().color());
	painter->setFont(font);
	painter->drawText(
		usernameRect,
		Qt::AlignLeft | Qt::AlignTop,
		index.data(canvas::UserListModel::NameRole).toString()
	);

	// Draw user flags
	QString flags;

	if(index.data(canvas::UserListModel::IsModRole).toBool()) {
		flags = tr("Moderator");

	} else {
		if(index.data(canvas::UserListModel::IsOpRole).toBool())
			flags = tr("Operator");
		else if(index.data(canvas::UserListModel::IsTrustedRole).toBool())
			flags = tr("Trusted");

		if(index.data(canvas::UserListModel::IsBotRole).toBool()) {
			if(!flags.isEmpty())
				flags += " | ";
			flags += tr("Bot");

		} else if(index.data(canvas::UserListModel::IsAuthRole).toBool()) {
			if(!flags.isEmpty())
				flags += " | ";
			flags += tr("Registered");
		}
	}

	if(!flags.isEmpty()) {
		font.setPixelSize(12);
		font.setWeight(QFont::Normal);
		painter->setFont(font);
		painter->drawText(usernameRect, Qt::AlignBottom, flags);
	}

	// Draw the context menu buttons
	const QRect buttonRect(
		option.rect.right() - BUTTON_WIDTH - MARGIN,
		option.rect.top() + MARGIN,
		BUTTON_WIDTH,
		option.rect.height() - 2*MARGIN
		);

	painter->setPen(Qt::NoPen);
	painter->setBrush(option.palette.color(QPalette::WindowText));

	const int buttonSize = buttonRect.height()/7;
	for(int i=0;i<3;++i) {
		painter->drawEllipse(QRect(
			buttonRect.x() + (buttonRect.width()-buttonSize)/2,
			buttonRect.y() + (1+i*2) * buttonSize,
			buttonSize,
			buttonSize
		));
	}

	painter->restore();
}

bool UserItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	Q_UNUSED(model);

	if(event->type() == QEvent::MouseButtonPress && m_doc) {
		const QMouseEvent *e = static_cast<const QMouseEvent*>(event);

		if(e->button() == Qt::RightButton || (e->button() == Qt::LeftButton && compat::mousePos(*e).x() > option.rect.right() - MARGIN - BUTTON_WIDTH)) {
			showContextMenu(index, compat::globalPos(*e));
			return true;
		}
	}
	else if(event->type() == QEvent::MouseButtonDblClick && m_doc) {
		const int userId = index.data(canvas::UserListModel::IdRole).toInt();
		if(userId>0 && userId != m_doc->canvas()->localUserId()) {
			emit requestPrivateChat(userId);
			return true;
		}
	}
	return false;
}

void UserItemDelegate::showContextMenu(const QModelIndex &index, const QPoint &pos)
{
	m_menuId = index.data(canvas::UserListModel::IdRole).toInt();

	m_menuTitle->setText(index.data(canvas::UserListModel::NameRole).toString());

	const bool amOp = m_doc->canvas()->aclState()->amOperator();
	const bool amDeputy = m_doc->canvas()->aclState()->amTrusted() && m_doc->isSessionDeputies();
	const bool isSelf = m_menuId == m_doc->canvas()->localUserId();
	const bool isMod = index.data(canvas::UserListModel::IsModRole).toBool();

	m_opAction->setChecked(index.data(canvas::UserListModel::IsOpRole).toBool());
	m_trustAction->setChecked(index.data(canvas::UserListModel::IsTrustedRole).toBool());
	m_lockAction->setChecked(index.data(canvas::UserListModel::IsLockedRole).toBool());
	m_muteAction->setChecked(index.data(canvas::UserListModel::IsMutedRole).toBool());

	// Can't deop self or moderators
	m_opAction->setEnabled(amOp && !isSelf && !isMod);

	m_trustAction->setEnabled(amOp);
	m_lockAction->setEnabled(amOp);
	m_muteAction->setEnabled(amOp);
	m_undoAction->setEnabled(amOp);
	m_redoAction->setEnabled(amOp);

	// Deputies can only kick non-trusted users
	// No-one can kick themselves or moderators
	const bool canKick = !isSelf && !isMod &&
		(
			amOp ||
			(amDeputy && !(
				index.data(canvas::UserListModel::IsOpRole).toBool() ||
				index.data(canvas::UserListModel::IsTrustedRole).toBool()
				)
			)
		);
	m_kickAction->setEnabled(canKick);
	m_banAction->setEnabled(canKick);

	// Can't chat with self
	m_chatAction->setEnabled(!isSelf);

	m_userMenu.popup(pos);
}

void UserItemDelegate::toggleOpMode(bool op)
{
	emit opCommand(m_doc->canvas()->userlist()->getOpUserCommand(m_doc->canvas()->localUserId(), m_menuId, op));
}

void UserItemDelegate::toggleTrusted(bool trust)
{
	emit opCommand(m_doc->canvas()->userlist()->getTrustUserCommand(m_doc->canvas()->localUserId(), m_menuId, trust));
}

void UserItemDelegate::toggleLock(bool op)
{
	emit opCommand(m_doc->canvas()->userlist()->getLockUserCommand(m_doc->canvas()->localUserId(), m_menuId, op));
}

void UserItemDelegate::toggleMute(bool mute)
{
	emit opCommand(net::ServerCommand::makeMute(m_menuId, mute));
}

void UserItemDelegate::kickUser()
{
	emit opCommand(net::ServerCommand::makeKick(m_menuId, false));
}

void UserItemDelegate::banUser()
{
	emit opCommand(net::ServerCommand::makeKick(m_menuId, true));
}

void UserItemDelegate::pmUser()
{
	emit requestPrivateChat(m_menuId);
}

void UserItemDelegate::undoByUser()
{
	net::EnvelopeBuilder eb;
	eb.buildUndo(m_doc->canvas()->localUserId(), m_menuId, false);
	emit opCommand(eb.toEnvelope());
}

void UserItemDelegate::redoByUser()
{
	net::EnvelopeBuilder eb;
	eb.buildUndo(m_doc->canvas()->localUserId(), m_menuId, true);
	emit opCommand(eb.toEnvelope());
}

}
