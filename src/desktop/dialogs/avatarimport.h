// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef AVATARIMPORT_H
#define AVATARIMPORT_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>
#include <QPointer>

class Ui_AvatarImport;

class QImage;
class AvatarListModel;

namespace dialogs {

class AvatarImport final : public DynamicUiWidget<QDialog, Ui_AvatarImport>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	AvatarImport(const QImage &source, QWidget *parent=nullptr);
	~AvatarImport() override;

	// Size of the final avatar image
	static const int Size = 32;

	QImage croppedAvatar() const;

	static void importAvatar(AvatarListModel *avatarList, QPointer<QWidget> parentWindow=QPointer<QWidget>());

private:
	QImage m_originalImage;
};

}

#endif
