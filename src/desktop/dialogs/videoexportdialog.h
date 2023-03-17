// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef VIDEOEXPORTDIALOG_H
#define VIDEOEXPORTDIALOG_H

#include "desktop/utils/dynamicui.h"
#include <QDialog>

class VideoExporter;
class Ui_VideoExport;

namespace dialogs {

class VideoExportDialog final : public DynamicUiWidget<QDialog, Ui_VideoExport>
{
	Q_OBJECT
	DP_DYNAMIC_UI
public:
	explicit VideoExportDialog(QWidget *parent=nullptr);
	~VideoExportDialog() override;

	/**
	 * @brief Get the new video exporter configured in this dialog
	 * @return exporter or nullptr if none was configured
	 */
	VideoExporter *getExporter();

private slots:
	void updateFfmpegArgumentPreview();

private:
	VideoExporter *getImageSeriesExporter();
	VideoExporter *getFfmpegExporter();
};

}

#endif // VIDEOEXPORTDIALOG_H
