// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOP_DIALOGS_SETTINGSDIALOG_TOOLS_H
#define DESKTOP_DIALOGS_SETTINGSDIALOG_TOOLS_H

#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;

namespace dialogs {
namespace settingsdialog {

class Tools final : public QWidget {
	Q_OBJECT
public:
	Tools(QWidget *parent = nullptr);
protected:
	void paintEvent(QPaintEvent *event) override;
private:
	void initGeneralTools(QVBoxLayout *layout);
	void initColorWheel(QVBoxLayout *layout);

	QHBoxLayout *m_fart;
};

} // namespace settingsdialog
} // namespace dialogs

#endif
