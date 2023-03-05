// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef MACMENU_H
#define MACMENU_H

#include <QMenuBar>

class QAction;
class QEvent;
class QMenu;
class QString;
class MainWindow;

class MacMenu : public QMenuBar
{
	Q_OBJECT
public:
	static MacMenu *instance();

	void updateRecentMenu();

	QMenu *windowMenu() { return _windows; }

signals:

public slots:
	void newDocument();
	void openDocument();
	void joinSession();
	void quitAll();

private slots:
	void openRecent(QAction *action);

private:
	MacMenu();

	QMenu *_recent;
	QMenu *_windows;
	QActionGroup *_windowActions;
};

#endif // MACMENU_H
