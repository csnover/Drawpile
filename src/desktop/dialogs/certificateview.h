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

class CertificateView final : public DynamicUiWidget<QDialog, Ui_CertificateView>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	CertificateView(const QString &hostname, const QSslCertificate &certificate, QWidget *parent = nullptr);
	~CertificateView() override;
};

}

#endif // CERTIFICATEVIEW_H
