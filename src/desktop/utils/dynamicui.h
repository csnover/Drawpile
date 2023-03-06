// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Drawpile contributors

#ifndef DP_DESKTOP_UTILS_DYNAMICUI_H
#define DP_DESKTOP_UTILS_DYNAMICUI_H

#include <QEvent>
#include <QObject>
#include <QPointer>
#include <functional>
#include <memory>
#include <utility>

class QString;

/**
 * Translates an object using the default translation function from the current
 * lexical scope and retranslates automatically if the language changes.
 */
#define AUTO_TR(object, method, call) \
makeTranslator((object), [=] { \
	(object)->method(call); \
})

template <typename ...Args>
class Translator final {
	class Base : public QObject {
	public:
		Base(QObject *parent) : QObject(parent) {}
		virtual ~Base() {}
		virtual void args(Args... args) = 0;
		virtual void trigger() = 0;
	};

	template <typename Callback>
	class Listener final : public Base {
	public:
		Listener(QObject *parent, Callback callback, Args... args)
			: Base(parent)
			, m_callback(std::move(callback))
			, m_args(args...)
		{
			std::apply(m_callback, m_args);
			parent->installEventFilter(this);
		}

		~Listener()
		{
			this->parent()->removeEventFilter(this);
		}

		bool eventFilter(QObject *object, QEvent *event) override
		{
			if (event->type() == QEvent::LanguageChange) {
				std::apply(m_callback, m_args);
			}
			return QObject::eventFilter(object, event);
		}

		void args(Args... args) override
		{
			m_args = std::tuple(args...);
			std::apply(m_callback, m_args);
		}

		void trigger() override
		{
			std::apply(m_callback, m_args);
		}
	private:
		Callback m_callback;
		std::tuple<Args...> m_args;
	};

public:
	Translator() : m_listener(nullptr) {}
	template <typename Callback>
	Translator(QObject *parent, Callback callback, Args... args)
		: m_listener(new Listener<Callback>(parent, std::move(callback), args...)) {}

	void args(Args... args)
	{
		if (m_listener) {
			m_listener->args(args...);
		}
	}

	void trigger()
	{
		if (m_listener) {
			m_listener->trigger();
		}
	}
private:
	QPointer<Base> m_listener;
};

template <typename Callback, typename ...Args>
Translator<Args...> makeTranslator(QObject *object, Callback callback, Args... args)
{
	Q_ASSERT(object);
	return Translator<Args...>(object, std::move(callback), args...);
}

#define DP_DYNAMIC_UI \
protected: \
	virtual void retranslateUi() override; \
private:

#define DP_DYNAMIC_DEFAULT_IMPL(Class) \
void Class::retranslateUi() \
{ \
	m_ui->retranslateUi(this); \
}

template <typename Base>
class Dynamic : public Base {
public:
	template<typename... Args, typename = decltype(Base(std::declval<Args>()...))>
	Dynamic(Args&&... args)
		: Base(std::forward<Args>(args)...) {}

protected:
	virtual void changeEvent(QEvent *event) override {
		Base::changeEvent(event);
		switch (event->type()) {
		case QEvent::LanguageChange:
			retranslateUi();
			break;
		case QEvent::PaletteChange:
			recolorUi();
			break;
		default: {}
		}
	}

	virtual void retranslateUi() = 0;
	virtual void recolorUi() {}
};

template<typename Base, typename Ui>
class DynamicUiWidget : public Dynamic<Base> {
public:
	template<typename... Args>
	DynamicUiWidget(Args&&... args)
		: Dynamic<Base>(std::forward<Args>(args)...)
		, m_ui(new Ui)
	{
		m_ui->setupUi(this);
	}

protected:
	const std::unique_ptr<Ui> m_ui;
};

#endif
