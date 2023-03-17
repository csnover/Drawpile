// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef PAINTCORE_BLENDMODES_H
#define PAINTCORE_BLENDMODES_H

#include <QString>
#include <QVector>
#include <QPair>

namespace rustpile {
	enum class Blendmode : uint8_t;
}

namespace canvas {
namespace blendmode {
	QString tr(const char *key, const char *disambiguation = nullptr, int n = -1);

	//! Get the SVG name for the given blend mode
	QString svgName(rustpile::Blendmode mode);

	//! Find a blend mode by its SVG name
	rustpile::Blendmode fromSvgName(const QString &name);

	//! Get a list of (brush) blend modes and their translated names
	QVector<QPair<rustpile::Blendmode, QString>> brushModeNames();

	//! Get a list of (layer) blend modes and their translated names
	QVector<QPair<rustpile::Blendmode, const char *>> layerModeNames();
}
}

#endif
