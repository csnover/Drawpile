// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>
#include <QSslCertificate>
#include <QMap>

class Ui_SettingsDialog;
class QListWidgetItem;
class CustomShortcutModel;
class AvatarListModel;

namespace sessionlisting { class ListServerModel; }

namespace color_widgets { class ColorWheel; }

namespace dialogs {

class SettingsDialog final : public DynamicUiWidget<QDialog, Ui_SettingsDialog>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	SettingsDialog(QWidget *parent=nullptr);
	~SettingsDialog() override;

private slots:
	void resetSettings();
	void rememberSettings();
	void saveCertTrustChanges();

	void viewCertificate(QListWidgetItem *item);
	void markTrustedCertificates();
	void removeCertificates();
	void certificateSelectionChanged();
	void importTrustedCertificate();

	void addListingServer();
	void removeListingServer();
	void moveListingServerUp();
	void moveListingServerDown();

	void lockParentalControls();

	void addAvatar();
	void removeSelectedAvatar();

	void changeColorWheelShape(int index);
	void changeColorWheelAngle(int index);
	void changeColorWheelSpace(int index);

private:
	void restoreSettings();
	void setParentalControlsLocked(bool lock);
	void rememberPcLevel();

	QStringList m_removeCerts;
	QStringList m_trustCerts;
	QList<QSslCertificate> m_importCerts;
	CustomShortcutModel *m_customShortcuts;
	sessionlisting::ListServerModel *m_listservers;
	AvatarListModel *m_avatars;

	Translator<int> m_volumeLabelText;
	Translator<bool> m_nsfmLockText;
};

}

#endif
