// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Aurélien Gâteau <agateau@kde.org>
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/widgets/groupedtoolbutton.h"
#include "libclient/utils/icon.h"

#include <QAction>
#include <QPainterPath>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QToolButton>
#include <QEvent>
#include <QScopedValueRollback>

namespace widgets
{

GroupedToolButton::GroupedToolButton(QWidget *parent) : GroupedToolButton(NotGrouped, parent) { }

GroupedToolButton::GroupedToolButton(GroupPosition position, QWidget* parent)
	: QToolButton(parent)
	, mGroupPosition(position)
{
	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void GroupedToolButton::setGroupPosition(GroupedToolButton::GroupPosition groupPosition)
{
	mGroupPosition = groupPosition;
}

void GroupedToolButton::setColorSwatch(const QColor &c)
{
	m_colorSwatch = c;
	update();
}

static QPainterPath makePath(const QRect &rect, GroupedToolButton::GroupPosition mGroupPosition, bool mask)
{
	constexpr auto RADIUS = 8;

	QPainterPath path;

	auto x = rect.x();
	auto y = rect.y();
	auto w = rect.width();
	auto h = rect.height();

	const auto rl = (mGroupPosition & GroupedToolButton::GroupRight) ? 0 : RADIUS;
	const auto rr = (mGroupPosition & GroupedToolButton::GroupLeft) ? 0 : RADIUS;

	if (rl) {
		path.arcMoveTo(x, y, rl, rl, 180);
		path.arcTo(x, y, rl, rl, 180, -90);
	} else {
		path.moveTo(x, y);
	}

	if (rr) {
		path.arcTo(x+w-rr, y, rr, rr, 90, -90);
		path.arcTo(x+w-rr, y+h-rr, rr, rr, 0, -90);
	} else {
		path.lineTo(x+w, y);
		if (mask) {
			path.lineTo(x+w, y+h);
		} else {
			path.moveTo(x+w, y+h);
		}
		path.arcMoveTo(x+w, y+h, 0, 0, 360);
	}

	if (rl) {
		path.arcTo(x, y+h-rl, rl, rl, 270, -90);
		// Since a path with no right radius is disjoint it is not possible
		// to close the subpath and need to just draw to connect it up to the
		// first element instead
		auto first = path.elementAt(0);
		path.lineTo(first.x, first.y);
	} else {
		path.lineTo(x, y+h);
		if (mask) {
			path.closeSubpath();
		}
	}

	return path;
}

void GroupedToolButton::paintEvent(QPaintEvent *)
{
	QStylePainter painter(this);
	QStyleOptionToolButton opt;
	initStyleOption(&opt);
	opt.palette.setColor(QPalette::Window, opt.palette.color(QPalette::Light));

	const auto border = makePath(opt.rect, mGroupPosition, false);
	const auto mask = makePath(opt.rect, mGroupPosition, true);

	painter.save();
	painter.setClipPath(mask);

	// Color swatch (if set)
	if(m_colorSwatch.isValid()) {
		const int swatchH = 5;
		const QRect swatchRect = QRect(opt.rect.x(), opt.rect.bottom()-swatchH, opt.rect.width(), swatchH);
		painter.fillRect(swatchRect, m_colorSwatch);
		opt.rect.setHeight(opt.rect.height() - swatchH);
	}

	auto panelOpt = opt;

	// Hide default borders by moving them outside the clipping area
	const auto fw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	panelOpt.rect.adjust(-fw, -fw, fw, fw);

	painter.drawComplexControl(QStyle::CC_ToolButton, panelOpt);
	painter.restore();

	painter.save();
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(opt.palette.color(QPalette::Mid));
	painter.drawPath(border);
	painter.restore();

	// Separators
	const int y1 = opt.rect.top() + 5;
	const int y2 = opt.rect.bottom() - 5;
	if (mGroupPosition & GroupRight) {
		const int x = opt.rect.left();
		painter.setPen(opt.palette.color(QPalette::Light));
		painter.drawLine(x, y1, x, y2);
	}
	if (mGroupPosition & GroupLeft) {
		const int x = opt.rect.right();
		painter.setPen(opt.palette.color(QPalette::Mid));
		painter.drawLine(x, y1, x, y2);
	}
}

}
