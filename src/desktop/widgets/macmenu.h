// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef MACMENU_H
#define MACMENU_H

#include <QMenuBar>

class MainWindow;

class MacMenu : public QMenuBar
{
	Q_OBJECT
public:
	static MacMenu *instance();

	void updateRecentMenu();

	void addWindow(MainWindow *win);
	void removeWindow(MainWindow *win);
	void updateWindow(MainWindow *win);

	QMenu *windowMenu() { return _windows; }

signals:

public slots:
	void newDocument();
	void openDocument();
	void joinSession();
	void quitAll();

private slots:
	void openRecent(QAction *action);

	void winMinimize();
	void winSelect(QAction *a);
	void updateWinMenu();

private:
	MacMenu();
	QAction *makeAction(QMenu *menu, const char *name, const QString &text, const QKeySequence &shortcut);

	QMenu *_recent;
	QMenu *_windows;
};

#endif // MACMENU_H
