// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/dialogs/settingsdialog/common.h"
#include "desktop/dialogs/settingsdialog/tools.h"

#include <QApplication>
#include <QPainter>
#include <QPen>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QStyle>
#include <QStyleOptionButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QtColorWidgets/ColorWheel>

namespace dialogs {
namespace settingsdialog {

enum Driver {
	WindowsInk,
	WinTab,
	WinTabRelativePenMode,
};

Tools::Tools(QWidget *parent)
	: QWidget(parent)
{
	auto *layout = new QVBoxLayout(this);

	initGeneralTools(layout);
	addSeparator(layout);
	initColorWheel(layout);

	layout->addStretch(1);
}

void Tools::initGeneralTools(QVBoxLayout *layout)
{
	auto *form = new QFormLayout;
	layout->addLayout(form);

	auto *toggleKeys = new QCheckBox(tr("Toggle between previous and current tool"));
	toggleKeys->setProperty(KEY, "tooltoggle");
	form->addRow(tr("Keyboard shortcuts:"), toggleKeys);

	auto *shareColor = new QCheckBox(tr("Share one colour across all brush slots"));
	shareColor->setProperty(KEY, "sharebrushslotcolor");
	form->addRow(tr("Brushes:"), shareColor);

	auto *brushCursor = new QComboBox;
	brushCursor->setProperty(KEY, "brushcursor");
	// Always adjust in case of locale changes
	brushCursor->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	brushCursor->addItems({
		tr("Dot"),
		tr("Crosshair"),
		tr("Arrow"),
		tr("Right-Handed Triangle"),
		tr("Left-Handed Triangle")
	});
	m_fart = encapsulate(tr("Draw the brush as a %1"), brushCursor);
	form->addRow(QString(), m_fart);

	auto *outlineSize = new QDoubleSpinBox;
	outlineSize->setProperty(KEY, "brushoutlinewidth");
	outlineSize->setDecimals(1);
	outlineSize->setMinimum(.5);
	outlineSize->setMaximum(25.0);
	outlineSize->setSingleStep(.5);
	outlineSize->setSuffix(tr("px"));
	auto *outlineSizeLayout = encapsulate(tr("Show a %1 outline around the brush"), outlineSize);

	auto *showOutline = new QCheckBox;
	showOutline->setAccessibleName(tr("Enable brush outline"));
	connect(showOutline, &QCheckBox::toggled, outlineSize, &QSpinBox::setEnabled);
	outlineSizeLayout->insertWidget(0, showOutline);
	outlineSizeLayout->insertSpacing(1, checkBoxLabelSpacing(this));

	// QTBUG-14643, QTBUG-85142
	// TODO: Use a proxy style that fixes this bullshit, already need one for
	// other things
#	ifdef Q_OS_MACOS
	// The magic number comes from qmacstyle_mac.mm - SE_CheckBoxLayoutItem
	outlineSizeLayout->insertSpacing(1, 9);
#	endif

	auto *container = new QWidget;
	container->setLayout(outlineSizeLayout);
	form->addRow(" ", container);
}

void Tools::initColorWheel(QVBoxLayout *parentLayout)
{
	auto *layout = new QHBoxLayout;
	parentLayout->addLayout(layout);

	auto *form = new QFormLayout;
	layout->addLayout(form);

	QButtonGroup *shape;
	std::tie(shape, std::ignore) = addRadioGroup(tr("Shape:"), "colorwheel/shape", form, false, {
		tr("Triangle"),
		tr("Square")
	});

	addSpacer(form);

	QButtonGroup *angle;
	std::tie(angle, std::ignore) = addRadioGroup(tr("Angle:"), "colorwheel/rotate", form, false, {
		tr("Fixed"),
		tr("Rotating")
	});

	addSpacer(form);

	QButtonGroup *space;
	std::tie(space, std::ignore) = addRadioGroup(tr("Color space:"), "colorwheel/space", form, false, {
		tr("HSV (Hue–Saturation–Value)"),
		tr("HSL (Hue–Saturation–Lightness)"),
		tr("HCL (Hue–Chroma–Luminance)")
	});

	auto *preview = new color_widgets::ColorWheel;
	layout->addWidget(preview);
	// TODO: This is wrong, needs to be reactive to the settings data model
	// that does not exist
	connect(shape, &QButtonGroup::idToggled, [=](int id, bool checked) {
		if (checked) {
			preview->setSelectorShape(static_cast<color_widgets::ColorWheel::ShapeEnum>(id));
		}
	});
	connect(angle, &QButtonGroup::idToggled, [=](int id, bool checked) {
		if (checked) {
			preview->setRotatingSelector(id != 0);
		}
	});
	connect(space, &QButtonGroup::idToggled, [=](int id, bool checked) {
		if (checked) {
			preview->setColorSpace(static_cast<color_widgets::ColorWheel::ColorSpaceEnum>(id));
		}
	});
}

void Tools::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);

	QPainter p(this);
	QPen poop;
	poop.setColor(Qt::red);
	poop.setWidth(1);
	p.setPen(poop);
	p.drawRect(m_fart->geometry());
}

} // namespace settingsdialog
} // namespace dialogs
