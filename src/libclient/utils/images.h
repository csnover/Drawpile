// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef IMAGESIZECHECK_H
#define IMAGESIZECHECK_H

class QSize;
class QImage;
class QColor;

#include <QVector>
#include <QPair>

namespace utils {

//! Check if image dimensions are not too big. Returns true if size is OK
bool checkImageSize(const QSize &size);

/**
 * @brief Check if we support writing an image file with this format
 *
 * The the format is identified from the filename suffix.
 * Note: only formats supported by the Rustpile library are included
 * in the list.
 */
bool isWritableFormat(const QString &filename);

enum FileFormatOption {
	Images = 0x01,
	Recordings = 0x02,
	AllFiles = 0x04,
	Save = 0x08,
	QtImagesOnly = 0x10,  // return images supported by Qt, rather than Rustpile

	OpenImages = Images | AllFiles,
	OpenEverything = Images | Recordings | AllFiles,
	SaveImages = Images | AllFiles | Save,
	SaveRecordings = Recordings | AllFiles | Save
};
Q_DECLARE_FLAGS(FileFormatOptions, FileFormatOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(FileFormatOptions)

//! Get a filter string to use in an Open or Save dialog
QString fileFormatFilter(FileFormatOptions formats);

}

#endif
