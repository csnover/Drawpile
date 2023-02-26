// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef InputSettings_H
#define InputSettings_H

#include "desktop/utils/dynamicui.h"
#include "libclient/canvas/pressure.h"

#include <QDialog>
#include <QMenu>

class Ui_InputSettings;

namespace input {
	class PresetModel;
	struct Preset;
}

namespace dialogs {

class InputSettings : public DynamicUiWidget<QDialog, Ui_InputSettings>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit InputSettings(QWidget *parent=nullptr);
	~InputSettings();

	void setCurrentPreset(const QString &id);

signals:
	void currentIndexChanged(int index);
	void activePresetModified(const input::Preset *preset);

public slots:
	void setCurrentIndex(int index);

private slots:
	void choosePreset(int index);
	void presetNameChanged(const QString &name);
	void addPreset();
	void copyPreset();
	void removePreset();
	void onPresetCountChanged();

private:
	void applyPresetToUi(const input::Preset &preset);
	void applyUiToPreset();
	void updateModeUi(PressureMapping::Mode mode);

	input::PresetModel *m_presetModel;
	bool m_updateInProgress;
	bool m_indexChangeInProgress;
	QMenu *m_presetmenu;
	QAction *m_newPresetAction;
	QAction *m_duplicatePresetAction;
	QAction *m_removePresetAction;
};

}

#endif
