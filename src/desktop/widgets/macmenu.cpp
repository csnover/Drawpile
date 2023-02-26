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
			.onTriggered(this, &MacMenu::joinSession)
		);

	// Window menu (Mac specific)
	_windowActions = new QActionGroup(this);
	connect(_windowActions, &QActionGroup::triggered, this, &MacMenu::winSelect);
	_windows = MainWindow::makeMenu(QT_TRANSLATE_NOOP("MainWindow", "&Window"), this)
		.onAboutToShow(this, &MacMenu::updateWinMenu)
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "Minimize"), "minimize", this)
			.shortcut("ctrl+m")
			.onTriggered(this, &MacMenu::winMinimize)
		)
		.action(MainWindow::makeAction(QT_TRANSLATE_NOOP("MainWindow", "Zoom"), "zoomwindow", this)
			.onTriggered(this, &MacMenu::winZoom)
		)
		.separator();

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

void MacMenu::joinSession()
{
	auto dlg = new dialogs::JoinDialog(QUrl());
	connect(dlg, &dialogs::JoinDialog::finished, [dlg](int i) {
		if(i==QDialog::Accepted) {
			QUrl url = dlg->getUrl();

			if(!url.isValid()) {
				// TODO add validator to prevent this from happening
				QMessageBox::warning(0, "Error", "Invalid address");
				return;
			}

			dlg->rememberSettings();

			MainWindow *mw = new MainWindow;
			mw->joinSession(url, dlg->autoRecordFilename());
		}
		dlg->deleteLater();
	});
	dlg->show();
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
		box.setText(MainWindow::tr("You have %n images with unsaved changes. Do you want to review these changes before quitting?", "", dirty));
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

void MacMenu::winMinimize()
{
	if(auto *w = qobject_cast<MainWindow *>(qApp->activeWindow()))
		w->showMinimized();
}

void MacMenu::winZoom()
{
	if(auto *w = qobject_cast<MainWindow *>(qApp->activeWindow())) {
		if (w->isMaximized()) {
			w->showNormal();
		} else {
			w->showMaximized();
		}
	}
}

void MacMenu::changeEvent(QEvent *event)
{
	QMenuBar::changeEvent(event);
	switch (event->type()) {
	case QEvent::LanguageChange:
		retranslateUi();
		break;
	default: {}
	}
}

static QString menuWinTitle(QString title)
{
	title.replace(QStringLiteral("[*]"), QString());
	return title.trimmed();
}

void MacMenu::addWindow(MainWindow *win)
{
	QAction *a = new QAction(menuWinTitle(win->windowTitle()), this);
	a->setProperty("mainwin", QVariant::fromValue(win));
	a->setCheckable(true);
	_windows->addAction(a);
	_windowActions->addAction(a);
}

void MacMenu::updateWindow(MainWindow *win)
{
	for (auto *a : _windowActions->actions()) {
		if(a->property("mainwin").value<MainWindow *>() == win) {
			a->setText(menuWinTitle(win->windowTitle()));
			break;
		}
	}
}

void MacMenu::removeWindow(MainWindow *win)
{
	for (auto *a : _windowActions->actions()) {
		if (a->property("mainwin").value<MainWindow *>() == win) {
			delete a;
			return;
		}
	}

	Q_ASSERT_X(false, __func__, "could not find window");
}

void MacMenu::winSelect(QAction *a)
{
	if (auto *w = a->property("mainwin").value<MainWindow *>()) {
		if (w->isMinimized()) {
			w->showNormal();
		}
		w->raise();
		w->activateWindow();
	}
}

void MacMenu::updateWinMenu()
{
	const MainWindow *top = qobject_cast<MainWindow *>(qApp->activeWindow());
	for (auto *a : _windowActions->actions()) {
		// TODO show bullet if window has unsaved changes and diamond
		// if minimized.
		auto *win = a->property("mainwin").value<MainWindow *>();
		auto text = menuWinTitle(win->windowTitle());
		if (win == top) {
			a->setChecked(true);
			a->setText(text);
		} else {
			a->setChecked(false);
			if (win->isMinimized()) {
				a->setText("♦︎ " + text);
			} else if (win->isWindowModified()) {
				a->setText("• " + text);
			} else {
				a->setText(text);
			}
		}
	}
}

void MacMenu::retranslateUi()
{
	MainWindow::retranslateMenuChildren(*this);
}
