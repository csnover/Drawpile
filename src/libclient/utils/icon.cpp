// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/utils/icon.h"
#include "libshared/util/paths.h"

#include <QDir>
#include <QPalette>

namespace icon {

bool isDark(const QColor &c)
{
	const qreal luminance = c.redF() * 0.216 + c.greenF() * 0.7152 + c.blueF() * 0.0722;

	return luminance <= 0.5;
}

bool isDarkThemeSelected() { return isDark(QPalette().color(QPalette::Window)); }

void setThemeSearchPaths()
{
	static QStringList defaultThemePaths{QIcon::themeSearchPaths()};

	QStringList themePaths{defaultThemePaths};
	for (const auto &path : utils::paths::dataPaths()) {
		themePaths.append(path + "/theme");
	}

	QIcon::setThemeSearchPaths(themePaths);

	themePaths.clear();
	auto *theme = isDarkThemeSelected() ? "dark" : "light";
	for (const auto &path : utils::paths::dataPaths()) {
		themePaths.append(path + "/theme/" + theme);
	}

	QDir::setSearchPaths("theme", themePaths);
	QIcon::setThemeName(theme);
}

}
