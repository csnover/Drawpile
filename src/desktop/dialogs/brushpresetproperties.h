// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: askmeaboutloom

#ifndef BRUSHPRESETPROPERTIES_H
#define BRUSHPRESETPROPERTIES_H

#include <QDialog>

class Ui_BrushPresetProperties;

namespace dialogs {

class BrushPresetProperties : public QDialog
{
Q_OBJECT
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
	Ui_BrushPresetProperties *m_ui;

	void showThumbnail(const QPixmap &thumbnail);
};

}

#endif
