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

	void addWindow(MainWindow *win);
	void removeWindow(MainWindow *win);
	void updateWindow(MainWindow *win);

	QMenu *windowMenu() { return _windows; }

	void changeEvent(QEvent *event) override;

signals:

public slots:
	void newDocument();
	void openDocument();
	void joinSession();
	void quitAll();

private slots:
	void openRecent(QAction *action);

	void winMinimize();
	void winZoom();
	void winSelect(QAction *a);
	void updateWinMenu();

private:
	MacMenu();
	void retranslateUi();

	QMenu *_recent;
	QMenu *_windows;
	QActionGroup *_windowActions;
};

#endif // MACMENU_H
