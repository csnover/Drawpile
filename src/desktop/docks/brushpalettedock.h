// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen, askmeaboutloom

#ifndef BRUSHPALETTEDOCK_H
#define BRUSHPALETTEDOCK_H

#include "libclient/tools/tool.h"

#include <QDockWidget>
#include <memory>

namespace tools {
	class ToolProperties;
	class ToolSettings;
}

namespace docks {

/**
 * @brief A brush palette dock
 *
 * This dock displays a list of brush presets to choose from.
 */
class BrushPalette : public QDockWidget
{
	Q_OBJECT
public:
	BrushPalette(QWidget *parent=nullptr);
	~BrushPalette();

	void connectBrushSettings(tools::ToolSettings *toolSettings);

private slots:
	void tagIndexChanged(int proxyRow);
	void presetsReset();
	void presetSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
	void newTag();
	void editCurrentTag();
	void deleteCurrentTag();
	void newPreset();
	void duplicateCurrentPreset();
	void overwriteCurrentPreset();
	void editCurrentPreset();
	void deleteCurrentPreset();
	void importMyPaintBrushes();
	void applyPresetProperties(int id, const QString &name, const QString &description,
		const QPixmap &thumbnail);
	void applyToBrushSettings(const QModelIndex &index);
	void showPresetContextMenu(const QPoint &pos);

private:
	struct Private;
	std::unique_ptr<Private> d;

	void changeTagAssignment(int tagId, bool assigned);

	int tagIdToProxyRow(int tagId);

	int presetRowToSource(int proxyRow);
	int presetRowToProxy(int sourceRow);
	QModelIndex presetIndexToSource(const QModelIndex &proxyIndex);
	QModelIndex presetIndexToProxy(const QModelIndex &sourceIndex);
	int presetProxyIndexToId(const QModelIndex &proxyIndex);
	int currentPresetId();
};

}

#endif
