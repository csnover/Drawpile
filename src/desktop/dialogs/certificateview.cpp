// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/certificateview.h"
#include "desktop/utils/dynamicui.h"
#include "ui_certificateview.h"

#include <QSslCertificate>

namespace dialogs {

namespace {
	QString first(const QStringList &sl)
	{
		if(sl.isEmpty())
			return QStringLiteral("(not set)");
		return sl.at(0);
	}
}

DP_DYNAMIC_DEFAULT_IMPL(CertificateView)

CertificateView::CertificateView(const QString &hostname, const QSslCertificate &certificate, QWidget *parent)
	: DynamicUiWidget(parent)
{
	setWindowTitle(tr("SSL Certificate for %1").arg(hostname));

	m_ui->cn_label->setText(first(certificate.subjectInfo(QSslCertificate::CommonName)));
	m_ui->org_label->setText(first(certificate.subjectInfo(QSslCertificate::Organization)));
	m_ui->orgunit_label->setText(first(certificate.subjectInfo(QSslCertificate::OrganizationalUnitName)));
	m_ui->sn_label->setText(certificate.serialNumber());

	m_ui->icn_label->setText(first(certificate.issuerInfo(QSslCertificate::CommonName)));
	m_ui->io_label->setText(first(certificate.issuerInfo(QSslCertificate::Organization)));
	m_ui->iou_label->setText(first(certificate.issuerInfo(QSslCertificate::OrganizationalUnitName)));

	m_ui->issuedon_label->setText(certificate.effectiveDate().toString());
	m_ui->expireson_label->setText(certificate.expiryDate().toString());

	m_ui->md5_label->setText(certificate.digest(QCryptographicHash::Md5).toHex());
	m_ui->sha1_label->setText(certificate.digest(QCryptographicHash::Sha1).toHex());
	m_ui->certificateText->setText(certificate.toText());
}

CertificateView::~CertificateView()
{}

}
