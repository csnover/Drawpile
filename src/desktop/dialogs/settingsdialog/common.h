// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_DIALOGS_SETTINGSDIALOG_COMMON_H
#define DESKTOP_DIALOGS_SETTINGSDIALOG_COMMON_H

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QDebug>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPalette>
#include <QRadioButton>
#include <QSpacerItem>
#include <QStyle>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>
#include <tuple>

namespace dialogs {
namespace settingsdialog {

constexpr auto KEY = "setting";

inline auto checkBoxLabelSpacing(QWidget *widget)
{
	const auto *style = widget->style() ? widget->style() : QApplication::style();
	Q_ASSERT(style);
	return style->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, nullptr, widget);
}

inline auto indentSize(QWidget *widget)
{
	const auto *style = widget->style() ? widget->style() : QApplication::style();
	Q_ASSERT(style);
	return style->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, widget)
		+ checkBoxLabelSpacing(widget);
}

inline QFrame *addSeparator(QLayout *layout)
{
	auto *separator = new QFrame;
	separator->setForegroundRole(QPalette::Dark);
	separator->setFrameShape(QFrame::HLine);
	layout->addWidget(separator);
	return separator;
}

inline QSpacerItem *addSpacer(QLayout *layout)
{
	auto *spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
	layout->addItem(spacer);
	return spacer;
}

inline std::tuple<QButtonGroup *, QBoxLayout *> addRadioGroup(
	const QString &label,
	const char *setting,
	QFormLayout *form,
	bool horizontal,
	const QStringList &items
)
{
	auto *group = new QButtonGroup(form);
	if (setting) {
		group->setProperty(KEY, setting);
	}

	QBoxLayout *layout = new QBoxLayout(
		horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom
	);

	layout->setSpacing(QApplication::style()->combinedLayoutSpacing(
		QSizePolicy::RadioButton,
		QSizePolicy::RadioButton,
		horizontal ? Qt::Horizontal : Qt::Vertical
	));

	for (auto index = 0; index < items.size(); ++index) {
		auto *button = new QRadioButton(items[index]);
		group->addButton(button, index);
		layout->addWidget(button);
	}

	form->addRow(label, layout);

	return std::make_tuple(group, layout);
}

inline QHBoxLayout *indent(QWidget *child)
{
	auto *layout = new QHBoxLayout;
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	// Use addSpacing instead of setContentsMargins to support RTL
	layout->addSpacing(indentSize(child));
	layout->addWidget(child);
	return layout;
}

inline QHBoxLayout *encapsulate(const QString &label, QWidget *child)
{
	auto *layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->setSizeConstraint(QLayout::SetFixedSize);

	auto text = label.split("%1");

	if(text.size() > 0 && !text.first().isEmpty()) {
		auto *left = new QLabel(text.first());
		left->setBuddy(child);
		layout->addWidget(left);
	}

	layout->addWidget(child);

	if(text.size() > 1 && !text.at(1).isEmpty()) {
		auto *right = new QLabel(text.at(1));
		right->setBuddy(child);
		layout->addWidget(right);
	}

	return layout;
}

} // namespace settingsdialog
} // namespace dialogs

#endif
