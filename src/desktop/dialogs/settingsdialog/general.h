// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOP_DIALOGS_SETTINGSDIALOG_GENERAL_H
#define DESKTOP_DIALOGS_SETTINGSDIALOG_GENERAL_H

#include <QWidget>

namespace dialogs {
namespace settingsdialog {

class General final : public QWidget {
	Q_OBJECT
public:
	General(QWidget *parent = nullptr);
};

} // namespace settingsdialog
} // namespace dialogs

#endif
