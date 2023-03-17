// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "config.h"

#include "desktop/main.h"
#include "desktop/mainwindow.h"

#include "libclient/utils/logging.h"
#include "libclient/utils/colorscheme.h"
#include "desktop/utils/qtguicompat.h"
#include "desktop/notifications.h"
#include "desktop/dialogs/versioncheckdialog.h"
#include "libshared/util/qtcompat.h"
#include "libshared/util/paths.h"
#include "rustpile/rustpile.h"

#ifdef Q_OS_MACOS
#include "desktop/utils/macui.h"
#include "desktop/widgets/macmenu.h"
#include <QTimer>
#endif

#if defined(Q_OS_WIN) && defined(KIS_TABLET)
#include "desktop/bundled/kis_tablet/kis_tablet_support_win8.h"
#include "desktop/bundled/kis_tablet/kis_tablet_support_win.h"
#endif

#include <QDebug>
#include <QDir>
#include <QCommandLineParser>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>
#include <QUrl>
#include <QTabletEvent>
#include <QLibraryInfo>
#include <QTranslator>
#include <QDateTime>
#include <QWidget>

DrawpileApp::DrawpileApp(int &argc, char **argv)
	: QApplication(argc, argv)
{
	setOrganizationName("drawpile");
	setOrganizationDomain("drawpile.net");
	setApplicationName("drawpile");
#ifdef BUILD_LABEL
	setApplicationVersion(DRAWPILE_VERSION " " BUILD_LABEL);
	setApplicationDisplayName("Drawpile (" BUILD_LABEL ")");
#else
	setApplicationVersion(DRAWPILE_VERSION);
	setApplicationDisplayName("Drawpile");
#endif
	setWindowIcon(QIcon(":/icons/drawpile.png"));
}

/**
 * Handle tablet proximity events. When the eraser is brought near
 * the tablet surface, switch to eraser tool on all windows.
 * When the tip leaves the surface, switch back to whatever tool
 * we were using before.
 *
 * Also, on MacOS we must also handle the Open File event.
 */
bool DrawpileApp::event(QEvent *e) {
	if(e->type() == QEvent::TabletEnterProximity || e->type() == QEvent::TabletLeaveProximity) {
		QTabletEvent *te = static_cast<QTabletEvent*>(e);
		if(te->pointerType()==compat::PointerType::Eraser)
			emit eraserNear(e->type() == QEvent::TabletEnterProximity);
		return true;

	} else if(e->type() == QEvent::FileOpen) {
		QFileOpenEvent *fe = static_cast<QFileOpenEvent*>(e);

		// Note. This is currently broken in Qt 5.3.1:
		// https://bugreports.qt-project.org/browse/QTBUG-39972
		openUrl(fe->url());

		return true;

	} else if (e->type() == QEvent::ApplicationPaletteChange) {
		updateThemeIcons();
	}
#ifdef Q_OS_MACOS
	else if(e->type() == QEvent::ApplicationStateChange) {
		QApplicationStateChangeEvent *ae = static_cast<QApplicationStateChangeEvent*>(e);
		if(ae->applicationState() == Qt::ApplicationActive && topLevelWindows().isEmpty()) {
			// Open a new window when application is activated and there are no windows.
			openBlankDocument();
		}
	}
#endif

	return QApplication::event(e);
}

void DrawpileApp::notifySettingsChanged()
{
	emit settingsChanged();
}

void DrawpileApp::updateThemeIcons()
{
	auto *iconTheme = QPalette().color(QPalette::Window).lightness() < 128
		? "dark"
		: "light";

	QStringList fallbackIconPaths;
	for (const auto &path : utils::paths::dataPaths()) {
		fallbackIconPaths.append(path + "/theme/" + iconTheme);
	}

	QDir::setSearchPaths("theme", fallbackIconPaths);
	QIcon::setThemeName(iconTheme);
}

void DrawpileApp::initTheme()
{
	static QStringList defaultThemePaths{QIcon::themeSearchPaths()};

	QStringList themePaths{defaultThemePaths};
	for (const auto &path : utils::paths::dataPaths()) {
		themePaths.append(path + "/theme");
	}
	QIcon::setThemeSearchPaths(themePaths);
	setThemeName(QSettings().value("settings/theme").toString());
	setDarkMode(QSettings().value("settings/darkmode").toBool());
	updateThemeIcons();
}

