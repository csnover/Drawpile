// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>

class Ui_NewDialog;

namespace dialogs {

/**
 * @brief Dialog to set new drawing settings
 * The "new drawing" dialog allows the user to set the width, height
 * and background color of a new image.
 */
class NewDialog : public QDialog
{
Q_OBJECT
public:
	NewDialog(QWidget *parent=0);
	~NewDialog();

	//! Set the width/height fields
	void setSize(const QSize &size);

	//! Set the background color field
	void setBackground(const QColor &color);

public slots:
	void done(int r);

signals:
	void accepted(const QSize &size, const QColor &background);

private:
	Ui_NewDialog *_ui;
};

}

#endif
