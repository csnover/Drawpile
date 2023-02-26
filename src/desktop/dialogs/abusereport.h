// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef ABUSEREPORTDIALOG_H
#define ABUSEREPORTDIALOG_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>
#include <memory>

class Ui_AbuseReportDialog;

namespace dialogs {

class AbuseReportDialog : public DynamicUiWidget<QDialog, Ui_AbuseReportDialog>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit AbuseReportDialog(QWidget *parent=nullptr);
	~AbuseReportDialog();

	/**
	 * @brief Set the info about the session
	 * @param id
	 * @param alias
	 * @param title
	 */
	void setSessionInfo(const QString &id, const QString &alias, const QString &title);

	/**
	 * @brief Add a user to the selection box
	 *
	 * @param id
	 * @param name
	 */
	void addUser(int id, const QString &name);

	/**
	 * @brief Get the ID of the selected user.
	 *
	 * If no user was selected, the report is about the session in
	 * general.
	 * @return user Id or 0 if none was selected
	 */
	int userId() const;

	//! Get the reason message
	QString message() const;

private:
	QString m_sessionId;
	int m_userId;
};

}

#endif
