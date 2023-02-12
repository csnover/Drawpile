// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CERTIFICATEVIEW_H
#define CERTIFICATEVIEW_H

#include <QDialog>

class Ui_CertificateView;
class QSslCertificate;

namespace dialogs {

class CertificateView : public QDialog
{
	Q_OBJECT
public:
	CertificateView(const QString &hostname, const QSslCertificate &certificate, QWidget *parent = 0);
	~CertificateView();

private:
	Ui_CertificateView *_ui;

};

}

#endif // CERTIFICATEVIEW_H
