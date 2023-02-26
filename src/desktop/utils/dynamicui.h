// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Drawpile contributors

#ifndef DP_DESKTOP_UTILS_DYNAMICUI_H
#define DP_DESKTOP_UTILS_DYNAMICUI_H

#include <QEvent>
#include <memory>
#include <utility>

#define DP_DYNAMIC_UI \
protected: \
	virtual void retranslateUi() override; \
private:

#define DP_DYNAMIC_DEFAULT_IMPL(Class) \
void Class::retranslateUi() \
{ \
	m_ui->retranslateUi(this); \
}

template<typename Base, typename Ui>
class DynamicUiWidget : public Base {
public:
	template<typename... Args>
	DynamicUiWidget(Args&&... args)
		: Base(std::forward<Args>(args)...)
		, m_ui(new Ui)
	{
		m_ui->setupUi(this);
	}

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

	const std::unique_ptr<Ui> m_ui;
};

#endif
