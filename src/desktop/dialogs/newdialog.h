// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>

class Ui_NewDialog;

namespace dialogs {

/**
 * @brief Dialog to set new drawing settings
 * The "new drawing" dialog allows the user to set the width, height
 * and background color of a new image.
 */
class NewDialog final : public DynamicUiWidget<QDialog, Ui_NewDialog>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	NewDialog(QWidget *parent=nullptr);
	~NewDialog() override;

	//! Set the width/height fields
	void setSize(const QSize &size);

	//! Set the background color field
	void setBackground(const QColor &color);

public slots:
	void done(int r) override;

signals:
	void accepted(const QSize &size, const QColor &background);
};

}

#endif
