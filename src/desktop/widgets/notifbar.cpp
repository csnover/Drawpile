// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/widgets/notifbar.h"

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QStyle>

namespace widgets {

static constexpr int BORDER = 4;

NotificationBar::NotificationBar(QWidget *parent)
	: QWidget(parent)
{
	hide();

	const auto space = style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, nullptr, this);

	auto *layout = new QHBoxLayout;
	setLayout(layout);
	const auto mx = style()->pixelMetric(QStyle::PM_LayoutLeftMargin, nullptr, this) / 2;
	layout->setContentsMargins(mx, BORDER, mx, BORDER - 1);
	layout->setSpacing(space);

	m_icon = new QLabel;
	layout->addWidget(m_icon);

	m_label = new QLabel;
	layout->addWidget(m_label);

	m_actionButton = new QPushButton;
#ifdef Q_OS_MACOS
	// This is to work around the shitshow that is QTBUG-2699 since the bad
	// alignment is just too obvious in this situation
	// (1) Stop the button from creating extra space at the bottom
	m_actionButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
	// (2) Shove the button down to align it correctly with the text baseline
	auto *buttonLayout = new QVBoxLayout;
	buttonLayout->setSpacing(0);
	buttonLayout->setContentsMargins(0, 0, 0, 0);
	buttonLayout->addSpacing(4);
	buttonLayout->addWidget(m_actionButton);
	layout->addLayout(buttonLayout);
#else
	layout->addWidget(m_actionButton, 0, Qt::AlignVCenter);
#endif
	connect(m_actionButton, &QPushButton::clicked, this, &NotificationBar::actionButtonClicked);
	connect(m_actionButton, &QPushButton::clicked, this, &NotificationBar::hide);

	layout->addStretch();

	m_closeButton = new QPushButton;
	m_closeButton->setFlat(true);
	m_closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
	const auto size = style()->pixelMetric(QStyle::PM_TitleBarButtonIconSize, nullptr, this);
	m_closeButton->setFixedSize(size, size);
	layout->addWidget(m_closeButton);
	connect(m_closeButton, &QPushButton::clicked, this, &NotificationBar::hide);
}

void NotificationBar::show(const QString &text, const QString &actionButtonLabel, RoleColor color)
{
	QStyle::StandardPixmap icon;
	switch(color) {
	case RoleColor::Warning:
		icon = QStyle::SP_MessageBoxWarning;
		m_color = Qt::yellow;
		break;
	case RoleColor::Fatal:
		icon = QStyle::SP_MessageBoxCritical;
		m_color = Qt::red;
		break;
	default:
		Q_UNREACHABLE();
	}

	auto pm = style()->standardIcon(icon, nullptr, this)
		.pixmap(style()->pixelMetric(QStyle::PM_LargeIconSize, nullptr, this));
	m_icon->setPixmap(pm);

	m_label->setText(text);
	m_actionButton->setHidden(actionButtonLabel.isEmpty());
	m_actionButton->setText(actionButtonLabel);

	if(color == RoleColor::Fatal) {
		m_closeButton->hide();
	}

	QWidget::show();
}

void NotificationBar::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);
	QPainter painter(this);
	auto rect = geometry();
	rect.setHeight(BORDER);
	painter.fillRect(rect, m_color);
}

}
