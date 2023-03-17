// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef HOSTDIALOG_H
#define HOSTDIALOG_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>

class Ui_HostDialog;

namespace dialogs {

class HostDialog final : public DynamicUiWidget<QDialog, Ui_HostDialog>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit HostDialog(QWidget *parent=nullptr);
	~HostDialog() override;

	//! Store settings in configuration file
	void rememberSettings() const;

	//! Get the remote host address
	//! Returns an empty string if remote address is not used
	QString getRemoteAddress() const;

	//! Get session title
	QString getTitle() const;

	//! Get session password
	QString getPassword() const;

	//! Get the desired session alias
	QString getSessionAlias() const;

	//! Get the announcement server URL (empty if not selected)
	QString getAnnouncementUrl() const;

	//! Make a private (room code only) announcement instead of a public one?
	bool getAnnouncmentPrivate() const;

private slots:
	void updateListingPermissions();
};

}

#endif
