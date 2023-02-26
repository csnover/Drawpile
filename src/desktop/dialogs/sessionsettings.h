// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SESSIONSETTINGSDIALOG_H
#define SESSIONSETTINGSDIALOG_H

#include "desktop/utils/dynamicui.h"
#include "libclient/canvas/acl.h"

#include <QDialog>
#include <QJsonObject>
#include <QTimer>

class QStringListModel;
class QTimer;
class QLabel;
class QComboBox;

class Ui_SessionSettingsDialog;
class Document;

namespace canvas { class CanvasModel; }

namespace dialogs {

class SessionSettingsDialog : public DynamicUiWidget<QDialog, Ui_SessionSettingsDialog>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	SessionSettingsDialog(Document *doc, QWidget *parent=nullptr);
	~SessionSettingsDialog();

	//! Is persistence available at all on this server?
	void setPersistenceEnabled(bool);

	//! Is autoreset supported?
	void setAutoResetEnabled(bool);

	//! Is the local user authenticated/not a guest?
	void setAuthenticated(bool);

signals:
	void requestAnnouncement(const QString &apiUrl);
	void requestUnlisting(const QString &apiUrl);

private slots:
	void onCanvasChanged(canvas::CanvasModel*);
	void onOperatorModeChanged(bool op);
	void onFeatureTiersChanged(const rustpile::FeatureTiers &features);

	void permissionPresetSaving(const QString &);
	void permissionPresetSelected(const QString &);

	void titleChanged(const QString &newTitle);

	void maxUsersChanged();
	void denyJoinsChanged(bool);
	void authOnlyChanged(bool);

	void permissionChanged();

	void autoresetThresholdChanged();
	void keepChatChanged(bool);
	void persistenceChanged(bool);
	void nsfmChanged(bool);
	void deputiesChanged(int);

	void changePassword();
	void changeOpword();

	void changeSessionConf(const QString &key, const QJsonValue &value, bool now=false);
	void sendSessionConf();

	void updatePasswordLabel(QLabel *label);

protected:
	void showEvent(QShowEvent *event) override;

private:
	void initPermissionComboBoxes(bool retranslate);
	void reloadSettings();
	QComboBox *featureBox(canvas::Feature f);

	Document *m_doc;
	QTimer m_saveTimer;

	QJsonObject m_sessionconf;
	bool m_featureTiersChanged = false;

	bool m_op = false;
	bool m_isAuth = false;
	bool m_canPersist = false;
	bool m_canAutoreset = false;
};

}

#endif
