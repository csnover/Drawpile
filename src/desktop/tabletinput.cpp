// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/main.h"
#include "desktop/tabletinput.h"
#include <QApplication>
#include <QSettings>

#if defined(Q_OS_WIN)
#	if defined(HAVE_KIS_TABLET)
#		include "bundled/kis_tablet/kis_tablet_support_win.h"
#		include "bundled/kis_tablet/kis_tablet_support_win8.h"
#	elif QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// 		See qtbase tests/manual/qtabletevent/regular_widgets/main.cpp
#		include <QtGui/private/qguiapplication_p.h>
#		include <QtGui/qpa/qplatformintegration.h>
#	endif
#endif

namespace tabletinput {

static const char *inputMode = "Qt tablet input";

#ifdef Q_OS_WIN
static void updateWindowsInk(bool enable)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	using QWindowsApplication = QNativeInterface::Private::QWindowsApplication;
	if (auto *wa = qApp->nativeInterface<QWindowsApplication>()) {
		wa->setWinTabEnabled(!enable);
		if(wa->isWinTabEnabled()) {
			qDebug("Wintab enabled");
			inputMode = "Qt6 Wintab input";
		} else {
			qDebug("Wintab disabled");
			inputMode = "Qt6 Windows Ink input";
		}
	} else {
		qWarning("Error retrieving Windows platform integration");
	}
#else
	// Enable Windows Ink tablet event handler
	// This was taken directly from Krita
	if(enable) {
		KisTabletSupportWin8 *penFilter = new KisTabletSupportWin8();
		if(penFilter->init()) {
			app->installNativeEventFilter(penFilter);
			inputMode = "KisTablet Windows Ink input";
			qDebug("Using Win8 Pointer Input for tablet support");
		} else {
			qWarning("No Win8 Pointer Input available");
			delete penFilter;
		}
	} else {
		// Enable modified Wintab support
		// This too was taken from Krita
		qDebug("Enabling custom Wintab support");
		KisTabletSupportWin::init();
		qDebug("Win8 Pointer Input disabled");
	}
#endif
}

void updateRelativePenMode(bool enable)
{
	KisTabletSupportWin::enableRelativePenModeHack(enable);
	if(enable) {
		inputMode = "KisTablet Wintab input with relative pen mode hack";
	} else {
		inputMode = "KisTablet Wintab input";
	}
}
#endif

void init(DrawpileApp &app)
{
#if defined(Q_OS_WIN) && defined(HAVE_KIS_TABLET)
	app.settings().bind(Settings::TabletWindowsInk, &updateWindowsInk);
	app.settings().bind(Settings::TabletRelativePenMode, &updateRelativePenMode);
#else
	// Nothing to do on other platforms.
	Q_UNUSED(app);
#endif
}

QString current()
{
	return QString::fromUtf8(inputMode);
}

}
