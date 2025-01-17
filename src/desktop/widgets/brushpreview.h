// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BRUSHPREVIEW_H
#define BRUSHPREVIEW_H

#include "libclient/brushes/brush.h"
#include "libclient/drawdance/brushpreview.h"

#include <QFrame>

#ifdef DESIGNER_PLUGIN
#include <QtUiPlugin/QDesignerExportWidget>
#else
#define QDESIGNER_WIDGET_EXPORT
#endif

class QMenu;

namespace widgets {

/**
 * @brief Brush previewing widget
 */
class QDESIGNER_WIDGET_EXPORT BrushPreview final : public QFrame {
	Q_OBJECT
public:
	enum PreviewShape {Stroke, Line, Rectangle, Ellipse, FloodFill, FloodErase};

	explicit BrushPreview(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
	~BrushPreview() override;

	//! Set preview shape
	void setPreviewShape(DP_BrushPreviewShape shape);

	DP_BrushPreviewShape previewShape() const { return m_shape; }

	QColor brushColor() const { return m_brush.qColor(); }
	const brushes::ActiveBrush &brush() const { return m_brush; }

public slots:
	//! Set the brush to preview
	void setBrush(const brushes::ActiveBrush& brush);

	//! This is used for flood fill preview only
	void setFloodFillTolerance(int tolerance);

	//! This is used for flood fill preview only
	void setFloodFillExpansion(int expansion);

	//! This is used for flood fill preview only
	void setFloodFillFeatherRadius(int featherRadius);

	//! This is used for flood fill preview only
	void setFloodFillMode(int mode);

signals:
	void requestColorChange();

protected:
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *) override;
	void changeEvent(QEvent *event) override;
	void mouseDoubleClickEvent(QMouseEvent*) override;

private:
	void updatePreview();
	void updateBackground();

	QPixmap m_background;
	brushes::ActiveBrush m_brush;

#ifndef DESIGNER_PLUGIN
	drawdance::BrushPreview m_brushPreview;
#endif

	DP_BrushPreviewShape m_shape = DP_BRUSH_PREVIEW_STROKE;
	int m_fillTolerance = 0;
	int m_fillExpansion = 0;
	int m_fillFeatherRadius = 0;
	int m_fillMode = 0;
	bool m_needUpdate = false;
};

}

#endif

