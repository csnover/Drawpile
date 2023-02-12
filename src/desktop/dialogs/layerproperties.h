// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef LAYERPROPERTIES_H
#define LAYERPROPERTIES_H

#include "libclient/canvas/layerlist.h"

#include <QDialog>

class Ui_LayerProperties;

namespace net {
	class Envelope;
}

namespace dialogs {

class LayerProperties : public QDialog
{
Q_OBJECT
public:
	explicit LayerProperties(uint8_t localUser, QWidget *parent = nullptr);
	~LayerProperties();

	void setLayerItem(const canvas::LayerListItem &data, const QString &creator, bool isDefault);
	void setControlsEnabled(bool enabled);
	void setOpControlsEnabled(bool enabled);

	int layerId() const { return m_item.id; }

signals:
	void layerCommand(const net::Envelope&);
	void visibilityChanged(int layerId, bool visible);

protected:
	virtual void showEvent(QShowEvent *event) override;

private slots:
	void emitChanges();

private:
	int searchBlendModeIndex(rustpile::Blendmode mode);

	Ui_LayerProperties *m_ui;
	canvas::LayerListItem m_item;
	uint8_t m_user;
};

}

#endif
