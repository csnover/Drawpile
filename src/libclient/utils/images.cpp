// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "config.h"
#include "libclient/utils/images.h"

#include <QSize>
#include <QImageReader>
#include <QImageWriter>
#include <QCoreApplication>

namespace utils {

//! Check if image dimensions are not too big. Returns true if size is OK
bool checkImageSize(const QSize &size)
{
	// The protocol limits width and height to 2^29 (PenMove, 2^32 for others)
	// However, QPixmap can be at most 32767 pixels wide/tall
	static const int MAX_SIZE = 32767;

	return
		size.width() <= MAX_SIZE &&
		size.height() <= MAX_SIZE;
}

static QVector<QPair<QString,QByteArray>> writableImageFormats()
{
	static QVector<QPair<QString,QByteArray>> formats;

	if(formats.isEmpty()) {
		// List of formats supported by Rustpile
		// (See the image crate's features in dpimpex/Cargo.toml)
		formats
			<< QPair<QString,QByteArray>("OpenRaster", "ora")
			<< QPair<QString,QByteArray>("JPEG", "jpeg")
			<< QPair<QString,QByteArray>("PNG", "png")
			<< QPair<QString,QByteArray>("GIF", "gif")
			;
	}

	return formats;
}

bool isWritableFormat(const QString &filename)
{
	const int dot = filename.lastIndexOf('.');
	if(dot<0)
		return false;
	const QByteArray suffix = filename.mid(dot+1).toLower().toLatin1();

	const auto writableFormats = writableImageFormats();
	for(const auto &pair : writableFormats) {
		if(suffix == pair.second)
			return true;
	}

	return false;
}

QString fileFormatFilter(FileFormatOptions formats)
{
	QStringList filter;
	QString readImages, recordings;

	if(formats.testFlag(FileFormatOption::Images)) {
		if(formats.testFlag(FileFormatOption::Save)) {
			if(formats.testFlag(FileFormatOption::QtImagesOnly)) {
				for(const QByteArray &fmt : QImageWriter::supportedImageFormats()) {
					filter << QStringLiteral("%1 (*.%2)").arg(QString::fromLatin1(fmt.toUpper()), QString::fromLatin1(fmt));
				}

			} else {
				for(const auto &format : utils::writableImageFormats()) {
					filter << QStringLiteral("%1 (*.%2)").arg(format.first, QString::fromLatin1(format.second));
				}
			}

		} else {
			// A single Images filter for loading
			if(formats.testFlag(FileFormatOption::QtImagesOnly)) {
				for(QByteArray format : QImageReader::supportedImageFormats()) {
					readImages += "*." + format + " ";
				}
			} else {
				// Formats supported by Rustpile
				readImages = DRAWPILE_FILE_GROUP_IMAGE;
			}

			filter << QCoreApplication::translate("FileFormatOptions", "Images (%1)").arg(readImages);
		}
	}

	if(formats.testFlag(FileFormatOption::Recordings)) {
		if(formats.testFlag(FileFormatOption::Save)) {
			// Recording formats individually for saving
			filter
				<< QCoreApplication::translate("FileFormatOptions", "Binary Recordings (%1)").arg("*.dprec")
				<< QCoreApplication::translate("FileFormatOptions", "Text Recordings (%1)").arg("*.dptxt")
				;

		} else {
			// A single Recordings filter for loading
			recordings = DRAWPILE_FILE_GROUP_RECORDING;
			filter
				<< QCoreApplication::translate("FileFormatOptions", "Recordings (%1)").arg(recordings)
				;
		}
	}

	if(!readImages.isEmpty() && !recordings.isEmpty()) {
		filter.prepend(
			QCoreApplication::translate("FileFormatOptions", "All Supported Files (%1)").arg(readImages + ' ' + recordings)
		);
	}

	// An all files filter when requested
	if(formats.testFlag(FileFormatOption::AllFiles)) {
		filter << QCoreApplication::translate("QFileDialog", "All Files (*)");
	}

	return filter.join(";;");
}

}
