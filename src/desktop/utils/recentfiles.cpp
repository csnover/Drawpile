// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include <QStringList>
#include <QFileInfo>
#include <QSettings>
#include <QMenu>

#include "desktop/utils/recentfiles.h"

/**
 * @param filename filename to add
 */
void RecentFiles::addFile(const QString& filename)
{
	const int maxrecent = getMaxFileCount();

	QSettings cfg;
	cfg.beginGroup("history");

	QStringList files = cfg.value("recentfiles").toStringList();
	files.removeAll(filename);
	files.prepend(filename);
	while (files.size() > maxrecent)
		files.removeLast();

	cfg.setValue("recentfiles", files);

}

/**
 * @param max maximum number of filenames that can be stored
 */
void RecentFiles::setMaxFileCount(int max)
{
	QSettings cfg;
	cfg.setValue("history/maxrecentfiles",max);
}

/**
 * @return maximum number of filenames stored
 */
int RecentFiles::getMaxFileCount()
{
	QSettings cfg;
	int maxrecent = cfg.value("history/maxrecentfiles").toInt();
	if(maxrecent<=0)
		maxrecent = DEFAULT_MAXFILES;
	return maxrecent;
}

/**
 * The full path is stored in the property "filepath".
 * If the list of recent files is empty, the menu is disabled. Actions in
 * the menu marked with the attribute "deletelater" are deleted later in
 * the event loop instead of immediately.
 * @param menu QMenu to fill
 */
void RecentFiles::initMenu(QMenu *menu)
{
	QSettings cfg;
	const QStringList files = cfg.value("history/recentfiles").toStringList();

	const QList<QAction*> actions = menu->actions();
	for(QAction *act : actions) {
		menu->removeAction(act);
		act->deleteLater();
	}

	// Generate menu contents
	menu->setEnabled(!files.isEmpty());
	int index = 1;
	for(const QString &filename : files) {
		const QFileInfo file(filename);
		QAction *a = menu->addAction(QString(index<10?"&%1. %2":"%1. %2").arg(index).arg(file.fileName()));
		a->setStatusTip(file.absoluteFilePath());
		a->setProperty("filepath",file.absoluteFilePath());
		++index;
	}
}
