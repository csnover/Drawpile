// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef LAYERPROPERTIES_H
#define LAYERPROPERTIES_H

#include "rustpile/rustpile.h"

#include <QDialog>
#include <QPersistentModelIndex>

class Ui_LayerProperties;

namespace canvas {
	class CanvasModel;
}

namespace net {
	class Envelope;
}

namespace dialogs {

class LayerProperties : public QDialog
{
Q_OBJECT
public:
	LayerProperties(canvas::CanvasModel &canvas, QPersistentModelIndex index, QWidget *parent = nullptr);
	~LayerProperties();

	void updateUi();
	void setControlsEnabled(bool enabled);
	void setOpControlsEnabled(bool enabled);

	int layerId() const { return m_layerId; }

signals:
	void visibilityChanged(int layerId, bool visible);

protected:
	virtual void showEvent(QShowEvent *event) override;

private slots:
	void modelReset();
	void emitChanges();

private:
	int searchBlendModeIndex(rustpile::Blendmode mode);

	Ui_LayerProperties *m_ui;
	canvas::CanvasModel &m_canvas;
	QPersistentModelIndex m_item;
	int m_layerId;
};

}

#endif
