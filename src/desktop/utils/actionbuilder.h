// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DP_ACTIONBUILDER_H
#define DP_ACTIONBUILDER_H

#include "desktop/utils/dynamicui.h"
#include "libclient/utils/customshortcutmodel.h"

#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QString>

/**
 * @brief A helper class for configuring QActions
 */
class ActionBuilder {
public:
	using Translator = QString(*)(const char *, const char *, int);

	explicit ActionBuilder(QAction *action, Translator translator)
		: m_action(action)
		, tr(translator)
	{
		Q_ASSERT(m_action);
	}

	ActionBuilder(QObject *parent, Translator translator)
		: m_action(new QAction(parent))
		, tr(translator)
	{}

	QAction *operator*()
	{
		return m_action;
	}

	operator QAction*()
	{
		return m_action;
	}

	template <typename T>
	ActionBuilder &addTo(T *target)
	{
		target->addAction(m_action);
		return *this;
	}

	ActionBuilder &autoRepeat(bool enabled)
	{
		m_action->setAutoRepeat(enabled);
		return *this;
	}

	ActionBuilder &checkable()
	{
		m_action->setCheckable(true);
		return *this;
	}

	ActionBuilder &checked(bool checked = true)
	{
		m_action->setCheckable(true);
		m_action->setChecked(checked);
		m_action->setProperty("defaultValue", true);
		return *this;
	}

	ActionBuilder &data(const QVariant &var)
	{
		m_action->setData(var);
		return *this;
	}

	ActionBuilder &disabled()
	{
		m_action->setEnabled(false);
		return *this;
	}

	ActionBuilder &icon(const QString &name)
	{
		m_action->setIcon(QIcon::fromTheme(name));
		return *this;
	}

	ActionBuilder &menuRole(QAction::MenuRole role)
	{
		m_action->setMenuRole(role);
		return *this;
	}

	ActionBuilder &noDefaultShortcut() {
		Q_ASSERT(!m_action->objectName().isEmpty());
		CustomShortcutModel::registerCustomizableAction(m_action->objectName(), m_action->text().remove('&'), QKeySequence());
		return *this;
	}

	ActionBuilder &objectName(const QString &name)
	{
		m_action->setObjectName(name);
		return *this;
	}

	template<typename Sender, typename Signal, typename Slot>
	ActionBuilder &on(const Sender *sender, Signal signal, Slot slot)
	{
		QObject::connect(sender, signal, m_action, slot);
		return *this;
	}

	template<typename Fn>
	ActionBuilder &onToggled(Fn fn)
	{
		QObject::connect(m_action, &QAction::toggled, fn);
		return *this;
	}

	template<typename Receiver, typename Slot>
	ActionBuilder &onToggled(const Receiver *receiver, Slot slot)
	{
		QObject::connect(m_action, &QAction::toggled, receiver, slot);
		return *this;
	}

	template<typename Fn>
	ActionBuilder &onTriggered(Fn fn)
	{
		QObject::connect(m_action, &QAction::triggered, fn);
		return *this;
	}

	template<typename Receiver, typename Slot>
	ActionBuilder &onTriggered(const Receiver *receiver, Slot slot)
	{
		QObject::connect(m_action, &QAction::triggered, receiver, slot);
		return *this;
	}

	ActionBuilder &property(const char *name, const QVariant &value)
	{
		m_action->setProperty(name, value);
		return *this;
	}

	ActionBuilder &remembered()
	{
		// Tag this (checkable) action so that its state will be
		// saved and loaded.
		Q_ASSERT(!m_action->objectName().isEmpty());
		m_action->setCheckable(true);
		m_action->setProperty("remembered", true);
		return *this;
	}

	ActionBuilder &separator(bool b)
	{
		m_action->setSeparator(b);
		return *this;
	}

	ActionBuilder &shortcut(const QString &key)
	{
		return shortcut(QKeySequence(key));
	}

	ActionBuilder &shortcut(const QKeySequence &shortcut)
	{
		m_action->setShortcut(shortcut);
		if (!m_action->objectName().isEmpty()) {
			CustomShortcutModel::registerCustomizableAction(m_action->objectName(), m_action->text().remove('&'), shortcut);
		}
		return *this;
	}

	ActionBuilder &statusTip(const char *tip, const char *disambiguation = nullptr, int n = -1)
	{
		makeTranslator(m_action, [=, tr=tr, m_action=m_action] {
			m_action->setStatusTip(tr(tip, disambiguation, n));
		});
		return *this;
	}

