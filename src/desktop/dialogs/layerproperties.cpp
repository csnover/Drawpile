// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/layerproperties.h"
#include "libclient/canvas/blendmodes.h"
#include "libclient/net/envelopebuilder.h"

#include "rustpile/rustpile.h"

#include "ui_layerproperties.h"

namespace dialogs {

LayerProperties::LayerProperties(uint8_t localUser, QWidget *parent)
	: QDialog(parent), m_user(localUser)
{
	m_ui = new Ui_LayerProperties;
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
}

LayerProperties::~LayerProperties()
{
	delete m_ui;
}

void LayerProperties::setLayerItem(const canvas::LayerListItem &item, const QString &creator, bool isDefault)
{
	m_item = item;

	m_ui->title->setText(item.title);
	m_ui->opacitySpinner->setValue(qRound(item.opacity * 100.0f));
	m_ui->visible->setChecked(!item.hidden);
	m_ui->censored->setChecked(item.censored);
	m_ui->defaultLayer->setChecked(isDefault);
	m_ui->defaultLayer->setEnabled(!isDefault);

	int blendModeIndex = searchBlendModeIndex(item.blend);
	if(blendModeIndex == -1) {
		// Apparently we don't know this blend mode, probably because
		// this client is outdated. Disable the control to avoid damage.
		m_ui->blendMode->setCurrentIndex(-1);
		m_ui->blendMode->setEnabled(false);

	} else {
		m_ui->blendMode->setCurrentIndex(blendModeIndex);
		m_ui->blendMode->setEnabled(true);
	}

	m_ui->passThrough->setChecked(!item.group || !item.isolated);
	m_ui->passThrough->setVisible(item.group);
	m_ui->createdBy->setText(creator);
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
	net::EnvelopeBuilder eb;

	if(m_item.title != m_ui->title->text()) {
		rustpile::write_retitlelayer(
			eb,
			m_user,
			m_item.id,
			reinterpret_cast<const uint16_t*>(m_ui->title->text().constData()),
			m_ui->title->text().length()
		);
	}

	const int oldOpacity = qRound(m_item.opacity * 100.0);
	const rustpile::Blendmode newBlendmode = static_cast<rustpile::Blendmode>(m_ui->blendMode->currentData().toInt());
	const bool censored = m_ui->censored->isChecked();
	const bool isolated = !m_ui->passThrough->isChecked();

	if(
		m_ui->opacitySpinner->value() != oldOpacity ||
		newBlendmode != m_item.blend ||
		censored != m_item.censored ||
		isolated != m_item.isolated
	) {
		rustpile::write_layerattr(
			eb,
			m_user,
			m_item.id,
			0,
			(censored ? rustpile::LayerAttributesMessage_FLAGS_CENSOR : 0) |
			(isolated ? rustpile::LayerAttributesMessage_FLAGS_ISOLATED : 0),
			qRound(m_ui->opacitySpinner->value() / 100.0 * 255),
			newBlendmode
		);
	}

	if(m_ui->visible->isChecked() != (!m_item.hidden)) {
		emit visibilityChanged(m_item.id, m_ui->visible->isChecked());
	}

	if(m_ui->defaultLayer->isEnabled() && m_ui->defaultLayer->isChecked()) {
		rustpile::write_defaultlayer(eb, m_user, m_item.id);
	}

	emit layerCommand(eb.toEnvelope());
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