void DrawpileApp::setDarkMode(bool dark)
{
#ifdef Q_OS_MACOS
	if (macui::setNativeAppearance(dark)) {
		return;
	}
#endif

	QPalette pal;
	if(dark) {
		const QString paletteFile = utils::paths::locateDataFile("nightmode.colors");
		if(paletteFile.isEmpty()) {
			qWarning("Cannot switch to night mode: couldn't find color scheme file!");
		} else {
			pal = colorscheme::loadFromFile(paletteFile);
		}
	} else {
		pal = style()->standardPalette();
	}

	setPalette(pal);
}

void DrawpileApp::openUrl(QUrl url)
{
	// See if there is an existing replacable window
	MainWindow *win = nullptr;
	for(QWidget *widget : topLevelWidgets()) {
		MainWindow *mw = qobject_cast<MainWindow*>(widget);
		if(mw && mw->canReplace()) {
			win = mw;
			break;
		}
	}

	// No? Create a new one
	if(!win)
		win = new MainWindow;

	if(url.scheme() == "drawpile") {
		// Our own protocol: connect to a session
		win->join(url);

	} else {
		// Other protocols: load image
		win->open(url);
	}
}

void DrawpileApp::openBlankDocument()
{
	// Open a new window with a blank image
	QSettings cfg;

	const QSize maxSize {65536, 65536};
	QSize size = cfg.value("history/newsize").toSize();

	if(size.width()<100 || size.height()<100) {
		// No previous size, or really small size
		size = QSize(800, 600);
	} else {
		// Make sure previous size is not ridiculously huge
		size = size.boundedTo(maxSize);
	}

	QColor color = cfg.value("history/newcolor").value<QColor>();
	if(!color.isValid())
		color = Qt::white;

	MainWindow *win = new MainWindow;
	win->newDocument(size, color);
}

static QString langFromLocale(const QLocale &locale)
{
	const auto preferredLangs = locale.uiLanguages();
	if(preferredLangs.isEmpty())
		return QString();

	// TODO we should work our way down the preferred language list
	// until we find one a translation exists for.
	QString preferredLang = preferredLangs.first();

	// On Windows, the locale name is sometimes in the form "fi-FI"
	// rather than "fi_FI" that Qt expects.
	preferredLang.replace('-', '_');

	return preferredLang;
}

void DrawpileApp::initTranslations()
{
	// Set override locale from settings, or use system locale if no override is set
	QLocale locale = QLocale::c();
	QString overrideLang = QSettings().value("settings/language").toString();
	if(!overrideLang.isEmpty())
		locale = QLocale(overrideLang);

	if(locale == QLocale::c())
		locale = QLocale::system();

	setLanguage(langFromLocale(locale));
}

