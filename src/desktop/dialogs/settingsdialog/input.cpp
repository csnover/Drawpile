// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/dialogs/settingsdialog/input.h"
#include "desktop/settings.h"
#include "desktop/utils/sanerformlayout.h"
#include "desktop/widgets/curvewidget.h"
#include "desktop/widgets/tablettest.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QWidget>

namespace dialogs {
namespace settingsdialog {

Input::Input(desktop::settings::Settings &settings, QWidget *parent)
	: QWidget(parent)
{
	auto *form = new utils::SanerFormLayout(this);

	initTablet(settings, form);
	form->addSeparator();
	initPressureCurve(settings, form);
	form->addSeparator();
	initTouch(settings, form);
}

void Input::initPressureCurve(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *curve = new widgets::CurveWidget(this);
	curve->setMinimumHeight(200);
	curve->setAxisTitleLabels(tr("Input"), tr("Output"));
	settings.bindGlobalPressureCurve(curve, &widgets::CurveWidget::setCurveFromString);
	connect(curve, &widgets::CurveWidget::curveChanged, &settings, [&settings](const KisCubicCurve &newCurve) {
		settings.setGlobalPressureCurve(newCurve.toString());
	});
	curve->setFixedButtonWidth(194);
	auto *label = new QLabel(tr("Global pressure curve:"));
	label->setWordWrap(true);
	label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	label->setAlignment(form->labelAlignment() | Qt::AlignTop);
	form->addRow(label, curve, 1, 2);
}

void Input::initTablet(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *pressure = new QCheckBox(tr("Enable pressure sensitivity"));
	settings.bindTabletEvents(pressure);
	form->addRow(tr("Tablet:"), pressure);

	auto *eraser = new QCheckBox(tr("Detect eraser tip"));
	settings.bindTabletEraser(eraser);
	form->addRow(nullptr, eraser);

	form->addSpacer();

#ifdef Q_OS_WIN
	auto *drivers = form->addRadioGroup(tr("Driver:"), false, {
		{ tr("Windows Ink"), 0 },
		{ tr("Wintab"), 1 }
	});
	drivers->button(1)->setChecked(true);
	settings.bindTabletWindowsInk(drivers->button(0));

	auto *relativePenMode = new QCheckBox(tr("Relative pen mode"));
	utils::setSpacingControlType(relativePenMode, QSizePolicy::RadioButton);
	settings.bindTabletRelativePenMode(relativePenMode);
	settings.bindTabletWindowsInk(relativePenMode, &QCheckBox::setDisabled);
	form->addRow(nullptr, utils::indent(relativePenMode));

	form->addSpacer();
#endif

	auto *smoothing = new QSlider(Qt::Horizontal);
	smoothing->setMaximum(10);
	smoothing->setTickPosition(QSlider::TicksBelow);
	smoothing->setTickInterval(1);
	settings.bindSmoothing(smoothing);
	form->addRow(tr("Smoothing:"), utils::labelEdges(smoothing, tr("Less"), tr("More")));

	form->addSpacer(QSizePolicy::MinimumExpanding);

	auto *testerFrame = new QFrame;
	testerFrame->setFrameShape(QFrame::StyledPanel);
	testerFrame->setFrameShadow(QFrame::Sunken);
	testerFrame->setMinimumSize(194, 194);
	auto *testerLayout = new QHBoxLayout(testerFrame);
	testerLayout->setContentsMargins(0, 0, 0, 0);
	testerLayout->addWidget(new widgets::TabletTester);
	form->addAside(testerFrame, 0);
}

void Input::initTouch(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *oneTouch = form->addRadioGroup(tr("One-finger input:"), true, {
		{ tr("Do nothing"), 0 },
		{ tr("Draw"), 1 },
		{ tr("Scroll"), 2 },
	}, 2);
	settings.bindOneFingerDraw(oneTouch->button(1));
	settings.bindOneFingerScroll(oneTouch->button(2));

	auto *twoTouch = new utils::EncapsulatedLayout;
	twoTouch->setContentsMargins(0, 0, 0, 0);
	auto *zoom = new QCheckBox(tr("Pinch to zoom"));
	settings.bindTwoFingerZoom(zoom);
	twoTouch->addWidget(zoom);
	auto *rotate = new QCheckBox(tr("Twist to rotate"));
	settings.bindTwoFingerRotate(rotate);
	twoTouch->addWidget(rotate);
	twoTouch->addStretch();
	form->addRow(tr("Touch gestures:"), twoTouch, 1, 2);
}

} // namespace settingsdialog
} // namespace dialogs
