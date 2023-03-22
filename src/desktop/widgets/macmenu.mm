// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/main.h"
#include "desktop/widgets/macmenu.h"
#include "desktop/mainwindow.h"
#include "desktop/utils/actionbuilder.h"
#include "desktop/utils/recentfiles.h"
#include "desktop/dialogs/newdialog.h"
#include "desktop/dialogs/hostdialog.h"
#include "desktop/dialogs/joindialog.h"
#include "libshared/util/qtcompat.h"

#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QMessageBox>
#include <QUrl>

#import <Cocoa/Cocoa.h>

MacMenu *MacMenu::instance()
{
	static MacMenu *menu;
	if(!menu)
		menu = new MacMenu;
	return menu;
}

MacMenu::MacMenu()
	: QMenuBar(nullptr)
{
	// File menu
	MainWindow::makeMenu(QT_TRANSLATE_NOOP("MainWindow", "&File"), this)
		.objectName("filemenu")
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "&New"), "newdocument", this)
			.shortcut(QKeySequence::New)
			.onTriggered(this, &MacMenu::newDocument)
		)
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "&Open..."), "opendocument", this)
			.shortcut(QKeySequence::Open)
			.onTriggered(this, &MacMenu::openDocument)
		)
		.submenu([=](MenuBuilder menu) {
			_recent = menu
				.title(QT_TRANSLATE_NOOP("MainWindow", "Open &Recent"))
				.onTriggered(this, &MacMenu::openRecent);
		})
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "&Quit"), "exitprogram", this)
			.shortcut("Ctrl+Q")
			.menuRole(QAction::QuitRole)
			.onTriggered(this, &MacMenu::quitAll)
		)
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "Prefere&nces"), "preferences", this)
			.menuRole(QAction::PreferencesRole)
			.onTriggered(&MainWindow::showSettings)
		);

	// Session menu
	MainWindow::makeMenu(QT_TRANSLATE_NOOP("MainWindow", "&Session"), this)
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "&Host..."), "hostsession", this)
			.disabled()
		)
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "&Join..."), "joinsession", this)
			.onTriggered([] { MainWindow::showJoinDialog(nullptr); })
		);

	// Window menu (Mac specific)
	_windows = MainWindow::makeMenu(QT_TRANSLATE_NOOP("MainWindow", "&Window"), this);
	auto *nativeMenu = _windows->toNSMenu();
	[nativeMenu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
	[nativeMenu addItemWithTitle:@"Zoom" action:@selector(performZoom:) keyEquivalent:@""];
	[[NSApplication sharedApplication] setWindowsMenu:nativeMenu];

	MainWindow::makeMenu(QT_TRANSLATE_NOOP("MainWindow", "&Help"), this)
		.objectName("helpmenu")
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "&Homepage"), "dphomepage", this)
			.onTriggered(&MainWindow::homepage)
		)
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "&About Drawpile"), "dpabout", this)
			.menuRole(QAction::AboutRole)
			.onTriggered(&MainWindow::about)
		)
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "About &Qt"), "aboutqt", this)
			.menuRole(QAction::AboutQtRole)
			.onTriggered(&QApplication::aboutQt)
		);

	updateRecentMenu();
}

void MacMenu::updateRecentMenu()
{
	RecentFiles::initMenu(_recent);
}

void MacMenu::newDocument()
{
	auto dlg = new dialogs::NewDialog;
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	connect(dlg, &dialogs::NewDialog::accepted, [](const QSize &size, const QColor &color) {
		MainWindow *mw = new MainWindow;
		mw->newDocument(size, color);
	});

	dlg->show();
}

void MacMenu::openDocument()
{
	MainWindow *mw = new MainWindow;
	mw->open();
}

void MacMenu::openRecent(QAction *action)
{
	MainWindow *mw = new MainWindow;
	mw->open(QUrl::fromLocalFile(action->property("filepath").toString()));
}

/**
 * @brief Quit program, closing all main windows
 *
 * This is currently used only on OSX because of the global menu bar.
 * On other platforms, there may be windows belonging to different processes open,
 * so shutting down the whole process when Quit was chosen from one window may
 * result in inconsistent operation.
 */
void MacMenu::quitAll()
{
	int mainwindows = 0;
	int dirty = 0;
	bool forceDiscard = false;

	for(const QWidget *widget : qApp->topLevelWidgets()) {
		const MainWindow *mw = qobject_cast<const MainWindow*>(widget);
		if(mw) {
			++mainwindows;
			if(!mw->canReplace())
				++dirty;
		}
	}

	if(mainwindows==0) {
		qApp->quit();
		return;
	}

	if(dirty>1) {
		QMessageBox box;
		box.setText(MainWindow::tr("You have %Ln images with unsaved changes. Do you want to review these changes before quitting?", "", dirty));
		box.setInformativeText(MainWindow::tr("If you don't review your documents, all changes will be lost"));
		box.addButton(MainWindow::tr("Review changes..."), QMessageBox::AcceptRole);
		box.addButton(QMessageBox::Cancel);
		box.addButton(MainWindow::tr("Discard changes"), QMessageBox::DestructiveRole);

		int r = box.exec();

		if(r == QMessageBox::Cancel)
			return;
		else if(r == 1)
			forceDiscard = true;
	}

	qApp->setQuitOnLastWindowClosed(true);

	if(forceDiscard) {
		for(QWidget *widget : qApp->topLevelWidgets()) {
			MainWindow *mw = qobject_cast<MainWindow*>(widget);
			if(mw)
				mw->exit();
		}

	} else {
		qApp->closeAllWindows();
		bool allClosed = true;
		for(QWidget *widget : qApp->topLevelWidgets()) {
			MainWindow *mw = qobject_cast<MainWindow*>(widget);
			if(mw) {
				allClosed = false;
				break;
			}
		}
		if(!allClosed) {
			// user cancelled quit
			qApp->setQuitOnLastWindowClosed(false);
		}
	}
}
