// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef IMAGESERIESEXPORTER_H
#define IMAGESERIESEXPORTER_H

#include "libclient/export/videoexporter.h"

class ImageSeriesExporter : public VideoExporter
{
	Q_OBJECT
public:
	ImageSeriesExporter(QObject *parent=0);

	void setOutputPath(const QString &path) { _path = path; }
	void setFilePattern(const QString &pattern) { _filepattern = pattern; }
	void setFormat(const QString &format) { _format = format.toLatin1(); }

protected:
	void initExporter();
	void writeFrame(const QImage &image, int repeat);
	void shutdownExporter();
	bool variableSizeSupported() { return true; }

private:
	QString _path;
	QString _filepattern;
	QByteArray _format;
};

#endif // IMAGESERIESEXPORTER_H
