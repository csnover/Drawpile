// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CERTIFICATEVIEW_H
#define CERTIFICATEVIEW_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>

class Ui_CertificateView;
class QSslCertificate;
class QString;

namespace dialogs {

class CertificateView : public DynamicUiWidget<QDialog, Ui_CertificateView>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	CertificateView(const QString &hostname, const QSslCertificate &certificate, QWidget *parent = 0);
	~CertificateView();
};

}

#endif // CERTIFICATEVIEW_H
