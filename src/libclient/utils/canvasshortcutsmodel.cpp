/*
 * Drawpile - a collaborative drawing program.
 *
 * Copyright (C) 2023 askmeaboutloom
 *
 * Drawpile is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drawpile is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "libclient/tools/canvasshortcutsmodel.h"
#include <QCoreApplication>
#include <QKeySequence>

CanvasShortcutsModel::CanvasShortcutsModel(QObject *parent)
	: QAbstractTableModel{parent}
	, m_canvasShortcuts{}
	, m_hasChanges{false}
{
}

CanvasShortcutsModel::~CanvasShortcutsModel() {}

void CanvasShortcutsModel::loadShortcuts(QSettings &cfg)
{
	beginResetModel();
	m_canvasShortcuts = CanvasShortcuts::load(cfg);
	m_hasChanges = false;
	endResetModel();
}

void CanvasShortcutsModel::saveShortcuts(QSettings &cfg)
{
	m_canvasShortcuts.save(cfg);
}

void CanvasShortcutsModel::restoreDefaults()
{
	beginResetModel();
	m_canvasShortcuts.clear();
	m_canvasShortcuts.loadDefaults();
	m_hasChanges = true;
	endResetModel();
}

int CanvasShortcutsModel::rowCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : m_canvasShortcuts.shortcutsCount();
}

int CanvasShortcutsModel::columnCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : 3;
}

QVariant CanvasShortcutsModel::data(const QModelIndex &index, int role) const
{
	bool canHandleIndex =
		index.isValid() && !index.parent().isValid() && role == Qt::DisplayRole;
	if(canHandleIndex) {
		const CanvasShortcuts::Shortcut *s = shortcutAt(index.row());
		if(s) {
			switch(index.column()) {
			case 0:
				return shortcutToString(s->type, s->mods, s->keys, s->button);
			case 1:
				return actionToString(*s);
			case 2:
				return flagsToString(*s);
			default:
				break;
			}
		}
	}
	return QVariant();
}


bool CanvasShortcutsModel::removeRows(
	int row, int count, const QModelIndex &parent)
{
	bool canRemove = !parent.isValid() && count > 0 && row >= 0 &&
					 row + count <= m_canvasShortcuts.shortcutsCount();
	if(canRemove) {
		beginRemoveRows(parent, row, row + count - 1);
		m_canvasShortcuts.removeShortcutAt(row, count);
		m_hasChanges = true;
		endRemoveRows();
	}
	return false;
}


QVariant CanvasShortcutsModel::headerData(
	int section, Qt::Orientation orientation, int role) const
{
	if(role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch(section) {
		case 0:
			return tr("Shortcut");
		case 1:
			return tr("Action");
		case 2:
			return tr("Modifiers");
		default:
			break;
		}
	}
	return QVariant{};
}

Qt::ItemFlags CanvasShortcutsModel::flags(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

const CanvasShortcuts::Shortcut *CanvasShortcutsModel::shortcutAt(int row) const
{
	return m_canvasShortcuts.shortcutAt(row);
}

int CanvasShortcutsModel::addShortcut(const CanvasShortcuts::Shortcut &s)
{
	if(s.isValid()) {
		beginResetModel();
		int row = m_canvasShortcuts.addShortcut(s);
		m_hasChanges = true;
		endResetModel();
		return row;
	} else {
		return -1;
	}
}

int CanvasShortcutsModel::editShortcut(
	const CanvasShortcuts::Shortcut &prev, const CanvasShortcuts::Shortcut &s)
{
	if(s.isValid()) {
		beginResetModel();
		int row = m_canvasShortcuts.editShortcut(prev, s);
		m_hasChanges = true;
		endResetModel();
		return row;
	} else {
		return -1;
	}
}

const CanvasShortcuts::Shortcut *CanvasShortcutsModel::searchConflict(
	const CanvasShortcuts::Shortcut &s,
	const CanvasShortcuts::Shortcut *except) const
{
	return m_canvasShortcuts.searchConflict(s, except);
}

QString CanvasShortcutsModel::shortcutTitle(
	const CanvasShortcuts::Shortcut *s, bool actionAndFlagsOnly)
{
	if(s) {
		QString action = actionToString(*s);
		QString flags = flagsToString(*s);
		if(actionAndFlagsOnly) {
			if(flags.isEmpty()) {
				//: Example: "Pan Canvas"
				return tr("%1").arg(action);
			} else {
				//: Example: "Pan Canvas (Inverted)"
				return tr("%1 (%2)").arg(action).arg(flags);
			}
		} else {
			QString ss = shortcutToString(s->type, s->mods, s->keys, s->button);
			if(flags.isEmpty()) {
				//: Example: "Space: Pan Canvas"
				return tr("%1: %2").arg(ss).arg(action);
			} else {
				//: Example: "Space: Pan Canvas (Inverted)"
				return tr("%1: %2 (%3)").arg(ss).arg(action).arg(flags);
			}
		}
	} else {
		return QString{};
	}
}

QString CanvasShortcutsModel::shortcutToString(
	unsigned int type, Qt::KeyboardModifiers mods, const QSet<Qt::Key> &keys,
	Qt::MouseButton button)
{
	QStringList components;
	for(const Qt::Key key : keys) {
		components.append(QKeySequence{key}.toString(QKeySequence::NativeText));
	}
	std::sort(components.begin(), components.end());

#ifdef Q_OS_MACOS
	// OSX has different modifier keys and also orders them differently.
	if(mods.testFlag(Qt::ControlModifier)) {
		components.prepend(QStringLiteral("⌘"));
	}
	if(mods.testFlag(Qt::ShiftModifier)) {
		components.prepend(QStringLiteral("⇧"));
	}
	if(mods.testFlag(Qt::AltModifier)) {
		components.prepend(QStringLiteral("⎇"));
	}
	if(mods.testFlag(Qt::MetaModifier)) {
		components.prepend(QStringLiteral("⌃"));
	}
#else
	// Qt translates modifiers in the QShortcut context, so we do too.
	if(mods.testFlag(Qt::ShiftModifier)) {
		components.prepend(QCoreApplication::translate("QShortcut", "Shift"));
	}
	if(mods.testFlag(Qt::AltModifier)) {
		components.prepend(QCoreApplication::translate("QShortcut", "Alt"));
	}
	if(mods.testFlag(Qt::ControlModifier)) {
		components.prepend(QCoreApplication::translate("QShortcut", "Ctrl"));
	}
	if(mods.testFlag(Qt::MetaModifier)) {
		components.prepend(QCoreApplication::translate("QShortcut", "Meta"));
	}
#endif

	if(type == CanvasShortcuts::MOUSE_BUTTON) {
		components.append(mouseButtonToString(button));
	} else if(type == CanvasShortcuts::MOUSE_WHEEL) {
		components.append(tr("Mouse Wheel"));
	}

	//: Joins shortcut components, probably doesn't need to be translated.
	return components.join(tr("+"));
}

bool CanvasShortcutsModel::hasChanges() const
{
	return m_hasChanges;
}

QString CanvasShortcutsModel::mouseButtonToString(Qt::MouseButton button)
{
	switch(button) {
	case Qt::NoButton:
		return tr("Unset");
	case Qt::LeftButton:
		return tr("Left Click");
	case Qt::RightButton:
		return tr("Right Click");
	case Qt::MiddleButton:
		return tr("Middle Click");
	default:
		break;
	}

	using MouseButtonType = std::underlying_type<Qt::MouseButton>::type;
	constexpr MouseButtonType max = Qt::MaxMouseButton;
	int buttonIndex = 0;
	for(MouseButtonType mb = 1, i = 1; mb <= max; mb *= 2, ++i) {
		if(mb == button) {
			buttonIndex = i;
			break;
		}
	}

	if(buttonIndex == 0) {
		return tr("Unknown Button 0x%1").arg(button, 0, 16);
	} else {
		return tr("Button %1").arg(buttonIndex);
	}
}

QString CanvasShortcutsModel::actionToString(const CanvasShortcuts::Shortcut &s)
{
	switch(s.action) {
	case CanvasShortcuts::CANVAS_PAN:
		return tr("Pan Canvas");
	case CanvasShortcuts::CANVAS_ROTATE:
		return tr("Rotate Canvas");
	case CanvasShortcuts::CANVAS_ZOOM:
		return tr("Zoom Canvas");
	case CanvasShortcuts::COLOR_PICK:
		return tr("Pick Color");
	case CanvasShortcuts::LAYER_PICK:
		return tr("Pick Layer");
	case CanvasShortcuts::TOOL_ADJUST:
		return tr("Change Brush Size");
	case CanvasShortcuts::CONSTRAINT:
		switch(s.flags & CanvasShortcuts::CONSTRAINT_MASK) {
		case CanvasShortcuts::TOOL_CONSTRAINT1:
			return tr("Constrain Tool");
		case CanvasShortcuts::TOOL_CONSTRAINT2:
			return tr("Center Tool");
		case CanvasShortcuts::TOOL_CONSTRAINT1 |
			CanvasShortcuts::TOOL_CONSTRAINT2:
			return tr("Constrain and Center Tool");
		default:
			return tr("Unknown Constraint 0x%1").arg(s.flags, 0, 16);
		}
	default:
		return tr("Unknown Action %1").arg(s.action);
	}
}

QString CanvasShortcutsModel::flagsToString(const CanvasShortcuts::Shortcut &s)
{
	if(s.flags & CanvasShortcuts::INVERTED) {
		if(s.flags & CanvasShortcuts::SWAP_AXES) {
			return tr("Inverted, Swap Axes");
		} else {
			return tr("Inverted");
		}
	} else if(s.flags & CanvasShortcuts::SWAP_AXES) {
		return tr("Swap Axes");
	} else {
		return QString{};
	}
}
