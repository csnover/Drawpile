// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef FLIPBOOK_H
#define FLIPBOOK_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>
#include <QList>
#include <QPixmap>
#include <QTimer>

class Ui_Flipbook;

namespace canvas {
	class PaintEngine;
}

namespace dialogs {

class Flipbook final : public DynamicUiWidget<QDialog, Ui_Flipbook>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit Flipbook(QWidget *parent=nullptr);
	~Flipbook() override;

	void setPaintEngine(canvas::PaintEngine *pe);

private slots:
	void loadFrame();
	void playPause();
	void rewind();
	void updateFps(int newFps);
	void updateRange();
	void setCrop(const QRectF &rect);
	void resetCrop();

private:
	void resetFrameCache();

	canvas::PaintEngine *m_paintengine;
	QList<QPixmap> m_frames;
	QTimer m_timer;
	QRect m_crop;
	int m_realFps;
	Translator<bool> m_timelineModeLabelText;
};

}

#endif // FLIPBOOK_H
