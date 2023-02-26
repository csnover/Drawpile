// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef RESETSESSIONDIALOG_H
#define RESETSESSIONDIALOG_H

#include <QDialog>
#include <memory>

namespace canvas {
	class PaintEngine;
}

namespace net {
	class Envelope;
}

namespace dialogs {

// TODO: Retranslate UI
class ResetDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ResetDialog(const canvas::PaintEngine *pe, QWidget *parent=nullptr);
	~ResetDialog();

	void setCanReset(bool canReset);

	net::Envelope getResetImage() const;

signals:
	void resetSelected();
	void newSelected();

private slots:
	void onSelectionChanged(int pos);
	void onOpenClicked();

private:
	struct Private;
	std::unique_ptr<Private> d;
};

}

#endif
