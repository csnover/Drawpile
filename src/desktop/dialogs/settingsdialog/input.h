// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DESKTOP_DIALOGS_SETTINGSDIALOG_INPUT_H
#define DESKTOP_DIALOGS_SETTINGSDIALOG_INPUT_H

#include <QWidget>

class QFrame;
class QPaintEvent;
class QResizeEvent;
class QSpacerItem;

namespace dialogs {
namespace settingsdialog {

class Input final : public QWidget {
	Q_OBJECT
public:
	Input(QWidget *parent = nullptr);
protected:
	void resizeEvent(QResizeEvent *event) override;
private:
	QFrame *m_testerFrame;
	QSpacerItem *m_tabletSpacer;
};

} // namespace settingsdialog
} // namespace dialogs

#endif
