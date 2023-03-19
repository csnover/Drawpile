// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QDockWidget>

class QBoxLayout;
class QSpacerItem;

namespace docks {

class TitleWidget final : public QWidget
{
	Q_OBJECT
public:
	explicit TitleWidget(QDockWidget *parent = nullptr);

	void addCustomWidget(QWidget *widget, bool stretch=false);
	void addSpace(int space = -1);
	void addStretch(int stretch=0);

private slots:
	void onFeaturesChanged(QDockWidget::DockWidgetFeatures features);

private:
	class Button;

	QBoxLayout *m_layout;
	Button *m_dockButton;
	Button *m_closeButton;
	QSpacerItem *m_buttonSpacer;
};

}

#endif // TITLEWIDGET_H
