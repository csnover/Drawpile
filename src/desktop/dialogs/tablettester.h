// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TABLETTESTDIALOG_H
#define TABLETTESTDIALOG_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>

class Ui_TabletTest;

namespace dialogs {

class TabletTestDialog : public DynamicUiWidget<QDialog, Ui_TabletTest>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	TabletTestDialog(QWidget *parent=nullptr);
	~TabletTestDialog();
};

}

#endif
