// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/newdialog.h"
#include "libclient/utils/images.h"

#include "ui_newdialog.h"

#include <QPushButton>
#include <QSettings>
#include <QMessageBox>

namespace dialogs {

DP_DYNAMIC_DEFAULT_IMPL(NewDialog)

NewDialog::NewDialog(QWidget *parent)
	: DynamicUiWidget(parent)
{
	QSettings cfg;

	QSize lastSize = cfg.value("history/newsize", QSize(800, 600)).toSize();
	if(lastSize.isValid())
		setSize(lastSize);

	QColor lastColor = cfg.value("history/newcolor").value<QColor>();
	if(lastColor.isValid())
		setBackground(lastColor);

	m_ui->buttons->button(QDialogButtonBox::Ok)->setText(tr("Create"));
}

NewDialog::~NewDialog()
{}

void NewDialog::setSize(const QSize &size)
{
	m_ui->width->setValue(size.width());
	m_ui->height->setValue(size.height());

}

void NewDialog::setBackground(const QColor &color)
{
	m_ui->background->setColor(color);
}

void NewDialog::done(int r)
{
	if(r == QDialog::Accepted) {
		QSize size(m_ui->width->value(), m_ui->height->value());

		if(!utils::checkImageSize(size)) {
			QMessageBox::information(this, tr("Error"), tr("Size is too large"));
			return;
		} else {
			QSettings cfg;
			cfg.setValue("history/newsize", size);
			cfg.setValue("history/newcolor", m_ui->background->color());
			emit accepted(size, m_ui->background->color());
		}
	}

	QDialog::done(r);
}

}
