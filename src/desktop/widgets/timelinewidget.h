// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QWidget>
#include <memory>

namespace canvas {
	class TimelineModel;
}

namespace net {
	class Envelope;
}

namespace widgets {

class TimelineWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TimelineWidget(QWidget *parent = nullptr);
	~TimelineWidget();

	void setModel(canvas::TimelineModel *model);
	void setCurrentFrame(int frame);
	void setCurrentLayer(int layerId);
	void setEditable(bool editable);

	canvas::TimelineModel *model() const;

	int currentFrame() const;
	int currentLayerId() const;

signals:
	void timelineEditCommand(const net::Envelope &e);
	void selectFrameRequest(int frame, int layerId);

protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);
	void wheelEvent(QWheelEvent *event);

private slots:
	void setHorizontalScroll(int pos);
	void setVerticalScroll(int pos);

	void onLayersChanged();

private:
	void updateScrollbars();

	struct Private;
	const std::unique_ptr<Private> d;
};

}

#endif // TIMELINEWIDGET_H
