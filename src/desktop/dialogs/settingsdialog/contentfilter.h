// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_DIALOGS_SETTINGSDIALOG_CONTENTFILTER_H
#define DESKTOP_DIALOGS_SETTINGSDIALOG_CONTENTFILTER_H

#include <QWidget>

class QVBoxLayout;

namespace desktop { namespace settings { class Settings; } }

namespace dialogs {
namespace settingsdialog {

class ContentFilter final : public QWidget {
	Q_OBJECT
public:
	ContentFilter(desktop::settings::Settings &settings, QWidget *parent = nullptr);

private:
	void initBuiltIn(desktop::settings::Settings &settings, QVBoxLayout *layout);
	void initInfoBar(QVBoxLayout *layout);
	void initOsManaged(QVBoxLayout *layout);
	void toggleLock();

	desktop::settings::Settings &m_settings;
};

} // namespace settingsdialog
} // namespace dialogs

#endif
