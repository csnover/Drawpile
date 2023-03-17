// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/hostdialog.h"

#include "desktop/utils/mandatoryfields.h"
#include "libclient/utils/sessionidvalidator.h"
#include "libclient/utils/images.h"
#include "libclient/utils/listservermodel.h"

#include "ui_hostdialog.h"

#include <QPushButton>
#include <QSettings>

namespace dialogs {

HostDialog::HostDialog(QWidget *parent)
	: DynamicUiWidget(parent)
{
	m_ui->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
	m_ui->idAlias->setValidator(new SessionIdAliasValidator(this));

	auto mandatoryfields = new MandatoryFields(this, m_ui->buttons->button(QDialogButtonBox::Ok));
	connect(m_ui->useremote, &QCheckBox::toggled,
		[this, mandatoryfields](bool checked) {
			m_ui->remotehost->setProperty("mandatoryfield", checked);
			mandatoryfields->update();
		}
	);

	QSettings cfg;
	cfg.beginGroup("history");

	m_ui->sessiontitle->setText(cfg.value("sessiontitle").toString());
	m_ui->idAlias->setText(cfg.value("idalias").toString());

	m_ui->announce->setChecked(cfg.value("announce", false).toBool());
	m_ui->listingserver->setModel(new sessionlisting::ListServerModel(false, this));
	m_ui->listingserver->setCurrentIndex(cfg.value("listingserver", 0).toInt());

	connect(m_ui->announce, &QCheckBox::toggled, this, &HostDialog::updateListingPermissions);
	connect(m_ui->listingserver, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &HostDialog::updateListingPermissions);

	if(cfg.value("hostremote", false).toBool())
		m_ui->useremote->setChecked(true);

	const QStringList recentRemoteHosts = cfg.value("recentremotehosts").toStringList();
	if(recentRemoteHosts.isEmpty())
		m_ui->remotehost->setCurrentText("pub.drawpile.net");
	else
		m_ui->remotehost->insertItems(0, recentRemoteHosts);

	updateListingPermissions();
	retranslateUi();
}

HostDialog::~HostDialog()
{}

void HostDialog::retranslateUi()
{
	m_ui->retranslateUi(this);
	m_ui->buttons->button(QDialogButtonBox::Ok)->setText(tr("Host"));
}

void HostDialog::rememberSettings() const
{
	QSettings cfg;
	cfg.beginGroup("history");

	cfg.setValue("sessiontitle", getTitle());
	cfg.setValue("announce", m_ui->announce->isChecked());
	cfg.setValue("listingserver", m_ui->listingserver->currentIndex());

	// Move current address to the top of the list
	const QString current = m_ui->remotehost->currentText();
	if(!current.isEmpty() && m_ui->useremote->isChecked()) {
		const int curIdx = m_ui->remotehost->findText(current);
		if(curIdx!=-1)
			m_ui->remotehost->removeItem(curIdx);

		QStringList hosts;
		hosts << current;

		for(int i=0;i<m_ui->remotehost->count();++i)
				hosts << m_ui->remotehost->itemText(i);
		m_ui->remotehost->setCurrentText(current);

		cfg.setValue("recentremotehosts", hosts);
	}

	cfg.setValue("hostremote", m_ui->useremote->isChecked());

	// Remember settings tab values
	cfg.setValue("idalias", m_ui->idAlias->text());

}

QString HostDialog::getRemoteAddress() const
{
	if(m_ui->useremote->isChecked())
		return m_ui->remotehost->currentText();
	return QString();
}

QString HostDialog::getTitle() const
{
	return m_ui->sessiontitle->text();
}

QString HostDialog::getPassword() const
{
	return m_ui->sessionpassword->text();
}

QString HostDialog::getSessionAlias() const
{
	return m_ui->idAlias->text();
}

QString HostDialog::getAnnouncementUrl() const
{
	if(m_ui->announce->isChecked())
		return m_ui->listingserver->currentData().toString();
	return QString();
}

bool HostDialog::getAnnouncmentPrivate() const
{
	return m_ui->listPrivate->isChecked();
}

void HostDialog::updateListingPermissions()
{
	if(!m_ui->announce->isChecked()) {
		m_ui->listPublic->setEnabled(false);
		m_ui->listPrivate->setEnabled(false);
	} else {
		const bool pub = m_ui->listingserver->currentData(sessionlisting::ListServerModel::PublicListRole).toBool();
		const bool priv = m_ui->listingserver->currentData(sessionlisting::ListServerModel::PrivateListRole).toBool();

		if(!pub && m_ui->listPublic->isChecked())
			m_ui->listPrivate->setChecked(true);
		else if(!priv && m_ui->listPrivate->isChecked())
			m_ui->listPublic->setChecked(true);

		m_ui->listPublic->setEnabled(pub);
		m_ui->listPrivate->setEnabled(priv);
	}
}

}
