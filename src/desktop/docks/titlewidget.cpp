// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/docks/titlewidget.h"
#include "desktop/utils/qtguicompat.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QAbstractButton>
#include <QPainter>
#ifndef QT_NO_STYLE_PROXY
#include <QProxyStyle>
#endif
#include <QSpacerItem>
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

static constexpr int DEFAULT_WIDGET_COUNT = 3;

TitleWidget::TitleWidget(QDockWidget *parent)
	: QWidget(parent)
{
	m_layout = new QHBoxLayout;
	m_layout->setSpacing(0);
	const auto mx = style()->pixelMetric(QStyle::PM_ToolBarItemSpacing, nullptr, this);
	const auto my = style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, this);
	m_layout->setContentsMargins(mx, my, mx, my);
	setLayout(m_layout);

	m_buttonSpacer = new QSpacerItem(mx, 0, QSizePolicy::Fixed);
	m_layout->addSpacerItem(m_buttonSpacer);

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
	m_layout->insertWidget(m_layout->count()-DEFAULT_WIDGET_COUNT, widget);
	if(stretch)
		m_layout->setStretchFactor(widget, 1);
}

void TitleWidget::addSpace(int space)
{
	m_layout->insertSpacing(m_layout->count()-DEFAULT_WIDGET_COUNT, space);
}

void TitleWidget::addStretch(int stretch)
{
	m_layout->insertStretch(m_layout->count()-DEFAULT_WIDGET_COUNT, stretch);
}

void TitleWidget::onFeaturesChanged(QDockWidget::DockWidgetFeatures features)
{
	const bool hasFeatures = features & (
		QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable
	);

	const auto hasWidgets = layout()->count() > DEFAULT_WIDGET_COUNT;

	// Just hiding the widget itself has no effect for an unknown reason
	if (!hasWidgets && !hasFeatures) {
		m_layout->setContentsMargins(0, 0, 0, 0);
	} else {
		const auto mx = style()->pixelMetric(QStyle::PM_ToolBarItemSpacing, nullptr, this);
		const auto my = style()->pixelMetric(QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, this);
		m_layout->setContentsMargins(mx, my, mx, my);
	}

	m_buttonSpacer->changeSize(hasFeatures
		? style()->pixelMetric(QStyle::PM_ToolBarItemSpacing, nullptr, this)
		: 0
	, 0, hasWidgets ? QSizePolicy::Fixed : QSizePolicy::Expanding);
	m_dockButton->setVisible(features & QDockWidget::DockWidgetMovable);
	m_closeButton->setVisible(features & QDockWidget::DockWidgetClosable);
}

}
