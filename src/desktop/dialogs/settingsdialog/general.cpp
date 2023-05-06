// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmake-config/config.h"
#include "desktop/dialogs/settingsdialog/general.h"
#include "desktop/settings.h"
#include "desktop/utils/sanerformlayout.h"

#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QString>
#include <QStyleFactory>
#include <QWidget>

namespace dialogs {
namespace settingsdialog {

General::General(desktop::settings::Settings &settings, QWidget *parent)
	: QWidget(parent)
{
	auto *form = new utils::SanerFormLayout(this);

	initTheme(settings, form);
	initLanguage(settings, form);
	form->addSeparator();
	initMiscUi(settings, form);
	form->addSpacer();
	initRecording(settings, form);
	form->addSeparator();
	initUndo(settings, form);
	form->addSpacer();
	initSnapshots(settings, form);
	form->addSeparator();
	initHistory(settings, form);
}

void General::initHistory(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *maxRecent = new QSpinBox;
	maxRecent->setRange(0, 20);
	settings.bindMaxRecentFiles(maxRecent);
	auto *maxRecentLayout = utils::encapsulate(tr("Remember the last %1 opened files and hosts"), maxRecent);
	form->addRow(tr("Recent items:"), maxRecentLayout);
}

void General::initLanguage(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *language = new QComboBox;
	language->addItem(tr("System"), QString());

	const QLocale localeC = QLocale::c();
	for (const auto *localeName : cmake_config::locales()) {
		QLocale locale(localeName);
		if (locale != localeC) {
			language->addItem(
				tr("%1 (%2)")
					.arg(locale.nativeLanguageName())
					.arg(QLocale::languageToString(locale.language())),
				localeName
			);
		}
	}

	settings.bindLanguage(language, Qt::UserRole);
	form->addRow(tr("Language:"), language);
}

void General::initMiscUi(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *scrollBars = new QCheckBox(tr("Show scroll bars on canvas"));
	settings.bindCanvasScrollBars(scrollBars);
	form->addRow(tr("User interface:"), scrollBars);

	auto *confirmDelete = new QCheckBox(tr("Ask before deleting layers"));
	settings.bindConfirmLayerDelete(confirmDelete);
	form->addRow(nullptr, confirmDelete);

	form->addSeparator();

	auto *enableLogging = new QCheckBox(tr("Write debugging log to file"));
	settings.bindWriteLogFile(enableLogging);
	form->addRow(tr("Logging:"), enableLogging);
}

void General::initRecording(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *recordMinimumPause = new QDoubleSpinBox;
	recordMinimumPause->setRange(0.03, 60);
	recordMinimumPause->setSingleStep(0.1);
	recordMinimumPause->setDecimals(2);
	settings.bindRecordMinimumPause(recordMinimumPause);

	auto *recordPausesLayout = utils::encapsulate(tr("Record pauses longer than %1 seconds"), recordMinimumPause);

	auto *recordPauses = utils::checkable(tr("Enable pause recording"), recordPausesLayout, recordMinimumPause);
	settings.bindRecordPause(recordPauses);

	form->addRow(tr("Session recording:"), recordPausesLayout);

	auto *recordTimestampInterval = new QSpinBox;
	recordTimestampInterval->setRange(1, 1440);
	recordTimestampInterval->setSingleStep(5);
	settings.bindRecordTimestampInterval(recordTimestampInterval);

	auto *recordTimestampLayout = utils::encapsulate(tr("Add timestamps every %1 minutes"), recordTimestampInterval);

	auto *recordTimestamp = utils::checkable(tr("Enable timestamp recording"), recordTimestampLayout, recordTimestampInterval);
	settings.bindRecordTimestamp(recordTimestamp);

	form->addRow(nullptr, recordTimestampLayout);
}

void General::initSnapshots(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *snapshotCount = new QSpinBox;
	snapshotCount->setRange(0, 99);
	settings.bindEngineSnapshotCount(snapshotCount);
	auto *snapshotCountLayout = utils::encapsulate(tr("Keep %1 canvas snapshots"), snapshotCount);
	snapshotCountLayout->setControlTypes(QSizePolicy::CheckBox);
	form->addRow(tr("Canvas snapshots:"), snapshotCountLayout);

	auto *snapshotInterval = new QSpinBox;
	snapshotInterval->setRange(1, 600);
	snapshotInterval->setSingleStep(5);
	settings.bindEngineSnapshotInterval(snapshotInterval);
	settings.bindEngineSnapshotCount(snapshotInterval, &QSpinBox::setEnabled);
	auto *snapshotIntervalLayout = utils::encapsulate(tr("Take one snapshot every %1 seconds"), snapshotInterval);
	snapshotIntervalLayout->setControlTypes(QSizePolicy::CheckBox);
	form->addRow(nullptr, snapshotIntervalLayout);

	form->addRow(nullptr, utils::note(tr("Snapshots are stored separately from the undo history in the <i>Session ▸ Reset…</i> menu."), QSizePolicy::Label));
}

void General::initTheme(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *style = new QComboBox;
	style->addItems(QStyleFactory::keys());
	settings.bindThemeStyle(style, Qt::DisplayRole);
	form->addRow(tr("Style:"), style);

	auto *theme = new QComboBox;
	theme->addItems({
		tr("System"),
		tr("Light"),
		tr("Dark"),
		tr("Krita Bright"),
		tr("Krita Dark"),
		tr("Krita Darker"),
		tr("Hotdog Stand")
	});
	settings.bindThemePalette(theme, std::nullopt);
	form->addRow(tr("Color scheme:"), theme);
}

void General::initUndo(desktop::settings::Settings &settings, utils::SanerFormLayout *form)
{
	auto *undoLimit = new QSpinBox;
	undoLimit->setRange(1, 255);
	settings.bindEngineUndoDepth(undoLimit);
	auto *undoLimitLayout = utils::encapsulate(tr("Use %1 undo levels by default"), undoLimit);
	undoLimitLayout->setControlTypes(QSizePolicy::CheckBox);
	form->addRow(tr("Session history:"), undoLimitLayout);
}

} // namespace settingsdialog
} // namespace dialogs
