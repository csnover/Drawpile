// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef VIDEOEXPORTDIALOG_H
#define VIDEOEXPORTDIALOG_H

#include <QDialog>

class VideoExporter;
class Ui_VideoExport;

namespace dialogs {

class VideoExportDialog : public QDialog
{
	Q_OBJECT
public:
	explicit VideoExportDialog(QWidget *parent=nullptr);
	~VideoExportDialog();

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

	Ui_VideoExport *m_ui;
};

}

#endif // VIDEOEXPORTDIALOG_H
