/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2021 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "desktop/docks/titlewidget.h"
#include "desktop/utils/qtguicompat.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QAbstractButton>
#include <QPainter>
#ifndef QT_NO_STYLE_PROXY
#include <QProxyStyle>
#endif
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <Qt>

namespace docks {

/**
 * A window button for the custom title bar.
 *
 * This is a modified version of Qt's internal QDockWidgetTitleButton.
 */
class TitleWidget::Button : public QAbstractButton
{
public:
	Button(const QIcon &icon, QWidget *parent);

	QSize sizeHint() const override;
	QSize minimumSizeHint() const override { return sizeHint(); }

	bool event(QEvent *event) override;
	void enterEvent(compat::EnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
private:
	QSize dockButtonIconSize() const;
	mutable int m_iconSize = -1;
};

TitleWidget::Button::Button(const QIcon &icon, QWidget *parent)
	: QAbstractButton(parent)
{
	setIcon(icon);
	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

bool TitleWidget::Button::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::StyleChange:
	case QEvent::ScreenChangeInternal:
		m_iconSize = -1;
		break;
	default:
		break;
	}
	return QAbstractButton::event(event);
}

static inline bool isWindowsStyle(const QStyle *style)
{
	// Note: QStyleSheetStyle inherits QWindowsStyle
	const QStyle *effectiveStyle = style;

	// Qt checked for its internal QStyleSheetStyle here to get its baseStyle,
	// which is impossible in this implementation
#if !defined(QT_NO_STYLE_PROXY)
	if (style->inherits("QProxyStyle"))
		effectiveStyle = static_cast<const QProxyStyle *>(style)->baseStyle();
#endif

	return effectiveStyle->inherits("QWindowsStyle");
}

QSize TitleWidget::Button::dockButtonIconSize() const
{
	if (m_iconSize < 0) {
		m_iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
		// Dock Widget title buttons on Windows where historically limited to size 10
		// (from small icon size 16) since only a 10x10 XPM was provided.
		// Adding larger pixmaps to the icons thus caused the icons to grow; limit
		// this to qpiScaled(10) here.
		if (isWindowsStyle(style()))
			m_iconSize = qMin((10 * logicalDpiX()) / 96, m_iconSize);
	}
	return QSize(m_iconSize, m_iconSize);
}

QSize TitleWidget::Button::sizeHint() const
{
	ensurePolished();

	int size = 2*style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, this);
	if (!icon().isNull()) {
		const QSize sz = icon().actualSize(dockButtonIconSize());
		size += qMax(sz.width(), sz.height());
	}

	return QSize(size, size);
}

void TitleWidget::Button::enterEvent(compat::EnterEvent *event)
{
	if (isEnabled()) update();
	QAbstractButton::enterEvent(event);
}

void TitleWidget::Button::leaveEvent(QEvent *event)
{
	if (isEnabled()) update();
	QAbstractButton::leaveEvent(event);
}

void TitleWidget::Button::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);

	QStyleOptionToolButton opt;
	opt.initFrom(this);
	opt.state |= QStyle::State_AutoRaise;

	if (style()->styleHint(QStyle::SH_DockWidget_ButtonsHaveFrame, nullptr, this)) {
		if (isEnabled() && underMouse() && !isChecked() && !isDown())
			opt.state |= QStyle::State_Raised;
		if (isChecked())
			opt.state |= QStyle::State_On;
		if (isDown())
			opt.state |= QStyle::State_Sunken;
		p.drawPrimitive(QStyle::PE_PanelButtonTool, opt);
	} else if (isDown() || isChecked()) {
		// no frame, but the icon might have explicit pixmaps for QIcon::On
		opt.state |= QStyle::State_On | QStyle::State_Sunken;
	}

	opt.icon = icon();
	opt.subControls = { };
	opt.activeSubControls = { };
	opt.features = QStyleOptionToolButton::None;
	opt.arrowType = Qt::NoArrow;
	opt.iconSize = dockButtonIconSize();
	p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

TitleWidget::TitleWidget(QDockWidget *parent) : QWidget(parent)
{
	m_layout = new QHBoxLayout;
	m_layout->setSpacing(0);
	m_layout->setContentsMargins(6, 2, 1, 2);
	setLayout(m_layout);

	// (un)dock and close buttons
	m_dockButton = new Button(style()->standardIcon(QStyle::SP_TitleBarNormalButton), this);
	connect(m_dockButton, &QAbstractButton::clicked, parent, [parent]() {
		parent->setFloating(!parent->isFloating());
	});
	m_layout->addWidget(m_dockButton);

	m_closeButton = new Button(style()->standardIcon(QStyle::SP_TitleBarCloseButton), this);
	connect(m_closeButton, &QAbstractButton::clicked, parent, &QDockWidget::close);
	m_layout->addWidget(m_closeButton);

	onFeaturesChanged(parent->features());
	connect(parent, &QDockWidget::featuresChanged, this, &TitleWidget::onFeaturesChanged);
}

void TitleWidget::addCustomWidget(QWidget *widget, bool stretch)
{
	m_layout->insertWidget(m_layout->count()-2, widget);
	if(stretch)
		m_layout->setStretchFactor(widget, 1);
}


void TitleWidget::addSpace(int space)
{
	m_layout->insertSpacing(m_layout->count()-2, space);
}

void TitleWidget::addStretch(int stretch)
{
	m_layout->insertStretch(m_layout->count()-2, stretch);
}

void TitleWidget::onFeaturesChanged(QDockWidget::DockWidgetFeatures features)
{
	m_dockButton->setVisible(features.testFlag(QDockWidget::DockWidgetMovable));
	m_closeButton->setVisible(features.testFlag(QDockWidget::DockWidgetClosable));
}

}
