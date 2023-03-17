// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef BRUSHPREVIEW_H
#define BRUSHPREVIEW_H

#include "libclient/brushes/brush.h"
#include "rustpile/rustpile.h"

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
	void setPreviewShape(rustpile::BrushPreviewShape shape);

	rustpile::BrushPreviewShape previewShape() const { return m_shape; }

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
	void setUnderFill(bool underfill);

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

	rustpile::BrushPreview *m_previewcanvas = nullptr;
	QPixmap m_preview;

	rustpile::BrushPreviewShape m_shape = rustpile::BrushPreviewShape::Stroke;
	int m_fillTolerance = 0;
	int m_fillExpansion = 0;
	bool m_underFill = false;
	bool m_needUpdate = false;
};

}

#endif
