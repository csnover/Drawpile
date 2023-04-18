// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/dialogs/settingsdialog/common.h"
#include "desktop/dialogs/settingsdialog/input.h"
#include "desktop/widgets/tablettest.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QSettings>
#include <QSpacerItem>
#include <QString>
#include <QStyleFactory>
#include <QVBoxLayout>
#include <QWidget>

namespace dialogs {
namespace settingsdialog {

enum Driver {
	WindowsInk,
	WinTab,
	WinTabRelativePenMode,
};

Input::Input(QWidget *parent)
	: QWidget(parent)
{
	auto *layout = new QGridLayout(this);

	auto *form = new QFormLayout;
	layout->setColumnStretch(0, 2);
	layout->setColumnStretch(1, 1);
	layout->addLayout(form, 0, 0, 2, 1);

	auto *pressure = new QCheckBox(tr("Enable pressure sensitivity"));
	pressure->setProperty(KEY, "tabletevents");
	form->addRow(tr("Tablet:"), pressure);

	auto *eraser = new QCheckBox(tr("Detect eraser tip"));
	eraser->setProperty(KEY, "tableteraser");
	form->addRow(nullptr, eraser);

	addSpacer(form);

	auto [drivers, driversLayout] = addRadioGroup(tr("Driver:"), "windowsink", form, false, {
		tr("Windows Ink"),
		tr("Wintab")
	});

	auto *relativePenMode = new QCheckBox(tr("Relative pen mode"));
	connect(drivers, &QButtonGroup::idToggled, [=](int id, bool on) {
		relativePenMode->setEnabled(id == 1 && on);
	});
	driversLayout->addLayout(indent(relativePenMode));

	m_tabletSpacer = addSpacer(form);
	addSeparator(form);

	QButtonGroup *oneTouch;
	std::tie(oneTouch, std::ignore) = addRadioGroup(tr("One-finger input:"), nullptr, form, true, {
		tr("Do nothing"),
		tr("Draw"),
		tr("Scroll")
	});
	oneTouch->button(1)->setProperty(KEY, "touchdraw");
	oneTouch->button(2)->setProperty(KEY, "touchscroll");

	QButtonGroup *twoTouch;
	std::tie(twoTouch, std::ignore) = addRadioGroup(tr("Touch gestures:"), nullptr, form, true, {
		tr("Pinch to zoom"),
		tr("Twist to rotate")
	});
	twoTouch->button(0)->setProperty(KEY, "touchpinch");
	twoTouch->button(1)->setProperty(KEY, "touchtwist");

	m_testerFrame = new QFrame;
	m_testerFrame->setFrameShape(QFrame::StyledPanel);
	m_testerFrame->setFrameShadow(QFrame::Sunken);
	m_testerFrame->setMinimumSize(200, 200);
	auto *testerLayout = new QHBoxLayout(m_testerFrame);
	testerLayout->setContentsMargins(0, 0, 0, 0);
	testerLayout->addWidget(new widgets::TabletTester);
	layout->addWidget(m_testerFrame, 0, 1);

	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 1, 1);
}

void Input::resizeEvent(QResizeEvent *event)
{
	const auto spacerSize = qMax(0, m_testerFrame->geometry().bottom() + 1 - m_tabletSpacer->geometry().y());
	m_tabletSpacer->changeSize(0, spacerSize, QSizePolicy::Fixed, QSizePolicy::Fixed);

	QWidget::resizeEvent(event);
}

} // namespace settingsdialog
} // namespace dialogs
