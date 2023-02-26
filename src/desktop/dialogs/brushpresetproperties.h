// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: askmeaboutloom

#ifndef BRUSHPRESETPROPERTIES_H
#define BRUSHPRESETPROPERTIES_H

#include "desktop/utils/dynamicui.h"

#include <QDialog>

class Ui_BrushPresetProperties;

namespace dialogs {

class BrushPresetProperties : public DynamicUiWidget<QDialog, Ui_BrushPresetProperties>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit BrushPresetProperties(int id, const QString &name, const QString &description,
		const QPixmap &thumbnail, QWidget *parent = nullptr);

	virtual ~BrushPresetProperties();

signals:
	void presetPropertiesApplied(int id, const QString &name, const QString &description,
		const QPixmap &thumbnail);

private slots:
	void chooseThumbnailFile();
	void emitChanges();

private:
	int m_id;
	QPixmap m_thumbnail;

	void showThumbnail(const QPixmap &thumbnail);
};

}

#endif
