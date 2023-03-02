// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MACMENU_H
#define MACMENU_H

#include <QMenuBar>

class MainWindow;

class MacMenu final : public QMenuBar
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
	QAction *makeAction(QMenu *menu, const char *name, const QString &text, const QKeySequence &shortcut);

	QMenu *_recent;
	QMenu *_windows;
};

#endif // MACMENU_H
