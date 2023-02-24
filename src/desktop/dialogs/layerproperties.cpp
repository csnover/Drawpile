// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/layerproperties.h"
#include "libclient/canvas/blendmodes.h"
#include "libclient/canvas/layerlist.h"
#include "libclient/canvas/userlist.h"
#include "libclient/net/envelopebuilder.h"

#include "rustpile/rustpile.h"

#include "ui_layerproperties.h"

namespace dialogs {

LayerProperties::LayerProperties(canvas::CanvasModel &canvas, QPersistentModelIndex index, QWidget *parent)
	: QDialog(parent)
	, m_ui(new Ui_LayerProperties)
	, m_canvas(canvas)
	, m_item(index)
	, m_layerId(index.data(canvas::LayerListModel::IdRole).toInt())
{
	m_ui->setupUi(this);

	const auto modes = canvas::blendmode::layerModeNames();
	for(const auto &bm : modes) {
		m_ui->blendMode->addItem(bm.second, int(bm.first));
	}

	connect(m_ui->title, &QLineEdit::returnPressed, this, &QDialog::accept);
	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton *b) {
		if(m_ui->buttonBox->buttonRole(b) == QDialogButtonBox::ApplyRole)
			emitChanges();
	});
	connect(this, &QDialog::accepted, this, &LayerProperties::emitChanges);
	connect(m_canvas.layerlist(), &canvas::LayerListModel::modelReset, this, &LayerProperties::modelReset);

	updateUi();
}

LayerProperties::~LayerProperties()
{
	delete m_ui;
}

void LayerProperties::modelReset()
{
	const auto index = m_canvas.layerlist()->layerIndex(m_layerId);
	if(index.isValid()) {
		m_item = index;
		updateUi();
	} else {
		deleteLater();
	}
}

void LayerProperties::updateUi()
{
	const auto item = m_item.data(canvas::LayerListModel::ItemRole).value<canvas::LayerListItem>();
	const auto isDefault = m_item.data(canvas::LayerListModel::IsDefaultRole).toBool();

	m_ui->title->setText(item.title);
	m_ui->opacitySpinner->setValue(qRound(item.attributes.opacity * 100.0f));
	m_ui->visible->setChecked(!item.hidden);
	m_ui->censored->setChecked(item.attributes.censored);
	m_ui->defaultLayer->setChecked(isDefault);
	m_ui->defaultLayer->setEnabled(!isDefault);

	int blendModeIndex = searchBlendModeIndex(item.attributes.blend);
	if(blendModeIndex == -1) {
		// Apparently we don't know this blend mode, probably because
		// this client is outdated. Disable the control to avoid damage.
		m_ui->blendMode->setCurrentIndex(-1);
		m_ui->blendMode->setEnabled(false);

	} else {
		m_ui->blendMode->setCurrentIndex(blendModeIndex);
		m_ui->blendMode->setEnabled(true);
	}

	m_ui->passThrough->setChecked(!item.group || !item.attributes.isolated);
	m_ui->passThrough->setVisible(item.group);
	m_ui->createdBy->setText(m_canvas.userlist()->getUsername((item.id >> 8) & 0xff));
}

void LayerProperties::setControlsEnabled(bool enabled) {
	QWidget *w[] = {
		m_ui->title,
		m_ui->opacitySlider,
		m_ui->opacitySpinner,
		m_ui->blendMode,
		m_ui->passThrough,
		m_ui->censored
	};
	for(unsigned int i=0;i<sizeof(w)/sizeof(*w);++i)
		w[i]->setEnabled(enabled);
}

void LayerProperties::setOpControlsEnabled(bool enabled)
{
	m_ui->defaultLayer->setEnabled(enabled);
}

void LayerProperties::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);
	m_ui->title->setFocus(Qt::PopupFocusReason);
	m_ui->title->selectAll();
}

void LayerProperties::emitChanges()
{
	const auto layer = m_item.data(canvas::LayerListModel::ItemRole).value<canvas::LayerListItem>();

	auto *layers = m_canvas.layerlist();
	Q_ASSERT(layers);

	layers->setData(m_item, m_ui->title->text(), Qt::EditRole);
	if (m_ui->defaultLayer->isEnabled() && m_ui->defaultLayer->isChecked()) {
		layers->setData(m_item, true, canvas::LayerListModel::IsDefaultRole);
	}

	const auto attributes = canvas::LayerListItem::Attributes {
		float(m_ui->opacitySpinner->value()) / 100.0f,
		m_ui->censored->isChecked(),
		!m_ui->passThrough->isChecked(),
		static_cast<rustpile::Blendmode>(m_ui->blendMode->currentData().toInt())
	};
	layers->setData(m_item, QVariant::fromValue(attributes), canvas::LayerListModel::AttributesRole);

	if(m_ui->visible->isChecked() != layer.hidden) {
		emit visibilityChanged(layer.id, m_ui->visible->isChecked());
	}

	layers->submit();
}

int LayerProperties::searchBlendModeIndex(rustpile::Blendmode mode)
{
	const int blendModeCount = m_ui->blendMode->count();
	for(int i = 0; i < blendModeCount; ++i) {
		if(m_ui->blendMode->itemData(i).toInt() == static_cast<int>(mode)) {
			return i;
		}
	}
	return -1;
}

}
