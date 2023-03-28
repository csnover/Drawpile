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

class TimelineWidget final : public QWidget
{
	Q_OBJECT
public:
	explicit TimelineWidget(QWidget *parent = nullptr);
	~TimelineWidget() override;

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
	void paintEvent(QPaintEvent *) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

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
