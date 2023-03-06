// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef VIEWSTATUS_H
#define VIEWSTATUS_H

#include "desktop/utils/dynamicui.h"

#include <QWidget>

class QComboBox;
class QSlider;

namespace widgets {

class GroupedToolButton;
class KisAngleGauge;

class ViewStatus : public QWidget
{
	Q_OBJECT
public:
	ViewStatus(QWidget *parent=nullptr);

	void setActions(QAction *flip, QAction *mirror, QAction *rotationReset, QAction *zoomReset);

public slots:
	void setTransformation(qreal zoom, qreal angle);
	void setMinimumZoom(int zoom);

signals:
	void zoomChanged(qreal newZoom);
	void angleChanged(qreal newAngle);

private slots:
	void zoomBoxChanged(const QString &text);
	void zoomSliderChanged(int value);
	void angleBoxChanged(const QString &text);

private:
	QSlider *m_zoomSlider;
	QComboBox *m_zoomBox;
	KisAngleGauge *m_compass;
	QComboBox *m_angleBox;
	bool m_updating;
	widgets::GroupedToolButton *m_viewFlip, *m_viewMirror, *m_rotationReset, *m_zoomReset;
};

}

#endif