void DrawpileApp::setThemeName(const QString &themeName)
{
	QStyle *style = nullptr;

#ifdef Q_OS_MACOS
	if (!themeName.isEmpty() && !themeName.toLower().startsWith("mac")) {
#else
	if (!themeName.isEmpty()) {
#endif
		style = QStyleFactory::create(themeName);

		if (!style) {
			qWarning() << "could not find theme" << themeName;
		}
	}

#ifdef Q_OS_MACOS
	if (!style)
		style = new macui::MacViewStatusBarProxyStyle;
#endif

	if (style)
		setStyle(style);
}

void DrawpileApp::setLanguage(QString preferredLang)
{
	if(preferredLang == m_currentLang)
		return;

	m_currentLang = preferredLang;

	for (auto *translator : m_translators) {
		removeTranslator(translator);
	}
	m_translators.clear();

	if(preferredLang.isEmpty()) {
		preferredLang = langFromLocale(QLocale::system());
	}

	// Special case: if english is preferred language, no translations are needed.
	if(preferredLang == "en")
		return;

	QTranslator *translator;
	// Qt's own translations
	translator = new QTranslator(this);
	if(translator->load("qt_" + preferredLang, compat::libraryPath(QLibraryInfo::TranslationsPath))) {
		m_translators.push_back(translator);
		installTranslator(translator);
	} else {
		qWarning("Qt translations not found");
		delete translator;
	}

	for(auto &bundle : { "libshared_", "libclient_", "drawpile_" }) {
		translator = new QTranslator(this);
		for(const QString &datapath : utils::paths::dataPaths()) {
			if(translator->load(bundle + preferredLang, datapath + "/i18n"))
				break;
		}

		if(translator->isEmpty()) {
			delete translator;
		} else {
			m_translators.push_back(translator);
			installTranslator(translator);
		}
	}
}

// Initialize the application and return a list of files to be opened (if any)
static QStringList initApp(DrawpileApp &app)
{
	// Parse command line arguments
	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();

	// --data-dir
	QCommandLineOption dataDir("data-dir", DrawpileApp::tr("Override read-only data directory."), "path");
	parser.addOption(dataDir);

	// --portable-data-dir
	QCommandLineOption portableDataDir("portable-data-dir", DrawpileApp::tr("Override settings directory."), "path");
	parser.addOption(portableDataDir);

	// URL
	parser.addPositionalArgument("url", DrawpileApp::tr("Filename or URL."));

	parser.process(app);

	// Override data directories
	if(parser.isSet(dataDir))
		utils::paths::setDataPath(parser.value(dataDir));

	if(parser.isSet(portableDataDir)) {
		utils::paths::setWritablePath(parser.value(portableDataDir));

		QSettings::setDefaultFormat(QSettings::IniFormat);
		QSettings::setPath(
			QSettings::IniFormat,
			QSettings::UserScope,
			utils::paths::writablePath(QStandardPaths::AppConfigLocation, QString())
		);
	}

	// Continue initialization (can use QSettings from now on)
	utils::initLogging();

	app.initTheme();

#ifdef Q_OS_MACOS
	// Mac specific settings
	app.setAttribute(Qt::AA_DontShowIconsInMenus);
	app.setQuitOnLastWindowClosed(false);

	// Global menu bar that is shown when no windows are open
	MacMenu::instance();

	// This is a hack to deal with the menu disappearing when the final
	// window is closed by a confirmation sheet.
	QObject::connect(&app, &QGuiApplication::lastWindowClosed, [] {
		QTimer::singleShot(0, [] {
			qGuiApp->focusWindowChanged(nullptr);
		});
	});
#endif

#if defined(Q_OS_WIN) && defined(KIS_TABLET)
	{
		bool useWindowsInk = false;
		// Enable Windows Ink tablet event handler
		// This was taken directly from Krita
		if(QSettings().value("settings/input/windowsink", true).toBool()) {
			KisTabletSupportWin8 *penFilter = new KisTabletSupportWin8();
			if (penFilter->init()) {
				app.installNativeEventFilter(penFilter);
				useWindowsInk = true;
				qDebug("Using Win8 Pointer Input for tablet support");

			} else {
				qWarning("No Win8 Pointer Input available");
				delete penFilter;
			}
		} else {
			qDebug("Win8 Pointer Input disabled");
		}

		if(!useWindowsInk) {
			// Enable modified Wintab support
			// This too was taken from Krita
			qDebug("Enabling custom Wintab support");
			KisTabletSupportWin::init();
		}
	}
#endif

	app.initTranslations();

	return parser.positionalArguments();
}

int main(int argc, char *argv[]) {
#ifndef HAVE_QT_COMPAT_DEFAULT_HIGHDPI_PIXMAPS
	// Set attributes that must be set before QApplication is constructed
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

	// CanvasView does not work correctly with this enabled.
	// (Scale factor must be taken in account when zooming)
	//QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	DrawpileApp app(argc, argv);

	{
		const auto files = initApp(app);

		rustpile::rustpile_init_logging(&utils::logMessage);

		if(files.isEmpty()) {
			app.openBlankDocument();

		} else {
			QUrl url(files.at(0));
			if(url.scheme().length() <= 1) {
				// no scheme (or a drive letter?) means this is probably a local file
				url = QUrl::fromLocalFile(files.at(0));
			}

			app.openUrl(url);
		}
	}

	dialogs::VersionCheckDialog::doVersionCheckIfNeeded();

	return app.exec();
}
