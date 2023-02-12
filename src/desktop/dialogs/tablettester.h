// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TABLETTESTDIALOG_H
#define TABLETTESTDIALOG_H

#include <QDialog>

class Ui_TabletTest;

namespace dialogs {

class TabletTestDialog : public QDialog
{
	Q_OBJECT
public:
	TabletTestDialog(QWidget *parent=nullptr);
	~TabletTestDialog();

private:
	Ui_TabletTest *m_ui;

};

}

#endif
