// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef VERSIONCHECKDIALOG_H
#define VERSIONCHECKDIALOG_H

#include "desktop/utils/dynamicui.h"
#include "libclient/utils/newversion.h"

#include <QDialog>

class Ui_VersionCheckDialog;
class QPushButton;

namespace dialogs {

class VersionCheckDialog : public DynamicUiWidget<QDialog, Ui_VersionCheckDialog>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit VersionCheckDialog(QWidget *parent=nullptr);
	~VersionCheckDialog();

	/**
	 * @brief Perform version check (if needed) and show dialog if new version is available
	 */
	static void doVersionCheckIfNeeded();

	/**
	 * @brief Set the list of new versions available
	 *
	 * If the list is empty, a "no new versions" message is shown instead.
	 *
	 * @param versions
	 */
	void setNewVersions(const QVector<NewVersionCheck::Version> &versions);

	//! Check for new versions and update the dialog's content based on the results
	void queryNewVersions();

private slots:
	void versionChecked(bool isNew, const QString &errorMessage);
	void rememberSettings();
	void downloadNewVersion();

private:
	NewVersionCheck *m_newversion;
	QPushButton *m_downloadButton;

	QUrl m_downloadUrl;
	QByteArray m_downloadSha256;
	int m_downloadSize;
};

}

#endif // VERSIONCHECKDIALOG_H
