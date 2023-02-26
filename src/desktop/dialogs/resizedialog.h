// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef RESIZEDIALOG_H
#define RESIZEDIALOG_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>

class Ui_ResizeDialog;

namespace dialogs {

struct ResizeVector {
	int top, right, bottom, left;

	bool isZero() const {
		return top==0 && right==0 && bottom==0 && left==0;
	}
};

class ResizeDialog : public DynamicUiWidget<QDialog, Ui_ResizeDialog>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit ResizeDialog(const QSize &oldsize, QWidget *parent=nullptr);
	~ResizeDialog();

	void setPreviewImage(const QImage &image);
	void setBounds(const QRect &rect);

	QSize newSize() const;
	QPoint newOffset() const;
	ResizeVector resizeVector() const;

public slots:
	void done(int r) override;

private slots:
	void widthChanged(int);
	void heightChanged(int);
	void toggleAspectRatio(bool keep);
	void reset();

private:
	QSize m_oldsize;
	float m_aspectratio;
	int m_lastchanged;
	QPushButton *m_centerButton;
};

}

#endif