	ActionBuilder &text(const char *name, const char *disambiguation = nullptr, int n = -1)
	{
		makeTranslator(m_action, [=, tr=tr, m_action=m_action] {
			m_action->setText(tr(name, disambiguation, n));
		});
		return *this;
	}

	template <typename Fn>
	typename std::enable_if_t<!std::is_convertible<Fn, const char *>::value, ActionBuilder &>
	text(Fn fn)
	{
		makeTranslator(m_action, [m_action=m_action, fn=std::move(fn)] {
			m_action->setText(fn());
		});
		return *this;
	}

	ActionBuilder &visible(bool v)
	{
		m_action->setVisible(v);
		return *this;
	}

private:
	struct TrFn {
		Translator tr;
		const char *name;
		const char *disambiguation;
		int n;

		QString operator()() {
			return tr(name, disambiguation, n);
		}
	};

	QAction *m_action;
	Translator tr;
};

class MenuBuilder {
public:
	using Translator = QString(*)(const char *, const char *, int);

	explicit MenuBuilder(QMenu *menu, Translator translator)
		: m_menu(menu)
		, tr(translator)
	{
		Q_ASSERT(m_menu);
	}

	MenuBuilder(QWidget *parent, Translator translator)
		: m_menu(new QMenu(parent))
		, tr(translator)
	{
		parent->addAction(m_menu->menuAction());
	}

	operator QMenu*()
	{
		return m_menu;
	}

	MenuBuilder &action(QAction *action)
	{
		m_menu->addAction(action);
		return *this;
	}

	template <typename Fn>
	typename std::enable_if_t<!std::is_convertible<Fn, QAction *>::value, MenuBuilder &>
	action(Fn fn)
	{
		QAction *unused;
		return action(unused, fn);
	}

	template <typename Fn>
	MenuBuilder &action(QAction *&var, Fn fn)
	{
		auto *action = new QAction(m_menu);
		fn(ActionBuilder(action, tr));
		m_menu->addAction(action);
		var = action;
		return *this;
	}

	MenuBuilder &addTo(QWidget *parent)
	{
		parent->addAction(m_menu->menuAction());
		return *this;
	}

	MenuBuilder &icon(const QString &name)
	{
		m_menu->setIcon(QIcon::fromTheme(name));
		return *this;
	}

	MenuBuilder &objectName(const QString &name)
	{
		m_menu->setObjectName(name);
		return *this;
	}

	template<typename Fn>
	MenuBuilder &onAboutToShow(Fn fn)
	{
		QObject::connect(m_menu, &QMenu::aboutToShow, fn);
		return *this;
	}

	template<typename Receiver, typename Slot>
	MenuBuilder &onAboutToShow(const Receiver *receiver, Slot slot)
	{
		QObject::connect(m_menu, &QMenu::aboutToShow, receiver, slot);
		return *this;
	}

	template<typename Fn>
	MenuBuilder &onTriggered(Fn fn)
	{
		QObject::connect(m_menu, &QMenu::triggered, fn);
		return *this;
	}

	template<typename Receiver, typename Slot>
	MenuBuilder &onTriggered(const Receiver *receiver, Slot slot)
	{
		QObject::connect(m_menu, &QMenu::triggered, receiver, slot);
		return *this;
	}

	MenuBuilder &separator()
	{
		m_menu->addSeparator();
		return *this;
	}

	MenuBuilder &submenu(QMenu *menu)
	{
		m_menu->addAction(menu->menuAction());
		return *this;
	}

	template <typename Fn>
	MenuBuilder &submenu(Fn fn)
	{
		auto *menu = new QMenu(m_menu);
		fn(MenuBuilder(menu, tr));
		m_menu->addAction(menu->menuAction());
		return *this;
	}

	MenuBuilder &title(const char *title, const char *disambiguation = nullptr, int n = -1)
	{
		auto *action = m_menu->menuAction();
		makeTranslator(action, [=, tr=tr] {
			action->setText(tr(title, disambiguation, n));
		});
		return *this;
	}

private:
	struct TrFn {
		Translator tr;
		const char *name;
		const char *disambiguation;
		int n;

		QString operator()() {
			return tr(name, disambiguation, n);
		}
	};

	QMenu *m_menu;
	Translator tr;
};

#endif // ACTIONBUILDER_H
