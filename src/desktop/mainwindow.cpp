// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include <QApplication>
#include <QActionGroup>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSettings>
#include <QFileDialog>
#include <QDesktopServices>
#include <QScreen>
#include <QUrl>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QCloseEvent>
#include <QPushButton>
#include <QToolButton>
#include <QImageReader>
#include <QImageWriter>
#include <QSplitter>
#include <QClipboard>
#include <QFile>
#include <QWindow>
#include <QVBoxLayout>
#include <QTimer>
#include <QTextEdit>
#include <QThreadPool>
#include <QKeySequence>
#ifdef Q_OS_MACOS
#include <QStyle>
#endif
#include <Qt>

#include <QtColorWidgets/ColorDialog>

#ifdef Q_OS_MACOS
static constexpr auto CTRL_KEY = Qt::META;
#include "desktop/widgets/macmenu.h"
#else
static constexpr auto CTRL_KEY = Qt::CTRL;
#endif

#include "config.h"
#include "desktop/mainwindow.h"
#include "libclient/document.h"
#include "desktop/main.h"

#include "libclient/canvas/canvasmodel.h"
#include "desktop/scene/canvasview.h"
#include "desktop/scene/canvasscene.h"
#include "desktop/scene/selectionitem.h"
#include "libclient/canvas/userlist.h"
#include "libclient/canvas/paintengine.h"
#include "libclient/canvas/documentmetadata.h"

#include "desktop/utils/recentfiles.h"
#include "libshared/util/whatismyip.h"
#include "libclient/utils/icon.h"
#include "libclient/utils/images.h"
#include "libshared/util/networkaccess.h"
#include "libshared/util/paths.h"
#include "libclient/utils/shortcutdetector.h"
#include "libclient/utils/customshortcutmodel.h"
#include "libclient/utils/logging.h"
#include "desktop/utils/actionbuilder.h"
#include "desktop/utils/hotbordereventfilter.h"
#include "libshared/util/qtcompat.h"

#include "desktop/widgets/viewstatus.h"
#include "desktop/widgets/netstatus.h"
#include "desktop/chat/chatbox.h"

#include "desktop/docks/toolsettingsdock.h"
#include "desktop/docks/brushpalettedock.h"
#include "desktop/docks/navigator.h"
#include "desktop/docks/colorpalette.h"
#include "desktop/docks/colorspinner.h"
#include "desktop/docks/colorsliders.h"
#include "desktop/docks/layerlistdock.h"
#include "desktop/docks/timeline.h"

#include "libclient/net/client.h"
#include "libclient/net/login.h"
#include "libclient/net/envelopebuilder.h"
#include "libclient/canvas/layerlist.h"
#include "libclient/parentalcontrols/parentalcontrols.h"

#include "libclient/tools/toolcontroller.h"
#include "desktop/toolwidgets/brushsettings.h"
#include "desktop/toolwidgets/colorpickersettings.h"
#include "desktop/toolwidgets/selectionsettings.h"
#include "desktop/toolwidgets/annotationsettings.h"
#include "desktop/toolwidgets/lasersettings.h"
#include "desktop/toolwidgets/zoomsettings.h"
#include "desktop/toolwidgets/inspectorsettings.h"

#include "libclient/export/animationsaverrunnable.h"
#include "libshared/record/reader.h"

#include "desktop/dialogs/newdialog.h"
#include "desktop/dialogs/hostdialog.h"
#include "desktop/dialogs/joindialog.h"
#include "desktop/dialogs/logindialog.h"
#include "desktop/dialogs/settingsdialog.h"
#include "desktop/dialogs/resizedialog.h"
#include "desktop/dialogs/playbackdialog.h"
#include "desktop/dialogs/flipbook.h"
#include "desktop/dialogs/resetdialog.h"
#include "desktop/dialogs/sessionsettings.h"
#include "desktop/dialogs/serverlogdialog.h"
#include "desktop/dialogs/tablettester.h"
#include "desktop/dialogs/abusereport.h"
#include "desktop/dialogs/versioncheckdialog.h"

#if defined(Q_OS_WIN) && defined(KIS_TABLET)
#include "desktop/bundled/kis_tablet/kis_tablet_support_win.h"
#endif

static QString getLastPath() { return QSettings().value("window/lastpath").toString(); }

static void setLastPath(const QString &lastpath) { QSettings().setValue("window/lastpath", lastpath); }

MainWindow::MainWindow(bool restoreWindowPosition)
	: QMainWindow()
	, m_splitter(nullptr)
	, m_dockToolSettings(nullptr)
	, m_dockBrushPalette(nullptr)
	, m_dockInput(nullptr)
	, m_dockLayers(nullptr)
	, m_dockColorPalette(nullptr)
	, m_dockNavigator(nullptr)
	, m_dockTimeline(nullptr)
	, m_chatbox(nullptr)
	, m_view(nullptr)
	, m_viewStatusBar(nullptr)
	, m_lockstatus(nullptr)
	, m_netstatus(nullptr)
	, m_viewstatus(nullptr)
	, m_statusChatButton(nullptr)
	, m_playbackDialog(nullptr)
	, m_sessionSettings(nullptr)
	, m_serverLogDialog(nullptr)
	, m_recentMenu(nullptr)
	, m_currentdoctools(nullptr)
	, m_admintools(nullptr)
	, m_modtools(nullptr)
	, m_canvasbgtools(nullptr)
	, m_resizetools(nullptr)
	, m_putimagetools(nullptr)
	, m_undotools(nullptr)
	, m_drawingtools(nullptr)
	, m_brushSlots(nullptr)
	, m_lastToolBeforePaste(-1)
	, m_fullscreenOldMaximized(false)
	, m_tempToolSwitchShortcut(nullptr)
	, m_exitAfterSave(false)
	, m_doc(new Document(this))
	, m_canvasscene(new drawingboard::CanvasScene(this))
{
	// Set up the main window widgets
	// The central widget consists of a custom status bar and a splitter
	// which includes the chat box and the main view.
	QWidget *centralwidget = new QWidget;
	QVBoxLayout *mainwinlayout = new QVBoxLayout(centralwidget);
	mainwinlayout->setContentsMargins(0, 0, 0 ,0);
	mainwinlayout->setSpacing(0);
	setCentralWidget(centralwidget);

	// Work area is split between the canvas view and the chatbox
	m_splitter = new QSplitter(Qt::Vertical, centralwidget);

	mainwinlayout->addWidget(m_splitter);

	// We don't use the normal QMainWindow statusbar to save some vertical space for the docks.
	m_viewStatusBar = new QStatusBar;
	m_viewStatusBar->setSizeGripEnabled(false);
	mainwinlayout->addWidget(m_viewStatusBar);

#ifdef Q_OS_MACOS
	recolorUi();
#endif

	// Create status indicator widgets
	m_viewstatus = new widgets::ViewStatus(this);

	m_netstatus = new widgets::NetStatus(this);
	m_lockstatus = new QLabel(this);
	m_lockstatus->setFixedSize(QSize(16, 16));

	// Statusbar chat button: this is normally hidden and only shown
	// when there are unread chat messages.
	m_statusChatButton = new QToolButton(this);
	m_statusChatButton->setAutoRaise(true);
	m_statusChatButton->setIcon(icon::fromTheme("drawpile_chat"));
	m_statusChatButton->hide();
	m_viewStatusBar->addWidget(m_statusChatButton);

	// Statusbar session size label
	QLabel *sessionHistorySize = new QLabel(this);
	m_viewStatusBar->addWidget(sessionHistorySize);

	m_viewStatusBar->addPermanentWidget(m_viewstatus);
	m_viewStatusBar->addPermanentWidget(m_netstatus);
	m_viewStatusBar->addPermanentWidget(m_lockstatus);

	int SPLITTER_WIDGET_IDX = 0;

	// Create canvas view (first splitter item)
	m_view = new widgets::CanvasView(this);
	m_splitter->addWidget(m_view);
	m_splitter->setCollapsible(SPLITTER_WIDGET_IDX++, false);

	// Create the chatbox
	m_chatbox = new widgets::ChatBox(m_doc, this);
	m_splitter->addWidget(m_chatbox);

	connect(m_chatbox, &widgets::ChatBox::reattachNowPlease, this, [this]() {
		m_splitter->addWidget(m_chatbox);
	});

	// Nice initial division between canvas and chat
	{
		const int h = height();
		m_splitter->setSizes(QList<int>() << (h * 2 / 3) << (h / 3));
	}

	// Create canvas scene
	m_canvasscene->setBackgroundBrush(
			palette().brush(QPalette::Active,QPalette::Window));
	m_view->setCanvas(m_canvasscene);

	// Create docks
	createDocks();

	// Crete persistent dialogs
	m_serverLogDialog = new dialogs::ServerLogDialog(this);
	m_serverLogDialog->setModel(m_doc->serverLog());

	// Document <-> Main window connections
	connect(m_doc, &Document::canvasChanged, this, &MainWindow::onCanvasChanged);
	connect(m_doc, &Document::canvasSaveStarted, this, &MainWindow::onCanvasSaveStarted);
	connect(m_doc, &Document::canvasSaved, this, &MainWindow::onCanvasSaved);
	connect(m_doc, &Document::dirtyCanvas, this, &MainWindow::setWindowModified);
	connect(m_doc, &Document::sessionTitleChanged, this, &MainWindow::updateTitle);
	connect(m_doc, &Document::currentFilenameChanged, this, &MainWindow::updateTitle);
	connect(m_doc, &Document::recorderStateChanged, this, &MainWindow::setRecorderStatus);

	connect(m_doc, &Document::autoResetTooLarge, this, [this](int maxSize) {
		m_doc->sendLockSession(true);
		auto *msgbox = new QMessageBox(
					QMessageBox::Warning,
					tr("Server out of space"),
					tr("Server is running out of history space and session has grown too large to automatically reset! (Limit is %1 MB)\nSimplify the canvas and reset manually before space runs out.")
						.arg(maxSize / double(1024*1024), 0, 'f', 2),
					QMessageBox::Ok,
					this
					);
		msgbox->show();
	});

	connect(m_doc->client(), &net::Client::sessionResetted, [this] {
		m_view->setUpdatesEnabled(false);
	});
	connect(m_doc->client(), &net::Client::catchupProgress, [this](int progress) {
		if(progress == 100) {
			m_view->setUpdatesEnabled(true);
		}
	});

	// Tool dock connections
	m_tempToolSwitchShortcut = new ShortcutDetector(this);

	connect(static_cast<tools::LaserPointerSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::LASERPOINTER)), &tools::LaserPointerSettings::pointerTrackingToggled,
		m_view, &widgets::CanvasView::setPointerTracking);
	connect(static_cast<tools::ZoomSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::ZOOM)), &tools::ZoomSettings::resetZoom,
		this, [this]() { m_view->setZoom(100.0); });
	connect(static_cast<tools::ZoomSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::ZOOM)), &tools::ZoomSettings::fitToWindow,
		m_view, &widgets::CanvasView::zoomToFit);

	tools::BrushSettings *brushSettings = static_cast<tools::BrushSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::FREEHAND));
	connect(brushSettings, &tools::BrushSettings::pressureMappingChanged, m_view, &widgets::CanvasView::setPressureMapping);

	connect(m_dockLayers, &docks::LayerList::layerSelected, this, &MainWindow::updateLockWidget);
	connect(m_dockLayers, &docks::LayerList::activeLayerVisibilityChanged, this, &MainWindow::updateLockWidget);

	connect(m_dockToolSettings, &docks::ToolSettings::sizeChanged, m_view, &widgets::CanvasView::setOutlineSize);
	connect(m_dockToolSettings, &docks::ToolSettings::subpixelModeChanged, m_view, &widgets::CanvasView::setOutlineMode);
	connect(m_view, &widgets::CanvasView::colorDropped, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(m_view, SIGNAL(imageDropped(QImage)), this, SLOT(pasteImage(QImage)));
	connect(m_view, &widgets::CanvasView::urlDropped, this, &MainWindow::dropUrl);
	connect(m_view, &widgets::CanvasView::viewTransformed, m_viewstatus, &widgets::ViewStatus::setTransformation);
	connect(m_view, &widgets::CanvasView::viewTransformed, m_dockNavigator, &docks::Navigator::setViewTransformation);

	connect(m_view, &widgets::CanvasView::viewRectChange, m_viewstatus, [this]() {
		const int min = qMin(int(m_view->fitToWindowScale() * 100), 100);
		m_viewstatus->setMinimumZoom(min);
		m_dockNavigator->setMinimumZoom(min);
	});

	connect(m_viewstatus, &widgets::ViewStatus::zoomChanged, m_view, &widgets::CanvasView::setZoom);
	connect(m_viewstatus, &widgets::ViewStatus::angleChanged, m_view, &widgets::CanvasView::setRotation);
	connect(m_dockNavigator, &docks::Navigator::zoomChanged, m_view, &widgets::CanvasView::setZoom);

	connect(m_dockToolSettings, &docks::ToolSettings::toolChanged, this, &MainWindow::toolChanged);

	// Color docks
	connect(m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged, m_dockColorPalette, &docks::ColorPaletteDock::setColor);
	connect(m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged, m_dockColorSpinner, &docks::ColorSpinnerDock::setColor);
	connect(m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged, m_dockColorSliders, &docks::ColorSliderDock::setColor);
	connect(m_dockToolSettings, &docks::ToolSettings::lastUsedColorsChanged, m_dockColorSpinner, &docks::ColorSpinnerDock::setLastUsedColors);
	connect(m_dockToolSettings, &docks::ToolSettings::lastUsedColorsChanged, m_dockColorSliders, &docks::ColorSliderDock::setLastUsedColors);
	connect(m_dockColorPalette, &docks::ColorPaletteDock::colorSelected, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(m_dockColorSpinner, &docks::ColorSpinnerDock::colorSelected, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(m_dockColorSliders, &docks::ColorSliderDock::colorSelected, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);

	// Navigator <-> View
	connect(m_dockNavigator, &docks::Navigator::focusMoved, m_view, &widgets::CanvasView::scrollTo);
	connect(m_view, &widgets::CanvasView::viewRectChange, m_dockNavigator, &docks::Navigator::setViewFocus);
	connect(m_dockNavigator, &docks::Navigator::wheelZoom, m_view, &widgets::CanvasView::zoomSteps);

	// Network client <-> UI connections
	connect(m_view, &widgets::CanvasView::pointerMoved, m_doc, &Document::sendPointerMove);

	connect(m_doc, &Document::catchupProgress, m_netstatus, &widgets::NetStatus::setCatchupProgress);

	connect(m_doc->client(), &net::Client::serverStatusUpdate, sessionHistorySize, [sessionHistorySize](int size) {
		sessionHistorySize->setText(tr("%1 MB").arg(size / float(1024*1024), 0, 'f', 2));
	});

	connect(m_chatbox, &widgets::ChatBox::message, m_doc->client(), &net::Client::sendEnvelope);
	connect(m_dockTimeline, &docks::Timeline::timelineEditCommand, m_doc->client(), &net::Client::sendEnvelope);

	connect(m_serverLogDialog, &dialogs::ServerLogDialog::opCommand, m_doc->client(), &net::Client::sendEnvelope);

	// Tool controller <-> UI connections
	connect(m_doc->toolCtrl(), &tools::ToolController::activeAnnotationChanged, m_canvasscene, &drawingboard::CanvasScene::setActiveAnnotation);
	connect(m_doc->toolCtrl(), &tools::ToolController::colorUsed, m_dockToolSettings, &docks::ToolSettings::addLastUsedColor);
	connect(m_doc->toolCtrl(), &tools::ToolController::zoomRequested, m_view, &widgets::CanvasView::zoomTo);

	connect(m_canvasscene, &drawingboard::CanvasScene::annotationDeleted, this, [this](int id) {
		if(m_doc->toolCtrl()->activeAnnotation() == id)
			m_doc->toolCtrl()->setActiveAnnotation(0);
	});

	connect(brushSettings, &tools::BrushSettings::smoothingChanged, m_doc->toolCtrl(), &tools::ToolController::setSmoothing);
	m_doc->toolCtrl()->setSmoothing(brushSettings->getSmoothing());
	connect(m_doc->toolCtrl(), &tools::ToolController::toolCursorChanged, m_view, &widgets::CanvasView::setToolCursor);
	m_view->setToolCursor(m_doc->toolCtrl()->activeToolCursor());

	connect(m_view, &widgets::CanvasView::penDown, m_doc->toolCtrl(), &tools::ToolController::startDrawing);
	connect(m_view, &widgets::CanvasView::penMove, m_doc->toolCtrl(), &tools::ToolController::continueDrawing);
	connect(m_view, &widgets::CanvasView::penHover, m_doc->toolCtrl(), &tools::ToolController::hoverDrawing);
	connect(m_view, &widgets::CanvasView::penUp, m_doc->toolCtrl(), &tools::ToolController::endDrawing);
	connect(m_view, &widgets::CanvasView::quickAdjust, m_dockToolSettings, &docks::ToolSettings::quickAdjustCurrent1);

	connect(m_dockLayers, &docks::LayerList::layerSelected, m_doc->toolCtrl(), &tools::ToolController::setActiveLayer);
	connect(m_dockLayers, &docks::LayerList::layerSelected, m_dockTimeline, &docks::Timeline::setCurrentLayer);
	connect(m_doc->toolCtrl(), &tools::ToolController::activeAnnotationChanged,
			static_cast<tools::AnnotationSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::ANNOTATION)), &tools::AnnotationSettings::setSelectionId);

	connect(m_canvasscene, &drawingboard::CanvasScene::canvasResized, m_doc->toolCtrl(), &tools::ToolController::offsetActiveTool);

	connect(m_view, &widgets::CanvasView::reconnectRequested, this, [this]() {
		joinSession(m_doc->client()->sessionUrl(true));
	});

	// Network status changes
	connect(m_doc, &Document::serverConnected, this, &MainWindow::onServerConnected);
	connect(m_doc, &Document::serverLoggedIn, this, &MainWindow::onServerLogin);
	connect(m_doc, &Document::serverDisconnected, this, &MainWindow::onServerDisconnected);
	connect(m_doc, &Document::serverDisconnected, sessionHistorySize, [sessionHistorySize]() {
		sessionHistorySize->setText(QString());
	});
	connect(m_doc, &Document::sessionNsfmChanged, this, &MainWindow::onNsfmChanged);

	connect(m_doc, &Document::serverConnected, m_netstatus, &widgets::NetStatus::connectingToHost);
	connect(m_doc->client(), &net::Client::serverDisconnecting, m_netstatus, &widgets::NetStatus::hostDisconnecting);
	connect(m_doc, &Document::serverDisconnected, m_netstatus, &widgets::NetStatus::hostDisconnected);
	connect(m_doc, &Document::sessionRoomcodeChanged, m_netstatus, &widgets::NetStatus::setRoomcode);

	connect(m_doc->client(), SIGNAL(bytesReceived(int)), m_netstatus, SLOT(bytesReceived(int)));
	connect(m_doc->client(), &net::Client::bytesSent, m_netstatus, &widgets::NetStatus::bytesSent);
	connect(m_doc->client(), &net::Client::lagMeasured, m_netstatus, &widgets::NetStatus::lagMeasured);
	connect(m_doc->client(), &net::Client::youWereKicked, m_netstatus, &widgets::NetStatus::kicked);

	connect(qApp, SIGNAL(settingsChanged()), this, SLOT(loadShortcuts()));
	connect(qApp, SIGNAL(settingsChanged()), this, SLOT(updateSettings()));
	connect(qApp, SIGNAL(settingsChanged()), m_view, SLOT(updateShortcuts()));

	updateSettings();

	// Create actions and menus
	setupActions();
	setDrawingToolsEnabled(false);

	// Restore settings
	readSettings(restoreWindowPosition);

	// Set status indicators
	updateLockWidget();
	setRecorderStatus(false);

#ifndef Q_OS_MACOS
	// OSX provides this feature itself
	HotBorderEventFilter *hbfilter = new HotBorderEventFilter(this);
	m_view->installEventFilter(hbfilter);
	for(QObject *c : children()) {
		QToolBar *tb = dynamic_cast<QToolBar*>(c);
		if(tb)
			tb->installEventFilter(hbfilter);
	}

	connect(hbfilter, &HotBorderEventFilter::hotBorder, this, &MainWindow::hotBorderMenubar);
#endif

	makeTranslator(this, [=] {
		updateTitle();
	});
	show();

	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
}

MainWindow::~MainWindow()
{
	// Clear this out first so there will be no weird signals emitted
	// while the document is being torn down.
	m_view->setScene(nullptr);
	delete m_canvasscene;

	// Make sure all child dialogs are closed
	QObjectList lst = children();
	for(QObject *obj : lst) {
		QDialog *child = qobject_cast<QDialog*>(obj);
		delete child;
	}
}

void MainWindow::onCanvasChanged(canvas::CanvasModel *canvas)
{
	m_canvasscene->initCanvas(canvas);

	connect(canvas->aclState(), &canvas::AclState::localOpChanged, this, &MainWindow::onOperatorModeChange);
	connect(canvas->aclState(), &canvas::AclState::localLockChanged, this, &MainWindow::updateLockWidget);
	connect(canvas->aclState(), &canvas::AclState::featureAccessChanged, this, &MainWindow::onFeatureAccessChange);

	connect(canvas, &canvas::CanvasModel::chatMessageReceived, this, [this]() {
		// Show a "new message" indicator when the chatbox is collapsed
		const auto sizes = m_splitter->sizes();
		if(sizes.length() > 1 && sizes.at(1)==0)
			m_statusChatButton->show();
	});

	connect(canvas, &canvas::CanvasModel::layerAutoselectRequest, m_dockLayers, &docks::LayerList::selectLayer);
	connect(canvas, &canvas::CanvasModel::colorPicked, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(canvas, &canvas::CanvasModel::colorPicked, static_cast<tools::ColorPickerSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::PICKER)), &tools::ColorPickerSettings::addColor);
	connect(canvas, &canvas::CanvasModel::canvasInspected, static_cast<tools::InspectorSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::INSPECTOR)), &tools::InspectorSettings::onCanvasInspected);
	connect(canvas, &canvas::CanvasModel::previewAnnotationRequested, m_doc->toolCtrl(), &tools::ToolController::setActiveAnnotation);

	connect(canvas, &canvas::CanvasModel::selectionRemoved, this, &MainWindow::selectionRemoved);

	connect(canvas, &canvas::CanvasModel::userJoined, this, [this](int, const QString &name) {
		m_viewStatusBar->showMessage(tr("🙋 %1 joined!").arg(name), 2000);
	});

	connect(m_serverLogDialog, &dialogs::ServerLogDialog::inspectModeChanged, canvas, QOverload<int>::of(&canvas::CanvasModel::inspectCanvas));
	connect(m_serverLogDialog, &dialogs::ServerLogDialog::inspectModeStopped, canvas, &canvas::CanvasModel::stopInspectingCanvas);

	updateLayerViewMode();

	m_dockLayers->setCanvas(canvas);
	m_serverLogDialog->setUserList(canvas->userlist());
	m_dockNavigator->setCanvasModel(canvas);
	m_dockTimeline->setTimeline(canvas->timeline());

	m_dockTimeline->setFps(canvas->metadata()->framerate());
	m_dockTimeline->setUseTimeline(canvas->metadata()->useTimeline());
	connect(canvas->metadata(), &canvas::DocumentMetadata::framerateChanged, m_dockTimeline, &docks::Timeline::setFps);
	connect(canvas->metadata(), &canvas::DocumentMetadata::useTimelineChanged, m_dockTimeline, &docks::Timeline::setUseTimeline);

	connect(m_dockTimeline, &docks::Timeline::currentFrameChanged, canvas->paintEngine(), &canvas::PaintEngine::setViewFrame);
	connect(m_dockTimeline, &docks::Timeline::layerSelectRequested, m_dockLayers, &docks::LayerList::selectLayer);

	static_cast<tools::InspectorSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::INSPECTOR))->setUserList(m_canvasscene->model()->userlist());

	// Make sure the UI matches the default feature access level
	m_currentdoctools->setEnabled(true);
	setDrawingToolsEnabled(true);
	for(int i=0;i<canvas::FeatureCount;++i) {
		onFeatureAccessChange(canvas::Feature(i), m_doc->canvas()->aclState()->canUseFeature(canvas::Feature(i)));
	}
}

/**
 * This function is used to check if the current board can be replaced
 * or if a new window is needed to open other content.
 *
 * The window cannot be replaced if any of the following conditions are true:
 * - there are unsaved changes
 * - there is a network connection
 * - session recording is in progress
 * - recording playback is in progress
 *
 * @retval false if a new window needs to be created
 */
bool MainWindow::canReplace() const {
	return !(m_doc->isDirty() || m_doc->client()->isConnected() || m_doc->isRecording() || m_playbackDialog);
}

/**
 * Get either a new MainWindow or this one if replacable
 */
MainWindow *MainWindow::replaceableWindow()
{
	if(!canReplace()) {
		if(windowState().testFlag(Qt::WindowFullScreen))
			toggleFullscreen();
		getAction("hidedocks")->setChecked(false);
		writeSettings();
		MainWindow *win = new MainWindow(false);
		Q_ASSERT(win->canReplace());
		return win;

	} else {
		return this;
	}
}

/**
 * The file is added to the list of recent files and the menus on all open
 * mainwindows are updated.
 * @param file filename to add
 */
void MainWindow::addRecentFile(const QString& file)
{
	RecentFiles::addFile(file);
	for(QWidget *widget : QApplication::topLevelWidgets()) {
		MainWindow *win = qobject_cast<MainWindow*>(widget);
		if(win)
			RecentFiles::initMenu(win->m_recentMenu);
	}
#ifdef Q_OS_MACOS
	MacMenu::instance()->updateRecentMenu();
#endif
}

/**
 * Set window title according to currently open file and session
 */
void MainWindow::updateTitle()
{
	QString name;
	if(m_doc->currentFilename().isEmpty()) {
		name = tr("Untitled");

	} else {
		const QFileInfo info(m_doc->currentFilename());
		name = info.baseName();
	}

	if(m_doc->sessionTitle().isEmpty())
		setWindowTitle(QStringLiteral("%1[*]").arg(name));
	else
		setWindowTitle(QStringLiteral("%1[*] - %2").arg(name, m_doc->sessionTitle()));
}

void MainWindow::toggleChat(bool show)
{
	QList<int> sizes;
	if(show) {
		QVariant oldHeight = m_chatbox->property("oldheight");
		if(oldHeight.isNull()) {
			const int h = height();
			sizes << h * 2 / 3;
			sizes << h / 3;
		} else {
			const int oh = oldHeight.toInt();
			sizes << height() - oh;
			sizes << oh;
		}
		m_chatbox->focusInput();
	} else {
		m_chatbox->setProperty("oldheight", m_chatbox->height());
		sizes << 1;
		sizes << 0;
	}
	m_splitter->setSizes(sizes);
}

void MainWindow::recolorUi()
{
#ifdef Q_OS_MACOS
	// The "native" style status bar is wrong because Qt uses the same gradient
	// as the title bar for the status bar. This makes it look better.
	if(icon::isDark(palette().color(QPalette::Window)))
		m_viewStatusBar->setStyleSheet("QStatusBar { background: #323232 }");
	else
		m_viewStatusBar->setStyleSheet("QStatusBar { background: #ececec }");
#endif
}

void MainWindow::setDrawingToolsEnabled(bool enable)
{
	m_drawingtools->setEnabled(enable && m_doc->canvas());
}

/**
 * Load customized shortcuts
 */
void MainWindow::loadShortcuts()
{
	QSettings cfg;
	cfg.beginGroup("settings/shortcuts");

	static const QRegularExpression shortcutAmpersand { "&([^&])" };

	disconnect(m_textCopyConnection);
	const QKeySequence standardCopyShortcut { QKeySequence::Copy };

	QList<QAction*> actions = findChildren<QAction*>();
	for(QAction *a : actions) {
		const QString &name = a->objectName();
		if(!name.isEmpty()) {
			if(cfg.contains(name)) {
				const auto v = cfg.value(name);
				QList<QKeySequence> shortcuts;

				if(v.canConvert<QKeySequence>()) {
					shortcuts << v.value<QKeySequence>();
				} else {
					const auto list = v.toList();
					for(const auto &vv : list) {
						if(vv.canConvert<QKeySequence>())
							shortcuts << vv.value<QKeySequence>();
					}
				}
				a->setShortcuts(shortcuts);

			} else {
				a->setShortcut(CustomShortcutModel::getDefaultShortcut(name));
			}

			if(a->shortcut() == standardCopyShortcut) {
				m_textCopyConnection = connect(a, &QAction::triggered, this, &MainWindow::copyText);
			}

			// If an action has a shortcut, show it in the tooltip
			if(a->shortcut().isEmpty()) {
				a->setToolTip(QString());

			} else {
				QString text = a->text();
				text.replace(shortcutAmpersand, QStringLiteral("\\1"));

				// In languages with non-latin alphabets, it's a common
				// convention to add a keyboard shortcut like this:
				// English: &File
				// Japanese: ファイル(&F)
				const int i = text.lastIndexOf('(');
				if(i>0)
					text.truncate(i);

				a->setToolTip(QStringLiteral("%1 (%2)").arg(text, a->shortcut().toString()));
			}
		}
	}

	// Update enabled status of certain actions
	QAction *uncensorAction = getAction("layerviewuncensor");
	const bool canUncensor = !parentalcontrols::isLayerUncensoringBlocked();
	uncensorAction->setEnabled(canUncensor);
	if(!canUncensor) {
		uncensorAction->setChecked(false);
		updateLayerViewMode();
	}
}

void MainWindow::updateSettings()
{
	QSettings cfg;
	cfg.beginGroup("settings/input");

	const bool enable = cfg.value("tabletevents", true).toBool();
	const bool eraser = cfg.value("tableteraser", true).toBool();

	m_view->setTabletEnabled(enable);

#if defined(Q_OS_WIN) && defined(KIS_TABLET)
	KisTabletSupportWin::enableRelativePenModeHack(cfg.value("relativepenhack", false).toBool());
#endif

	// Handle eraser event
	if(eraser)
		connect(qApp, SIGNAL(eraserNear(bool)), m_dockToolSettings, SLOT(eraserNear(bool)), Qt::UniqueConnection);
	else
		disconnect(qApp, SIGNAL(eraserNear(bool)), m_dockToolSettings, SLOT(eraserNear(bool)));

	// not really tablet related, but close enough
	m_view->setTouchGestures(
		cfg.value("touchscroll", true).toBool(),
		cfg.value("touchpinch", true).toBool(),
		cfg.value("touchtwist", true).toBool()
	);
	cfg.endGroup();

	cfg.beginGroup("settings");
	m_view->setBrushCursorStyle(cfg.value("brushcursor").toInt(), cfg.value("brushoutlinewidth").toReal());
	static_cast<tools::BrushSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::FREEHAND))->setShareBrushSlotColor(cfg.value("sharebrushslotcolor", false).toBool());
	cfg.endGroup();

	cfg.beginGroup("settings/input");
	m_view->setEnableViewportEntryHack(cfg.value("viewportentryhack").toBool());
	cfg.endGroup();
}

void MainWindow::updateLayerViewMode()
{
	if(!m_doc->canvas())
		return;

	const bool censor = !getAction("layerviewuncensor")->isChecked();

	rustpile::LayerViewMode mode = rustpile::LayerViewMode::Normal;

	if(getAction("layerviewsolo")->isChecked()) {
		mode = rustpile::LayerViewMode::Solo;

	} else if(getAction("layerviewframe")->isChecked()) {
		if(getAction("layerviewonionskin")->isChecked())
			mode = rustpile::LayerViewMode::Onionskin;
		else
			mode = rustpile::LayerViewMode::Frame;
	}

	m_doc->canvas()->paintEngine()->setViewMode(mode, censor);
	updateLockWidget();
}

/**
 * Read and apply mainwindow related settings.
 */
void MainWindow::readSettings(bool windowpos)
{
	QSettings cfg;
	cfg.beginGroup("window");

	// Restore previously used window size and position
	resize(cfg.value("size",QSize(800,600)).toSize());

	if(windowpos && cfg.contains("pos")) {
		const QPoint pos = cfg.value("pos").toPoint();
		if(qApp->primaryScreen()->availableGeometry().contains(pos))
			move(pos);
	}

	bool maximize = cfg.value("maximized", false).toBool();
	if(maximize)
		setWindowState(Qt::WindowMaximized);

	// Restore dock, toolbar and view states
	if(cfg.contains("state")) {
		restoreState(cfg.value("state").toByteArray());
	}
	if(cfg.contains("viewstate")) {
		m_splitter->restoreState(cfg.value("viewstate").toByteArray());
	}

	// Restore remembered actions
	cfg.beginGroup("actions");
	for(QAction *act : actions()) {
		if(act->isCheckable() && act->property("remembered").toBool()) {
			act->setChecked(cfg.value(act->objectName(), act->property("defaultValue")).toBool());
		}
	}
	cfg.endGroup();
	cfg.endGroup();

	// Restore tool settings
	m_dockToolSettings->readSettings();

	// Customize shortcuts
	loadShortcuts();

	// Restore recent files
	RecentFiles::initMenu(m_recentMenu);
}

/**
 * Write out settings
 */
void MainWindow::writeSettings()
{
	QSettings cfg;
	cfg.beginGroup("window");

	cfg.setValue("pos", normalGeometry().topLeft());
	cfg.setValue("size", normalGeometry().size());

	cfg.setValue("maximized", isMaximized());
	cfg.setValue("state", saveState());
	cfg.setValue("viewstate", m_splitter->saveState());

	// Save all remembered actions
	cfg.beginGroup("actions");
	for(const QAction *act : actions()) {
		if(act->isCheckable() && act->property("remembered").toBool())
			cfg.setValue(act->objectName(), act->isChecked());
	}
	cfg.endGroup();
	cfg.endGroup();

	m_dockToolSettings->saveSettings();
}

/**
 * Confirm exit. A confirmation dialog is popped up if there are unsaved
 * changes or network connection is open.
 * @param event event info
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
	if(m_doc->isSaveInProgress()) {
		// Don't quit while save is in progress
		m_exitAfterSave = true;
		event->ignore();
		return;
	}

	if(canReplace() == false) {

		// First confirm disconnection
		if(m_doc->client()->isLoggedIn()) {
			QMessageBox box(
				QMessageBox::Information,
				tr("Exit Drawpile"),
				tr("You are still connected to a drawing session."),
				QMessageBox::NoButton, this);
			box.setWindowModality(Qt::WindowModal);

			const QPushButton *exitbtn = box.addButton(tr("Exit anyway"),
					QMessageBox::AcceptRole);
			box.addButton(tr("Cancel"),
					QMessageBox::RejectRole);

			box.exec();
			if(box.clickedButton() == exitbtn) {
				m_doc->client()->disconnectFromServer();
			} else {
				event->ignore();
				return;
			}
		}

		// Then confirm unsaved changes
		if(isWindowModified()) {
			QMessageBox box(QMessageBox::Question, tr("Exit Drawpile"),
					tr("There are unsaved changes. Save them before exiting?"),
					QMessageBox::NoButton, this);
			box.setWindowModality(Qt::WindowModal);
			const QPushButton *savebtn = box.addButton(tr("Save"),
					QMessageBox::AcceptRole);
			box.addButton(tr("Discard"),
					QMessageBox::DestructiveRole);
			const QPushButton *cancelbtn = box.addButton(tr("Cancel"),
					QMessageBox::RejectRole);

			box.exec();
			bool cancel = false;
			// Save and exit, or cancel exit if couldn't save.
			if(box.clickedButton() == savebtn) {
				cancel = true;
				m_exitAfterSave = true;
				save();
			}

			// Cancel exit
			if(box.clickedButton() == cancelbtn || cancel) {
				event->ignore();
				return;
			}
		}
	}
	exit();
}

bool MainWindow::event(QEvent *event)
{
	if(event->type() == QEvent::StatusTip) {
		m_viewStatusBar->showMessage(static_cast<QStatusTipEvent*>(event)->tip());
		return true;
	} else {
		// Monitor key-up events to switch back from temporary tools/tool slots.
		// A short tap of the tool switch shortcut switches the tool permanently as usual,
		// but when holding it down, the tool is activated just temporarily. The
		// previous tool be switched back automatically when the shortcut key is released.
		// Note: for simplicity, we only support tools with single key shortcuts.
		if(event->type() == QEvent::KeyRelease && m_toolChangeTime.elapsed() > 250) {
			const QKeyEvent *e = static_cast<const QKeyEvent*>(event);
			if(!e->isAutoRepeat()) {
				if(m_tempToolSwitchShortcut->isShortcutSent()) {
					if(e->modifiers() == Qt::NoModifier) {
						// Return from temporary tool change
						for(const QAction *act : m_drawingtools->actions()) {
							const QKeySequence &seq = act->shortcut();
							if(seq.count()==1 && compat::keyPressed(*e) == seq[0]) {
								m_dockToolSettings->setPreviousTool();
								break;
							}
						}

						// Return from temporary tool slot change
						for(const QAction *act : m_brushSlots->actions()) {
							const QKeySequence &seq = act->shortcut();
							if(seq.count()==1 && compat::keyPressed(*e) == seq[0]) {
								m_dockToolSettings->setPreviousTool();
								break;
							}
						}
					}
				}

				m_tempToolSwitchShortcut->reset();
			}

		} else if(event->type() == QEvent::ShortcutOverride) {
			// QLineEdit doesn't seem to override the Return key shortcut,
			// so we have to do it ourself.
			const QKeyEvent *e = static_cast<QKeyEvent*>(event);
			if(e->key() == Qt::Key_Return) {
				QWidget *focus = QApplication::focusWidget();
				if(focus && focus->inherits("QLineEdit")) {
					event->accept();
					return true;
				}
			}
		}

		return QMainWindow::event(event);
	}
}

/**
 * Show the "new document" dialog
 */
void MainWindow::showNew()
{
	auto dlg = new dialogs::NewDialog(this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	connect(dlg, &dialogs::NewDialog::accepted, this, &MainWindow::newDocument);
	dlg->show();
}

void MainWindow::newDocument(const QSize &size, const QColor &background)
{
	MainWindow *w = replaceableWindow();
	w->m_doc->loadCanvas(size, background);
}

/**
 * Open the selected file
 * @param file file to open
 * @pre file.isEmpty()!=false
 */
void MainWindow::open(const QUrl& url)
{
	MainWindow *w = replaceableWindow();
	if(w != this) {
		w->open(url);
		return;
	}

	if(url.isLocalFile()) {
		QString file = url.toLocalFile();
		if(recording::Reader::isRecordingExtension(file)) {
			const auto result = m_doc->loadRecording(file);
			showErrorMessage(result);
			if(result == rustpile::CanvasIoError::NoError || result == rustpile::CanvasIoError::PartiallySupportedFormat || result == rustpile::CanvasIoError::UnknownRecordingVersion) {
				QFileInfo fileinfo(file);
				m_playbackDialog = new dialogs::PlaybackDialog(m_doc->canvas(), this);
				m_playbackDialog->setWindowTitle(fileinfo.baseName() + " - " + m_playbackDialog->windowTitle());
				m_playbackDialog->setAttribute(Qt::WA_DeleteOnClose);
				m_playbackDialog->show();
				m_playbackDialog->centerOnParent();
			}

		} else {
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			const auto result = m_doc->loadCanvas(file);
			showErrorMessage(result);
			if(result != rustpile::CanvasIoError::NoError && result != rustpile::CanvasIoError::PartiallySupportedFormat) {
				QApplication::restoreOverrideCursor();
				return;
			}

			QApplication::restoreOverrideCursor();
			getAction("hostsession")->setEnabled(true);
		}

		addRecentFile(file);

	} else {
		auto *filedownload = new networkaccess::FileDownload(this);

		filedownload->setTarget(
			utils::paths::writablePath(
				QStandardPaths::DownloadLocation,
				".",
				url.fileName().isEmpty() ? QStringLiteral("drawpile-download") : url.fileName()
				)
			);

		connect(filedownload, &networkaccess::FileDownload::progress, m_netstatus, &widgets::NetStatus::setDownloadProgress);
		connect(filedownload, &networkaccess::FileDownload::finished, this, [this, filedownload](const QString &errorMessage) {
			m_netstatus->hideDownloadProgress();
			filedownload->deleteLater();

			if(errorMessage.isEmpty()) {
				QFile *f = static_cast<QFile*>(filedownload->file()); // this is guaranteed to be a QFile when we used setTarget()
				open(QUrl::fromLocalFile(f->fileName()));
			} else {
				showErrorMessage(errorMessage);
			}
		});

		filedownload->start(url);
	}
}

/**
 * Show a file selector dialog. If there are unsaved changes, open the file
 * in a new window.
 */
void MainWindow::open()
{
	const QString file = QFileDialog::getOpenFileName(
		this,
		tr("Open Image"),
		getLastPath(),
		utils::fileFormatFilter(utils::FileFormatOption::OpenEverything)
	);

	const QUrl url = QUrl::fromLocalFile(file);
	if(url.isValid()) {
		setLastPath(file);
		open(url);
	}
}

/**
 * Allows the user three choices:
 * <ul>
 * <li>Cancel</li>
 * <li>Go ahead and flatten the image, then save<li>
 * <li>Save in OpenRaster format instead</li>
 * </ul>
 * If user chooces to save in OpenRaster, the suffix of file parameter is
 * altered.
 * @param file file name (may be altered)
 * @return true if file should be saved
 */
bool MainWindow::confirmFlatten(QString& file) const
{
	QMessageBox box(QMessageBox::Information, tr("Save Image"),
			tr("The selected format does not support layers or annotations."),
			QMessageBox::Cancel);
	box.addButton(tr("Flatten"), QMessageBox::AcceptRole);
	QPushButton *saveora = box.addButton(tr("Save as OpenRaster"), QMessageBox::ActionRole);

	// Don't save at all
	if(box.exec() == QMessageBox::Cancel)
		return false;

	// Save
	if(box.clickedButton() == saveora) {
		file = file.left(file.lastIndexOf('.')) + ".ora";
	}
	return true;
}

/**
 * If no file name has been selected, \a saveas is called.
 */
void MainWindow::save()
{
	QString filename = m_doc->currentFilename();

	if(filename.isEmpty() || !utils::isWritableFormat(filename)) {
		saveas();
		return;
	}

	if(!filename.endsWith("ora", Qt::CaseInsensitive) && m_doc->canvas()->paintEngine()->needsOpenRaster()) {
		// Note: the user may decide to save an ORA file instead, in which case the name is changed
		if(confirmFlatten(filename)==false)
			return;
	}

	// Overwrite current file
	m_doc->saveCanvas(filename);

	addRecentFile(filename);
}

/**
 * A standard file dialog is used to get the name of the file to save.
 * If no suffix is the suffix from the current filter is used.
 */
void MainWindow::saveas()
{
	QString selfilter;

	QString file = QFileDialog::getSaveFileName(
			this,
			tr("Save Image"),
			getLastPath(),
			utils::fileFormatFilter(utils::FileFormatOption::SaveImages),
			&selfilter);

	if(!file.isEmpty()) {

		// Set file suffix if missing
		const QFileInfo info(file);
		if(info.suffix().isEmpty()) {
			if(selfilter.isEmpty()) {
				// If we don't have selfilter, pick what is best
				if(m_doc->canvas()->paintEngine()->needsOpenRaster())
					file += ".ora";
				else
					file += ".png";
			} else {
				// Use the currently selected filter
				int i = selfilter.indexOf("*.")+1;
				int i2 = selfilter.indexOf(')', i);
				file += selfilter.mid(i, i2-i);
			}
		}

		// Confirm format choice if saving would result in flattening layers
		if(m_doc->canvas()->paintEngine()->needsOpenRaster() && !file.endsWith(".ora", Qt::CaseInsensitive)) {
			if(confirmFlatten(file)==false)
				return;
		}

		// Save the image
		setLastPath(file);
		m_doc->saveCanvas(file);
		addRecentFile(file);
	}
}

void MainWindow::saveSelection()
{
	QString selfilter = "PNG (*.png)";

	QString file = QFileDialog::getSaveFileName(
			this,
			tr("Save Image"),
			getLastPath(),
			utils::fileFormatFilter(utils::FileFormatOption::SaveImages | utils::FileFormatOption::QtImagesOnly),
			&selfilter);

	if(!file.isEmpty()) {
		// Set file suffix if missing
		const QFileInfo info(file);
		if(info.suffix().isEmpty()) {
			if(selfilter.isEmpty()) {
				file += ".png";
			} else {
				// Use the currently selected filter
				int i = selfilter.indexOf("*.")+1;
				int i2 = selfilter.indexOf(')', i);
				file += compat::StringView{selfilter}.mid(i, i2-i);
			}
		}

		// Save the image
		setLastPath(file);
		m_doc->saveSelection(file);
	}
}

void MainWindow::onCanvasSaveStarted()
{
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
	getAction("savedocument")->setEnabled(false);
	getAction("savedocumentas")->setEnabled(false);
	m_viewStatusBar->showMessage(tr("Saving..."));

}

void MainWindow::onCanvasSaved(const QString &errorMessage)
{
	QApplication::restoreOverrideCursor();
	getAction("savedocument")->setEnabled(true);
	getAction("savedocumentas")->setEnabled(true);

	setWindowModified(m_doc->isDirty());
	updateTitle();

	if(!errorMessage.isEmpty())
		showErrorMessage(tr("Couldn't save image"), errorMessage);
	else
		m_viewStatusBar->showMessage(tr("Image saved"), 1000);

	// Cancel exit if canvas is modified while it was being saved
	if(m_doc->isDirty())
		m_exitAfterSave = false;

	if(m_exitAfterSave)
		close();
}

void MainWindow::exportGifAnimation()
{
	const QString path = QFileDialog::getSaveFileName(
		this,
		tr("Export Animated GIF"),
		getLastPath(),
		"GIF (*.gif)"
	);

	exportAnimation(path, rustpile::AnimationExportMode::Gif);
}

void MainWindow::exportAnimationFrames()
{
	const QString path = QFileDialog::getExistingDirectory(
		this,
		tr("Choose folder to save frames in"),
		getLastPath()
	);

	exportAnimation(path, rustpile::AnimationExportMode::Frames);
}

void MainWindow::exportAnimation(const QString &path, rustpile::AnimationExportMode mode)
{
	if(path.isEmpty())
		return;

	auto *progressDialog = new QProgressDialog(
		tr("Saving animation..."),
		tr("Cancel"),
		0,
		100,
		this);
	progressDialog->setMinimumDuration(500);

	auto *saver = new AnimationSaverRunnable(
		m_doc->canvas()->paintEngine(),
		mode,
		path);

	connect(saver, &AnimationSaverRunnable::progress, progressDialog, &QProgressDialog::setValue);
	connect(saver, &AnimationSaverRunnable::saveComplete, this, QOverload<const QString&, const QString&>::of(&MainWindow::showErrorMessage));
	connect(progressDialog, &QProgressDialog::canceled, saver, &AnimationSaverRunnable::cancelExport);

	progressDialog->setValue(0);
	QThreadPool::globalInstance()->start(saver);
}

void MainWindow::showFlipbook()
{
	dialogs::Flipbook *fp = new dialogs::Flipbook(this);
	fp->setAttribute(Qt::WA_DeleteOnClose);
	fp->setPaintEngine(m_doc->canvas()->paintEngine());
	fp->show();
}

void MainWindow::setRecorderStatus(bool on)
{
	QAction *recordAction = getAction("recordsession");

	if(m_playbackDialog) {
		if(m_playbackDialog->isPlaying()) {
			recordAction->setIcon(icon::fromTheme("media-playback-pause"));
			recordAction->setText(tr("Pause"));
		} else {
			recordAction->setIcon(icon::fromTheme("media-playback-start"));
			recordAction->setText(tr("Play"));
		}
	} else {
		if(on) {
			recordAction->setText(tr("Stop Recording"));
			recordAction->setIcon(icon::fromTheme("media-playback-stop"));
		} else {
			recordAction->setText(tr("Record..."));
			recordAction->setIcon(icon::fromTheme("media-record"));
		}
	}
}

void MainWindow::toggleRecording()
{
	if(m_playbackDialog) {
		// If the playback dialog is visible, this action works as the play/pause button
		m_playbackDialog->setPlaying(!m_playbackDialog->isPlaying());
		return;
	}

	if(m_doc->isRecording()) {
		m_doc->stopRecording();
		return;
	}

	QString file = QFileDialog::getSaveFileName(
		this,
		tr("Record Session"),
		getLastPath(),
		utils::fileFormatFilter(utils::FileFormatOption::SaveRecordings)
	);

	if(!file.isEmpty()) {
		const auto result = m_doc->startRecording(file);
		showErrorMessage(result);
	}
}

/**
 * The settings window will be window modal and automatically destruct
 * when it is closed.
 */
void MainWindow::showSettings()
{
	dialogs::SettingsDialog *dlg = new dialogs::SettingsDialog;
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setWindowModality(Qt::ApplicationModal);
	dlg->show();
}

void MainWindow::host()
{
	if(!m_doc->canvas()) {
		qWarning("No canvas!");
		return;
	}

	auto dlg = new dialogs::HostDialog(this);

	connect(dlg, &dialogs::HostDialog::finished, this, [this, dlg](int i) {
		if(i==QDialog::Accepted) {
			dlg->rememberSettings();
			hostSession(dlg);
		}
		dlg->deleteLater();
	});
	dlg->show();
}

void MainWindow::hostSession(dialogs::HostDialog *dlg)
{
	const bool useremote = !dlg->getRemoteAddress().isEmpty();
	QUrl address;

	if(useremote) {
		QString scheme;
		if(dlg->getRemoteAddress().startsWith("drawpile://")==false)
			scheme = "drawpile://";
		address = QUrl(scheme + dlg->getRemoteAddress(),
				QUrl::TolerantMode);

	} else {
		address.setHost(WhatIsMyIp::guessLocalAddress());
	}

	if(address.isValid() == false || address.host().isEmpty()) {
		dlg->show();
		showErrorMessage(tr("Invalid address"));
		return;
	}

	// Start server if hosting locally
	if(!useremote) {
#if 0 // FIXME
		auto *server = new server::BuiltinServer(
			m_doc->canvas()->stateTracker(),
			m_doc->canvas()->aclFilter(),
			this);

		QString errorMessage;
		if(!server->start(&errorMessage)) {
			QMessageBox::warning(this, tr("Host Session"), errorMessage);
			delete server;
			return;
		}

		connect(m_doc->client(), &net::Client::serverDisconnected, server, &server::BuiltinServer::stop);
		connect(m_doc->canvas()->stateTracker(), &canvas::StateTracker::softResetPoint, server, &server::BuiltinServer::doInternalReset);

		if(server->port() != DRAWPILE_PROTO_DEFAULT_PORT)
			address.setPort(server->port());
#endif
	}

	// Connect to server
	net::LoginHandler *login = new net::LoginHandler(
		useremote ? net::LoginHandler::Mode::HostRemote : net::LoginHandler::Mode::HostBuiltin,
		address,
		this);
	login->setUserId(m_doc->canvas()->localUserId());
	login->setSessionAlias(dlg->getSessionAlias());
	login->setPassword(dlg->getPassword());
	login->setTitle(dlg->getTitle());
	login->setAnnounceUrl(dlg->getAnnouncementUrl(), dlg->getAnnouncmentPrivate());
	if(useremote) {
		login->setInitialState(m_doc->canvas()->generateSnapshot());
	}

	(new dialogs::LoginDialog(login, this))->show();

	m_doc->client()->connectToServer(login);
}

/**
 * Show the join dialog
 */
void MainWindow::join(const QUrl &url)
{
	auto dlg = new dialogs::JoinDialog(url, this);

	connect(dlg, &dialogs::JoinDialog::finished, this, [this, dlg](int i) {
		if(i==QDialog::Accepted) {
			QUrl url = dlg->getUrl();

			if(!url.isValid()) {
				// TODO add validator to prevent this from happening
				showErrorMessage("Invalid address");
				return;
			}

			dlg->rememberSettings();

			joinSession(url, dlg->autoRecordFilename());
		}
		dlg->deleteLater();
	});
	dlg->show();
}

/**
 * Leave action triggered, ask for confirmation
 */
void MainWindow::leave()
{
	QMessageBox *leavebox = new QMessageBox(
		QMessageBox::Question,
		m_doc->sessionTitle().isEmpty() ? tr("Untitled") : m_doc->sessionTitle(),
		tr("Really leave the session?"),
		QMessageBox::NoButton,
		this,
		Qt::MSWindowsFixedSizeDialogHint|Qt::Sheet
	);
	leavebox->setAttribute(Qt::WA_DeleteOnClose);
	leavebox->addButton(tr("Leave"), QMessageBox::YesRole);
	leavebox->setDefaultButton(
			leavebox->addButton(tr("Stay"), QMessageBox::NoRole)
			);
	connect(leavebox, &QMessageBox::finished, this, [this](int result) {
		if(result == 0)
			m_doc->client()->disconnectFromServer();
	});

	if(m_doc->client()->uploadQueueBytes() > 0) {
		leavebox->setIcon(QMessageBox::Warning);
		leavebox->setInformativeText(tr("There is still unsent data! Please wait until transmission completes!"));
	}

	leavebox->show();
}

void MainWindow::reportAbuse()
{
	dialogs::AbuseReportDialog *dlg = new dialogs::AbuseReportDialog(this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);

	dlg->setSessionInfo(QString(), QString(), m_doc->sessionTitle());

	const canvas::UserListModel *userlist = m_doc->canvas()->userlist();
	for(const auto &u : userlist->users()) {
		if(u.isOnline && u.id != m_doc->canvas()->localUserId())
			dlg->addUser(u.id, u.name);
	}

	connect(dlg, &dialogs::AbuseReportDialog::accepted, this, [this, dlg]() {
		m_doc->sendAbuseReport(dlg->userId(), dlg->message());
	});

	dlg->show();
}

void MainWindow::tryToGainOp()
{
	QString opword = QInputDialog::getText(
				this,
				tr("Become Operator"),
				tr("Enter operator password"),
				QLineEdit::Password
	);
	if(!opword.isEmpty())
		m_doc->sendOpword(opword);
}

void MainWindow::resetSession()
{
	auto dlg = new dialogs::ResetDialog(m_doc->canvas()->paintEngine(), this);
	dlg->setWindowModality(Qt::WindowModal);
	dlg->setAttribute(Qt::WA_DeleteOnClose);

	// It's always possible to create a new document from a snapshot
	connect(dlg, &dialogs::ResetDialog::newSelected, this, [dlg]() {
		MainWindow *w = new MainWindow(false);
		w->m_doc->sendResetSession(dlg->getResetImage());
		dlg->deleteLater();
	});

	// Session resetting is available only to session operators
	if(m_doc->canvas()->aclState()->amOperator()) {
		connect(dlg, &dialogs::ResetDialog::resetSelected, this, [this, dlg]() {
			if(m_doc->canvas()->aclState()->amOperator()) {
				m_doc->sendResetSession(dlg->getResetImage());
			}
			dlg->deleteLater();
		});

	} else {
		dlg->setCanReset(false);
	}

	dlg->show();
}

void MainWindow::terminateSession()
{
	auto dlg = new QMessageBox(
		QMessageBox::Question,
		tr("Terminate session"),
		tr("Really terminate this session?"),
		QMessageBox::Ok|QMessageBox::Cancel,
		this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setDefaultButton(QMessageBox::Cancel);
	dlg->button(QMessageBox::Ok)->setText(tr("Terminate"));
	dlg->setWindowModality(Qt::WindowModal);

	connect(dlg, &QMessageBox::finished, this, [this](int res) {
		if(res == QMessageBox::Ok)
			m_doc->sendTerminateSession();
	});

	dlg->show();
}

/**
 * @param url URL
 */
void MainWindow::joinSession(const QUrl& url, const QString &autoRecordFile)
{
	if(!canReplace()) {
		MainWindow *win = new MainWindow(false);
		Q_ASSERT(win->canReplace());
		win->joinSession(url, autoRecordFile);
		return;
	}

	net::LoginHandler *login = new net::LoginHandler(net::LoginHandler::Mode::Join, url, this);
	auto *dlg = new dialogs::LoginDialog(login, this);
	connect(m_doc, &Document::catchupProgress, dlg, &dialogs::LoginDialog::catchupProgress);
	connect(m_doc, &Document::serverLoggedIn, dlg, [dlg,this](bool join) {
		dlg->onLoginDone(join);
		m_canvasscene->hideCanvas();
	});
	connect(dlg, &dialogs::LoginDialog::destroyed, m_canvasscene, &drawingboard::CanvasScene::showCanvas);

	dlg->show();
	m_doc->setRecordOnConnect(autoRecordFile);
	m_doc->client()->connectToServer(login);
}

/**
 * Now connecting to server
 */
void MainWindow::onServerConnected()
{
	// Enable connection related actions
	getAction("hostsession")->setEnabled(false);
	getAction("leavesession")->setEnabled(true);
	getAction("sessionsettings")->setEnabled(true);

	// Disable UI until login completes
	m_view->setEnabled(false);
	setDrawingToolsEnabled(false);
}

/**
 * Connection lost, so disable and enable some UI elements
 */
void MainWindow::onServerDisconnected(const QString &message, const QString &errorcode, bool localDisconnect)
{
	getAction("hostsession")->setEnabled(m_doc->canvas() != nullptr);
	getAction("leavesession")->setEnabled(false);
	getAction("sessionsettings")->setEnabled(false);
	getAction("reportabuse")->setEnabled(false);
	m_admintools->setEnabled(false);
	m_modtools->setEnabled(false);
	if(m_sessionSettings) {
		m_sessionSettings->close();
	}

	// Re-enable UI
	m_view->setEnabled(true);
	setDrawingToolsEnabled(true);

	// Display login error if not yet logged in
	if(!m_doc->client()->isLoggedIn() && !localDisconnect) {
		QMessageBox *msgbox = new QMessageBox(
			QMessageBox::Warning,
			QString(),
			tr("Could not connect to server"),
			QMessageBox::Ok,
			this
		);

		msgbox->setAttribute(Qt::WA_DeleteOnClose);
		msgbox->setWindowModality(Qt::WindowModal);
		msgbox->setInformativeText(message);

		if(errorcode == "SESSIONIDINUSE") {
			// We tried to host a session using with a vanity ID, but that
			// ID was taken. Show a button for quickly joining that session instead
			msgbox->setInformativeText(msgbox->informativeText() + "\n" + tr("Would you like to join the session instead?"));

			QAbstractButton *joinbutton = msgbox->addButton(tr("Join"), QMessageBox::YesRole);

			msgbox->removeButton(msgbox->button(QMessageBox::Ok));
			msgbox->addButton(QMessageBox::Cancel);

			QUrl url = m_doc->client()->sessionUrl(true);

			connect(joinbutton, &QAbstractButton::clicked, this, [this, url]() {
				joinSession(url);
			});

		}

		// Work around Qt macOS bug(?): if a window has more than two modal dialogs (sheets)
		// open at the same time (in this case, the login dialog that hasn't closed yet)
		// the main window will still be stuck after the dialogs close.
		QTimer::singleShot(1, msgbox, &QMessageBox::show);
	}
	// If logged in but disconnected unexpectedly, show notification bar
	else if(m_doc->client()->isLoggedIn() && !localDisconnect) {
		m_view->showDisconnectedWarning(tr("Disconnected:") + " " + message);
	}
}

/**
 * Server connection established and login successfull
 */
void MainWindow::onServerLogin()
{
	m_netstatus->loggedIn(m_doc->client()->sessionUrl());
	m_netstatus->setSecurityLevel(m_doc->client()->securityLevel(), m_doc->client()->hostCertificate());
	m_view->setEnabled(true);
	updateSessionSettings();
	setDrawingToolsEnabled(true);
	m_modtools->setEnabled(m_doc->client()->isModerator());
	getAction("reportabuse")->setEnabled(m_doc->client()->serverSupportsReports());
}

void MainWindow::updateSessionSettings()
{
	if(!m_sessionSettings || !m_doc || !m_doc->client())
		return;

	m_sessionSettings->setPersistenceEnabled(m_doc->client()->serverSuppotsPersistence());
	m_sessionSettings->setAutoResetEnabled(m_doc->client()->sessionSupportsAutoReset());
	m_sessionSettings->setAuthenticated(m_doc->client()->isAuthenticated());
}

void MainWindow::showSessionSettings()
{
	if(!m_sessionSettings) {
		m_sessionSettings = new dialogs::SessionSettingsDialog(m_doc, this);
		updateSessionSettings();
	}

	m_sessionSettings->show();
}

void MainWindow::updateLockWidget()
{
	bool locked = m_doc->canvas() && m_doc->canvas()->aclState()->amLocked();
	getAction("locksession")->setChecked(locked);

	locked |= m_dockLayers->isCurrentLayerLocked();

	if(locked) {
		m_lockstatus->setPixmap(icon::fromTheme("object-locked").pixmap(16, 16));
		m_lockstatus->setToolTip(tr("Board is locked"));
	} else {
		m_lockstatus->setPixmap(QPixmap());
		m_lockstatus->setToolTip(QString());
	}
	m_view->setLocked(locked);
}

void MainWindow::onNsfmChanged(bool nsfm)
{
	if(nsfm && parentalcontrols::level() >= parentalcontrols::Level::Restricted) {
		m_doc->client()->disconnectFromServer();
		showErrorMessage(tr("Session blocked by parental controls"));
	}
}

void MainWindow::onOperatorModeChange(bool op)
{
	m_admintools->setEnabled(op);
	m_serverLogDialog->setOperatorMode(op);
	getAction("gainop")->setEnabled(!op && m_doc->isSessionOpword());
}

void MainWindow::onFeatureAccessChange(canvas::Feature feature, bool canUse)
{
	switch(feature) {
	case canvas::Feature::PutImage:
		m_putimagetools->setEnabled(canUse);
		getAction("toolfill")->setEnabled(canUse);
		break;
	case canvas::Feature::Resize:
		m_resizetools->setEnabled(canUse);
		break;
	case canvas::Feature::Background:
		m_canvasbgtools->setEnabled(canUse);
		break;
	case canvas::Feature::Laser:
		getAction("toollaser")->setEnabled(canUse && getAction("showlasers")->isChecked());
		break;
	case canvas::Feature::Undo:
		m_undotools->setEnabled(canUse);
		break;
	case canvas::Feature::Timeline:
		m_dockTimeline->setFeatureAccess(canUse);
		break;
	default: break;
	}
}

/**
 * Write settings and exit. The application will not be terminated until
 * the last mainwindow is closed.
 */
void MainWindow::exit()
{
	if(windowState().testFlag(Qt::WindowFullScreen))
		toggleFullscreen();
	setDocksHidden(false);
	writeSettings();
	deleteLater();
}

/**
 * @param message error message
 * @param details error details
 */
void MainWindow::showErrorMessage(const QString& message, const QString& details)
{
	if(message.isEmpty())
		return;

	QMessageBox *msgbox = new QMessageBox(
		QMessageBox::Warning,
		QString(),
		message, QMessageBox::Ok,
		this,
		Qt::Dialog|Qt::Sheet|Qt::MSWindowsFixedSizeDialogHint
	);
	msgbox->setAttribute(Qt::WA_DeleteOnClose);
	msgbox->setWindowModality(Qt::WindowModal);
	msgbox->setInformativeText(details);
	msgbox->show();
}

void MainWindow::showErrorMessage(rustpile::CanvasIoError error)
{
	QString msg;
	switch(error) {
	case rustpile::CanvasIoError::NoError: return;
	case rustpile::CanvasIoError::FileOpenError:
		msg = tr("Couldn't open file");
		break;
	case rustpile::CanvasIoError::FileIoError:
		msg = tr("Something went wrong with the file");
		break;
	case rustpile::CanvasIoError::UnsupportedFormat:
		msg = tr("This file format is not supported");
		break;
	case rustpile::CanvasIoError::PartiallySupportedFormat:
		msg = tr("This file is only partially supported. It may not appear as it should.");
		break;
	case rustpile::CanvasIoError::UnknownRecordingVersion:
		msg = tr("This recording was made with an unknown version. It may not appear as it should.");
		break;
	case rustpile::CanvasIoError::CodecError:
		msg = tr("Couldn't decode image");
		break;
	case rustpile::CanvasIoError::PaintEngineCrashed:
		msg = tr("Paint engine has crashed! Save your work and restart the application.");
		break;
	}

	showErrorMessage(msg);
}

void MainWindow::setShowAnnotations(bool show)
{
	QAction *annotationtool = getAction("tooltext");
	annotationtool->setEnabled(show);
	m_canvasscene->showAnnotations(show);
	if(!show) {
		if(annotationtool->isChecked())
			getAction("toolbrush")->trigger();
	}
}

void MainWindow::setShowLaserTrails(bool show)
{
	QAction *lasertool = getAction("toollaser");
	lasertool->setEnabled(show);
	m_canvasscene->showLaserTrails(show);
	if(!show) {
		if(lasertool->isChecked())
			getAction("toolbrush")->trigger();
	}
}

/**
 * @brief Enter/leave fullscreen mode
 *
 * Window position and configuration is saved when entering fullscreen mode
 * and restored when leaving
 *
 * @param enable
 */
void MainWindow::toggleFullscreen()
{
	if(windowState().testFlag(Qt::WindowFullScreen)==false) {
		// Save windowed mode state
		m_fullscreenOldGeometry = geometry();
		m_fullscreenOldMaximized = isMaximized();

		menuBar()->hide();
		m_view->setFrameShape(QFrame::NoFrame);
		m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		showFullScreen();

	} else {
		// Restore old state
		if(m_fullscreenOldMaximized) {
			showMaximized();
		} else {
			showNormal();
			setGeometry(m_fullscreenOldGeometry);
		}
		menuBar()->show();
		m_view->setFrameShape(QFrame::StyledPanel);

		m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}
}

void MainWindow::hotBorderMenubar(bool show)
{
	if(windowState().testFlag(Qt::WindowFullScreen)) {
		menuBar()->setVisible(show);
	}
}

void MainWindow::setFreezeDocks(bool freeze)
{
	const auto features = QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable;
	for(QObject *c : children()) {
		QDockWidget *dw = qobject_cast<QDockWidget*>(c);
		if(dw) {
			if(freeze)
				dw->setFeatures(dw->features() & ~features);
			else
				dw->setFeatures(dw->features() | features);
		}
	}
}

void MainWindow::setDocksHidden(bool hidden)
{
	int xOffset1=0, xOffset2=0, yOffset=0;

	for(QObject *c : children()) {
		QWidget *w = qobject_cast<QWidget*>(c);
		if(w && (w->inherits("QDockWidget") || w->inherits("QToolBar"))) {
			bool visible = w->isVisible();

			if(hidden) {
				w->setProperty("wasVisible", w->isVisible());
				w->hide();
			} else {
				const QVariant v = w->property("wasVisible");
				if(!v.isNull()) {
					w->setVisible(v.toBool());
					visible = v.toBool();
				}
			}

			QToolBar *tb = qobject_cast<QToolBar*>(w);
			if(tb && visible && !tb->isFloating()) {
				if(toolBarArea(tb) == Qt::TopToolBarArea)
					yOffset = tb->height();
				else if(toolBarArea(tb) == Qt::LeftToolBarArea)
					xOffset1 = tb->width();
			}

			QDockWidget *dw = qobject_cast<QDockWidget*>(w);
			if(dw && visible && !dw->isFloating() && dockWidgetArea(dw) == Qt::LeftDockWidgetArea)
				xOffset2 = dw->width();
		}
	}

	m_viewStatusBar->setHidden(hidden);

	// Docks can only dock on the left or right, so only one yOffset is needed.
	const int dir = hidden ? -1 : 1;
	m_view->scrollBy(dir * (xOffset1+xOffset2), dir * yOffset);
}

/**
 * User selected a tool
 * @param tool action representing the tool
 */
void MainWindow::selectTool(QAction *tool)
{
	// Note. Actions must be in the same order in the enum and the group
	int idx = m_drawingtools->actions().indexOf(tool);
	Q_ASSERT(idx>=0);
	if(idx<0)
		return;

	if(m_dockToolSettings->currentTool() == idx) {
		if(QSettings().value("settings/tooltoggle", true).toBool())
			m_dockToolSettings->setPreviousTool();
		m_tempToolSwitchShortcut->reset();

	} else {
		m_dockToolSettings->setTool(tools::Tool::Type(idx));
		m_toolChangeTime.start();
		m_lastToolBeforePaste = -1;
	}
}

/**
 * @brief Handle tool change
 * @param tool
 */
void MainWindow::toolChanged(tools::Tool::Type tool)
{
	QAction *toolaction = m_drawingtools->actions().at(int(tool));
	toolaction->setChecked(true);

	// When using the annotation tool, highlight all text boxes
	m_canvasscene->showAnnotationBorders(tool==tools::Tool::ANNOTATION);

	// Send pointer updates when using the laser pointer (TODO checkbox)
	m_view->setPointerTracking(tool==tools::Tool::LASERPOINTER && static_cast<tools::LaserPointerSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::LASERPOINTER))->pointerTracking());

	// Remove selection when not using selection tool
	if(tool != tools::Tool::SELECTION && tool != tools::Tool::POLYGONSELECTION)
		m_doc->selectNone();

	// Deselect annotation when tool changed
	if(tool != tools::Tool::ANNOTATION)
		m_doc->toolCtrl()->setActiveAnnotation(0);

	m_doc->toolCtrl()->setActiveTool(tool);
}

void MainWindow::selectionRemoved()
{
	if(m_lastToolBeforePaste>=0) {
		// Selection was just removed and we had just pasted an image
		// so restore the previously used tool
		QAction *toolaction = m_drawingtools->actions().at(m_lastToolBeforePaste);
		toolaction->trigger();
	}
}

void MainWindow::copyText()
{
	// Attempt to copy text if a text widget has focus
	QWidget *focus = QApplication::focusWidget();

	auto *textedit = qobject_cast<QTextEdit*>(focus);
	if(textedit)
		textedit->copy();
}

void MainWindow::paste()
{
	const QMimeData *mimeData = QApplication::clipboard()->mimeData();
	if(mimeData->hasImage()) {
		QPoint pastepos;
		bool pasteAtPos = false;

		// Get source position
		QByteArray srcpos = mimeData->data("x-drawpile/pastesrc");
		if(!srcpos.isNull()) {
			QList<QByteArray> pos = srcpos.split(',');
			if(pos.size() == 2) {
				bool ok1, ok2;
				pastepos = QPoint(pos.at(0).toInt(&ok1), pos.at(1).toInt(&ok2));
				pasteAtPos = ok1 && ok2;
			}
		}

		// Paste-in-place if source was Drawpile (and source is visible)
		if(pasteAtPos && m_view->isPointVisible(pastepos))
			pasteImage(mimeData->imageData().value<QImage>(), &pastepos);
		else
			pasteImage(mimeData->imageData().value<QImage>());
	}
}

void MainWindow::pasteFile()
{
	const QString file = QFileDialog::getOpenFileName(
		this,
		tr("Paste Image"),
		getLastPath(),
		// note: Only Qt's native image formats are supported at the moment
		utils::fileFormatFilter(utils::FileFormatOption::OpenImages | utils::FileFormatOption::QtImagesOnly)
	);

	if(file.isEmpty()==false) {
		const QFileInfo info(file);
		setLastPath(info.absolutePath());

		pasteFile(QUrl::fromLocalFile(file));
	}
}

void MainWindow::pasteFile(const QUrl &url)
{
	if(url.isLocalFile()) {
		QImage img(url.toLocalFile());
		if(img.isNull()) {
			showErrorMessage(tr("The image could not be loaded"));
			return;
		}

		pasteImage(img);
	} else {
		auto *filedownload = new networkaccess::FileDownload(this);

		filedownload->setExpectedType("image/");

		connect(filedownload, &networkaccess::FileDownload::progress, m_netstatus, &widgets::NetStatus::setDownloadProgress);
		connect(filedownload, &networkaccess::FileDownload::finished, this, [this, filedownload](const QString &errorMessage) {
			m_netstatus->hideDownloadProgress();
			filedownload->deleteLater();

			if(errorMessage.isEmpty()) {
				QImageReader reader(filedownload->file());
				QImage image = reader.read();

				if(image.isNull())
					showErrorMessage(reader.errorString());
				else
					pasteImage(image);

			} else {
				showErrorMessage(errorMessage);
			}
		});

		filedownload->start(url);
	}
}

void MainWindow::pasteImage(const QImage &image, const QPoint *point)
{
	if(!m_canvasscene->model()->aclState()->canUseFeature(canvas::Feature::PutImage))
		return;

	if(m_dockToolSettings->currentTool() != tools::Tool::SELECTION && m_dockToolSettings->currentTool() != tools::Tool::POLYGONSELECTION) {
		int currentTool = m_dockToolSettings->currentTool();
		getAction("toolselectrect")->trigger();
		m_lastToolBeforePaste = currentTool;
	}

	QPoint p;
	bool force;
	if(point) {
		p = *point;
		force = true;
	} else {
		p = m_view->viewCenterPoint();
		force = false;
	}

	m_doc->pasteImage(image, p, force);
}

void MainWindow::dropUrl(const QUrl &url)
{
	if(m_canvasscene->hasImage())
		pasteFile(url);
	else
		open(url);
}

void MainWindow::clearOrDelete()
{
	// This slot is triggered in response to the 'Clear' action, which
	// which in turn can be triggered via the 'Delete' shortcut. In annotation
	// editing mode, the current selection may be an annotation, so we should delete
	// that instead of clearing out the canvas.
	QAction *annotationtool = getAction("tooltext");
	if(annotationtool->isChecked()) {
		const uint16_t a = static_cast<tools::AnnotationSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::ANNOTATION))->selected();
		if(a>0) {
			net::EnvelopeBuilder eb;
			rustpile::write_undopoint(eb, m_doc->client()->myId());
			rustpile::write_deleteannotation(eb, m_doc->client()->myId(), a);
			m_doc->client()->sendEnvelope(eb.toEnvelope());
			return;
		}
	}

	// No annotation selected: clear seleted area as usual
	m_doc->fillArea(Qt::white, rustpile::Blendmode::Erase);
}

void MainWindow::resizeCanvas()
{
	if(!m_doc->canvas()) {
		qWarning("resizeCanvas: no canvas!");
		return;
	}

	const QSize size = m_doc->canvas()->size();
	dialogs::ResizeDialog *dlg = new dialogs::ResizeDialog(size, this);
	dlg->setPreviewImage(m_doc->canvas()->paintEngine()->getPixmap().scaled(300, 300, Qt::KeepAspectRatio).toImage());
	dlg->setAttribute(Qt::WA_DeleteOnClose);

	// Preset crop from selection if one exists
	if (m_doc->canvas()->selection()) {
		dlg->setBounds(m_doc->canvas()->selection()->boundingRect());
	}

	connect(dlg, &QDialog::accepted, this, [this, dlg]() {
		if (m_doc->canvas()->selection()) {
			m_doc->canvas()->setSelection(nullptr);
		}
		dialogs::ResizeVector r = dlg->resizeVector();
		if(!r.isZero()) {
			m_doc->sendResizeCanvas(r.top, r.right, r.bottom, r.left);
		}
	});
	dlg->show();
}

void MainWindow::changeCanvasBackground()
{
	if(!m_doc->canvas()) {
		qWarning("changeCanvasBackground: no canvas!");
		return;
	}
	auto *dlg = new color_widgets::ColorDialog(this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setColor(m_doc->canvas()->paintEngine()->backgroundColor());

	connect(dlg, &color_widgets::ColorDialog::colorSelected, m_doc, &Document::sendCanvasBackground);
	dlg->show();
}

void MainWindow::about()
{
#ifdef BUILD_LABEL
	const QString version = DRAWPILE_VERSION " (" BUILD_LABEL ")";
#else
	const QString version = DRAWPILE_VERSION;
#endif
	QMessageBox::about(nullptr, tr("About Drawpile"),
			QStringLiteral("<p><b>Drawpile %1</b><br>").arg(version) +
			tr("A collaborative drawing program.") + QStringLiteral("</p>"

			"<p>Copyright © Calle Laakkonen and Drawpile contributors</p>"

			"<p>This program is free software; you may redistribute it and/or "
			"modify it under the terms of the GNU General Public License as "
			"published by the Free Software Foundation, either version 3, or "
			"(at your opinion) any later version.</p>"

			"<p>This program is distributed in the hope that it will be useful, "
			"but WITHOUT ANY WARRANTY; without even the implied warranty of "
			"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
			"GNU General Public License for more details.</p>"

			"<p>You should have received a copy of the GNU General Public License "
			"along with this program.  If not, see <a href=\"http://www.gnu.org/licences/\">http://www.gnu.org/licenses/</a>.</p>"
			)
	);
}

void MainWindow::homepage()
{
	QDesktopServices::openUrl(QUrl(WEBSITE));
}

/**
 * @brief Create a new action.
 *
 * All created actions are added to a list that is used in the
 * settings dialog to edit the shortcuts.
 */
ActionBuilder MainWindow::makeAction(const char *text, const char *name)
{
	return makeAction(text, name, this)
		// Add this action to the mainwindow so its shortcut can be used
		// even when the menu/toolbar is not visible
		.addTo(this);
}

ActionBuilder MainWindow::makeAction(const char *text, const char *name, QObject *parent)
{
	Q_ASSERT(text);
	Q_ASSERT(name);
	return ActionBuilder(parent, &MainWindow::tr)
		.text(text)
		.objectName(name)
		.autoRepeat(false);
}

MenuBuilder MainWindow::makeMenu(const char *title, QWidget *parent)
{
	Q_ASSERT(title);
	return MenuBuilder(parent, &MainWindow::tr).title(title);
}

QToolBar *MainWindow::makeToolBar(const char *title, QWidget *parent)
{
	if (!parent) {
		parent = this;
	}

	QToolBar *t = new QToolBar(parent);
	AUTO_TR(t, setWindowTitle, tr(title));
	return t;
}

QAction *MainWindow::getAction(const QString &name)
{
	QAction *a = findChild<QAction*>(name, Qt::FindDirectChildrenOnly);
	if(!a)
		qFatal("%s: no such action", qPrintable(name));
	Q_ASSERT(a);
	return a;
}

/**
 * @brief Create actions, menus and toolbars
 */
void MainWindow::setupActions()
{
	Q_ASSERT(m_doc);
	Q_ASSERT(m_dockLayers);

	QMenu *toggledockmenu = makeMenu(QT_TR_NOOP("&Docks"), this);

	// Collect list of docks for dock menu
	for(auto *dw : findChildren<QDockWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
		toggledockmenu->addAction(dw->toggleViewAction());
	}

	toggledockmenu->addSeparator();
	makeAction(QT_TR_NOOP("Lock in place"), "freezedocks")
		.checkable()
		.remembered()
		.addTo(toggledockmenu)
		.onTriggered(this, &MainWindow::setFreezeDocks);

	makeAction(QT_TR_NOOP("Hide Docks"), "hidedocks")
		.checkable()
		.shortcut("tab")
		.addTo(toggledockmenu)
		.onTriggered(this, &MainWindow::setDocksHidden);

	// File menu and toolbar
	QToolBar *filetools = makeToolBar(QT_TR_NOOP("File Tools"));
	filetools->setObjectName("filetoolsbar");

	QMenu *toggletoolbarmenu = makeMenu(QT_TR_NOOP("&Toolbars"), this)
		.action(filetools->toggleViewAction());

	addToolBar(Qt::TopToolBarArea, filetools);

	m_currentdoctools = new QActionGroup(this);
	m_currentdoctools->setExclusive(false);
	m_currentdoctools->setEnabled(false);

	makeMenu(QT_TR_NOOP("&File"), menuBar())
		.action(makeAction(QT_TR_NOOP("&New"), "newdocument")
			.icon("document-new")
			.shortcut(QKeySequence::New)
			.onTriggered(this, &MainWindow::showNew)
			.addTo(filetools)
		)
		.action(makeAction(QT_TR_NOOP("&Open..."), "opendocument")
			.icon("document-open")
			.shortcut(QKeySequence::Open)
			.onTriggered(this, QOverload<>::of(&MainWindow::open))
			.addTo(filetools)
		)
		.submenu([=](MenuBuilder menu) {
			m_recentMenu = menu
				.title(QT_TR_NOOP("Open &Recent"))
				.onTriggered([=](QAction *action) {
					auto filepath = action->property("filepath").toString();
					this->open(QUrl::fromLocalFile(filepath));
				});
		})
		.separator()
#ifdef Q_OS_MACOS
		.action(makeAction(QT_TR_NOOP("Close"), "closedocument")
			.shortcut(QKeySequence::Close)
			.addTo(m_currentdoctools)
			.onTriggered(this, &MainWindow::close)
		)
#endif
		.action(makeAction(QT_TR_NOOP("&Save"), "savedocument")
			.icon("document-save")
			.shortcut(QKeySequence::Save)
			.addTo(m_currentdoctools)
			.addTo(filetools)
			.onTriggered(this, &MainWindow::save)
		)
		.action(makeAction(QT_TR_NOOP("Save &As..."), "savedocumentas")
			.icon("document-save-as")
			.shortcut(QKeySequence::SaveAs)
			.addTo(m_currentdoctools)
			.onTriggered(this, &MainWindow::saveas)
		)
		.action(makeAction(QT_TR_NOOP("Save Selection..."), "saveselection")
			.icon("document-save-as")
			.addTo(m_currentdoctools)
			.onTriggered(this, &MainWindow::saveSelection)
		)
		.action(makeAction(QT_TR_NOOP("Autosave"), "autosave")
			.checkable()
			.disabled()
			.onTriggered(m_doc, &Document::setAutosave)
			.on(m_doc, &Document::autosaveChanged, &QAction::setChecked)
			.on(m_doc, &Document::canAutosaveChanged, &QAction::setEnabled)
		)
		.separator()
		.submenu([=](MenuBuilder menu) {
			menu.title(QT_TR_NOOP("&Export"))
				.icon("document-export")
				.action(
					makeAction(QT_TR_NOOP("Animated &GIF..."), "exportanimgif")
					.addTo(m_currentdoctools)
					.onTriggered(this, &MainWindow::exportGifAnimation)
				)
				.action(
					makeAction(QT_TR_NOOP("Animation &Frames..."), "exportanimframes")
					.addTo(m_currentdoctools)
					.onTriggered(this, &MainWindow::exportAnimationFrames)
				);
		})
		.action(makeAction(QT_TR_NOOP("Record..."), "recordsession")
			.icon("media-record")
			.addTo(m_currentdoctools)
			.addTo(filetools)
			.onTriggered(this, &MainWindow::toggleRecording)
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("&Quit"), "exitprogram")
			.icon("application-exit")
			.shortcut("Ctrl+Q")
			.menuRole(QAction::QuitRole)
#ifdef Q_OS_MACOS
			.onTriggered(MacMenu::instance(), &MacMenu::quitAll)
#else
			.onTriggered(this, &MainWindow::close)
#endif
		);

	// Edit menu and toolbar
	QToolBar *edittools = makeToolBar(QT_TR_NOOP("Edit Tools"));
	edittools->setObjectName("edittoolsbar");
	toggletoolbarmenu->addAction(edittools->toggleViewAction());
	addToolBar(Qt::TopToolBarArea, edittools);

	m_canvasbgtools = new QActionGroup(this);
	m_canvasbgtools->setEnabled(false);

	m_resizetools = new QActionGroup(this);
	m_resizetools->setEnabled(false);

	m_putimagetools = new QActionGroup(this);
	m_putimagetools->setEnabled(false);

	m_undotools = new QActionGroup(this);
	m_undotools->setEnabled(false);

	makeMenu(QT_TR_NOOP("&Edit"), menuBar())
		.action(makeAction(QT_TR_NOOP("&Undo"), "undo")
			.icon("edit-undo")
			.shortcut(QKeySequence::Undo)
			.addTo(m_undotools)
			.addTo(edittools)
			.onTriggered(m_doc, &Document::undo)
		)
		.action(makeAction(QT_TR_NOOP("&Redo"), "redo")
			.icon("edit-redo")
			.shortcut(QKeySequence::Redo)
			.addTo(m_undotools)
			.addTo(edittools)
			.onTriggered(m_doc, &Document::redo)
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("Cu&t Layer"), "cutlayer")
			.icon("edit-cut")
			.statusTip(QT_TR_NOOP("Cut selected area of the current layer to the clipboard"))
			.shortcut(QKeySequence::Cut)
			.addTo(edittools)
			.addTo(m_putimagetools)
			.onTriggered(m_doc, &Document::cutLayer)
		)
		.action(makeAction(QT_TR_NOOP("&Copy Visible"), "copyvisible")
			.icon("edit-copy")
			.statusTip(QT_TR_NOOP("Copy selected area to the clipboard"))
			.shortcut("Shift+Ctrl+C")
			.addTo(m_currentdoctools)
			.onTriggered(m_doc, &Document::copyVisible)
		)
		.action(makeAction(QT_TR_NOOP("Copy Merged"), "copymerged")
			.icon("edit-copy")
			.statusTip(QT_TR_NOOP("Copy selected area, excluding the background, to the clipboard"))
			.shortcut("Ctrl+Alt+C")
			.onTriggered(m_doc, &Document::copyMerged)
		)
		.action(makeAction(QT_TR_NOOP("Copy &Layer"), "copylayer")
			.icon("edit-copy")
			.statusTip(QT_TR_NOOP("Copy selected area of the current layer to the clipboard"))
			.shortcut(QKeySequence::Copy)
			.addTo(m_currentdoctools)
			.addTo(edittools)
			.onTriggered(m_doc, &Document::copyLayer)
		)
		.action(makeAction(QT_TR_NOOP("&Paste"), "paste")
			.icon("edit-paste")
			.shortcut(QKeySequence::Paste)
			.addTo(edittools)
			.addTo(m_putimagetools)
			.onTriggered(this, &MainWindow::paste)
		)
		.action(makeAction(QT_TR_NOOP("Paste &From File..."), "pastefile")
			.icon("document-open")
			.addTo(m_putimagetools)
			.onTriggered(this, QOverload<>::of(&MainWindow::pasteFile))
		)
		.action(makeAction(QT_TR_NOOP("&Stamp"), "stamp")
			.shortcut("Ctrl+T")
			.addTo(m_putimagetools)
			.onTriggered(m_doc, &Document::stamp)
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("Select &All"), "selectall")
			.shortcut(QKeySequence::SelectAll)
			.addTo(m_currentdoctools)
			.onTriggered([=]() {
				auto *selectRect = getAction("toolselectrect");
				if(!selectRect->isChecked())
					selectRect->trigger();
				m_doc->selectAll();
			})
		)
		.action(makeAction(QT_TR_NOOP("&Deselect"), "selectnone")
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN) // Deselect is not defined on Mac and Win
			.shortcut("Shift+Ctrl+A")
#else
			.shortcut(QKeySequence::Deselect)
#endif
			.addTo(m_currentdoctools)
			.onTriggered(m_doc, &Document::selectNone)
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("Resi&ze Canvas..."), "resizecanvas")
			.addTo(m_resizetools)
			.onTriggered(this, &MainWindow::resizeCanvas)
		)
		.submenu([=](MenuBuilder menu) {
			menu
			.title(QT_TR_NOOP("&Expand Canvas"))
			.action(makeAction(QT_TR_NOOP("Expand &Up"), "expandup")
				.shortcut(CTRL_KEY | Qt::Key_J)
				.autoRepeat(true)
				.addTo(m_resizetools)
				.onTriggered([=] {
					m_doc->sendResizeCanvas(64, 0, 0, 0);
				})
			)
			.action(makeAction(QT_TR_NOOP("Expand &Down"), "expanddown")
				.shortcut(CTRL_KEY | Qt::Key_K)
				.autoRepeat(true)
				.addTo(m_resizetools)
				.onTriggered([=] {
					m_doc->sendResizeCanvas(0, 0, 64, 0);
				})
			)
			.action(makeAction(QT_TR_NOOP("Expand &Left"), "expandleft")
				.shortcut(CTRL_KEY | Qt::Key_H)
				.autoRepeat(true)
				.addTo(m_resizetools)
				.onTriggered([=] {
					m_doc->sendResizeCanvas(0, 0, 0, 64);
				})
			)
			.action(makeAction(QT_TR_NOOP("Expand &Right"), "expandright")
				.shortcut(CTRL_KEY | Qt::Key_L)
				.autoRepeat(true)
				.addTo(m_resizetools)
				.onTriggered([=] {
					m_doc->sendResizeCanvas(0, 64, 0, 0);
				})
			);
		})
		.action(makeAction(QT_TR_NOOP("Set Background..."), "canvas-background")
			.onTriggered(this, &MainWindow::changeCanvasBackground)
			.addTo(m_canvasbgtools)
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("Delete Empty Annotations"), "deleteemptyannotations")
			.addTo(m_currentdoctools)
			.onTriggered(m_doc, &Document::removeEmptyAnnotations)
		)
		.action(makeAction(QT_TR_NOOP("Delete"), "cleararea")
			.shortcut(QKeySequence::Delete)
			.addTo(m_putimagetools)
			.onTriggered(this, &MainWindow::clearOrDelete)
		)
		.action(makeAction(QT_TR_NOOP("Fill Selection"), "fillfgarea")
			.shortcut(CTRL_KEY | Qt::Key_Comma)
			.addTo(m_putimagetools)
			.onTriggered([=] {
				m_doc->fillArea(m_dockToolSettings->foregroundColor(), rustpile::Blendmode::Normal);
			})
		)
		.action(makeAction(QT_TR_NOOP("Recolor Selection"), "recolorarea")
			.shortcut(CTRL_KEY | Qt::SHIFT | Qt::Key_Comma)
			.addTo(m_putimagetools)
			.onTriggered([=] {
				m_doc->fillArea(m_dockToolSettings->foregroundColor(), rustpile::Blendmode::Recolor);
			})
		)
		.action(makeAction(QT_TR_NOOP("Color Erase Selection"), "colorerasearea")
			.shortcut(Qt::SHIFT | Qt::Key_Delete)
			.addTo(m_putimagetools)
			.onTriggered([=] {
				m_doc->fillArea(m_dockToolSettings->foregroundColor(), rustpile::Blendmode::ColorErase);
			})
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("Prefere&nces"), "preferences")
			.menuRole(QAction::PreferencesRole)
			.onTriggered(this, &MainWindow::showSettings)
		);

	// View menu
	QAction *viewflip = makeAction(QT_TR_NOOP("Flip"), "viewflip")
		.icon("object-flip-vertical")
		.shortcut("C")
		.checkable()
		.onTriggered(m_view, &widgets::CanvasView::setViewFlip);

	QAction *viewmirror = makeAction(QT_TR_NOOP("Mirror"), "viewmirror")
		.icon("object-flip-horizontal")
		.shortcut("V")
		.checkable()
		.onTriggered(m_view, &widgets::CanvasView::setViewMirror);

	QAction *rotateorig = makeAction(QT_TR_NOOP("&Reset Rotation"), "rotatezero")
		.icon("transform-rotate")
		.shortcut("ctrl+r")
		.onTriggered([=] { m_view->setRotation(0); });

	QAction *zoomorig = makeAction(QT_TR_NOOP("&Normal Size"), "zoomone")
		.icon("zoom-original")
		.shortcut("ctrl+0")
		.onTriggered([=] { m_view->setZoom(100.0); });

#ifndef Q_OS_MACOS // macOS has its own full screen toggle
	QAction *fullscreen = makeAction(QT_TR_NOOP("&Full Screen"), "fullscreen")
		.shortcut(QKeySequence::FullScreen)
		.checkable()
		.onTriggered(this, &MainWindow::toggleFullscreen);
	connect(windowHandle(), &QWindow::windowStateChanged, [=](Qt::WindowState state) {
		fullscreen->setChecked(state & Qt::WindowFullScreen);
	});
#endif

	makeMenu(QT_TR_NOOP("&View"), menuBar())
		.submenu(toggletoolbarmenu)
		.submenu(toggledockmenu)
		.action(makeAction(QT_TR_NOOP("Chat"), "togglechat")
			.shortcut("Alt+C")
			.checked()
			.on(m_statusChatButton, &QToolButton::clicked, &QAction::trigger)
			.on(m_chatbox, &widgets::ChatBox::expandedChanged, &QAction::setChecked)
			.on(m_chatbox, &widgets::ChatBox::expandPlease, &QAction::trigger)
			.onTriggered(this, &MainWindow::toggleChat)
		)
		.action(makeAction(QT_TR_NOOP("Flipbook"), "showflipbook")
			.statusTip(QT_TR_NOOP("Show animation preview window"))
			.shortcut("Ctrl+F")
			.addTo(m_currentdoctools)
			.onTriggered(this, &MainWindow::showFlipbook)
		)
		.separator()
		.submenu([=](MenuBuilder menu) {
			menu
			.title(QT_TR_NOOP("&Zoom"))
			.action(makeAction(QT_TR_NOOP("Zoom &In"), "zoomin")
				.icon("zoom-in")
				.shortcut(QKeySequence::ZoomIn)
				.autoRepeat(true)
				.onTriggered(m_view, &widgets::CanvasView::zoomin)
			)
			.action(makeAction(QT_TR_NOOP("Zoom &Out"), "zoomout")
				.icon("zoom-out")
				.shortcut(QKeySequence::ZoomOut)
				.autoRepeat(true)
				.onTriggered(m_view, &widgets::CanvasView::zoomout)
			)
			.action(zoomorig);
		})
		.submenu([=](MenuBuilder menu) {
			menu
			.title(QT_TR_NOOP("Rotation"))
			.action(rotateorig)
			.action(makeAction(QT_TR_NOOP("Rotate Canvas Clockwise"), "rotatecw")
				.icon("object-rotate-right")
				.autoRepeat(true)
				.shortcut("shift+.")
				.onTriggered([=] { m_view->setRotation(m_view->rotation() + 5); })
			)
			.action(makeAction(QT_TR_NOOP("Rotate Canvas Counterclockwise"), "rotateccw")
				.icon("object-rotate-left")
				.autoRepeat(true)
				.shortcut("shift+,")
				.onTriggered([=] { m_view->setRotation(m_view->rotation() - 5); })
			);
		})
		.action(viewflip)
		.action(viewmirror)
		.separator()
		.submenu([=](MenuBuilder menu) {
			menu
			.title(QT_TR_NOOP("User Pointers"))
			.action(makeAction(QT_TR_NOOP("Show User &Pointers"), "showusermarkers")
				.checked()
				.remembered()
				.onToggled(m_canvasscene, &drawingboard::CanvasScene::showUserMarkers)
			)
			.action(makeAction(QT_TR_NOOP("Show La&ser Trails"), "showlasers")
				.checked()
				.remembered()
				.onToggled(this, &MainWindow::setShowLaserTrails)
			)
			.separator()
			.action(makeAction(QT_TR_NOOP("Show Names"), "showmarkernames")
				.checked()
				.remembered()
				.onToggled(m_canvasscene, &drawingboard::CanvasScene::showUserNames)
			)
			.action(makeAction(QT_TR_NOOP("Show Layers"), "showmarkerlayers")
				.checked()
				.remembered()
				.onToggled(m_canvasscene, &drawingboard::CanvasScene::showUserLayers)
			)
			.action(makeAction(QT_TR_NOOP("Show Avatars"), "showmarkeravatars")
				.checked()
				.remembered()
				.onToggled(m_canvasscene, &drawingboard::CanvasScene::showUserAvatars)
			);
		})
		.action(makeAction(QT_TR_NOOP("Show &Annotations"), "showannotations")
			.checked()
			.remembered()
			.onTriggered(this, &MainWindow::setShowAnnotations)
		)
		.action(makeAction(QT_TR_NOOP("Show Pixel &Grid"), "showgrid")
			.checked()
			.remembered()
			.onToggled(m_view, &widgets::CanvasView::setPixelGrid)
		)
		.separator()
#ifndef Q_OS_MACOS // macOS has its own full screen toggle
		.action(fullscreen)
#endif
		;

	connect(m_chatbox, &widgets::ChatBox::expandedChanged, m_statusChatButton, &QToolButton::hide);

	m_viewstatus->setActions(viewflip, viewmirror, rotateorig, zoomorig);

	// Layer menu
	QAction *layerAdd = makeAction(QT_TR_NOOP("New Layer"), "layeradd")
		.icon("list-add")
		.shortcut("Shift+Ctrl+Insert");
	QAction *groupAdd = makeAction(QT_TR_NOOP("New Group"), "groupadd")
		.icon("folder-new");
	QAction *layerDupe = makeAction(QT_TR_NOOP("Duplicate Layer"), "layerdupe")
		.icon("edit-copy");
	QAction *layerMerge = makeAction(QT_TR_NOOP("Merge with Layer Below"), "layermerge")
		.icon("arrow-down-double");
	QAction *layerProperties = makeAction(QT_TR_NOOP("Properties..."), "layerproperties")
		.icon("configure");
	QAction *layerDelete = makeAction(QT_TR_NOOP("Delete Layer"), "layerdelete")
		.icon("list-remove");

	makeMenu(QT_TR_NOOP("Layer"), menuBar())
		.action(layerAdd)
		.action(groupAdd)
		.action(layerDupe)
		.action(layerMerge)
		.action(layerDelete)
		.separator()
		.action(makeAction(QT_TR_NOOP("Frame"), "layerviewframe")
			.shortcut("Home")
			.checkable()
			.onToggled(this, &MainWindow::updateLayerViewMode)
		)
		.action(makeAction(QT_TR_NOOP("Solo"), "layerviewsolo")
			.shortcut("Shift+Home")
			.checkable()
			.onToggled(this, &MainWindow::updateLayerViewMode)
		)
		.action(makeAction(QT_TR_NOOP("Onionskin"), "layerviewonionskin")
			.shortcut("Shift+Ctrl+O")
			.checkable()
			.onToggled(this, &MainWindow::updateLayerViewMode)
		)
		.action(makeAction(QT_TR_NOOP("Show Censored Layers"), "layerviewuncensor")
			.checkable()
			.remembered()
			.onToggled(this, &MainWindow::updateLayerViewMode)
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("Next Frame"), "frame-next")
			.shortcut("Shift+X")
			.onTriggered(m_dockTimeline, &docks::Timeline::setNextFrame)
		)
		.action(makeAction(QT_TR_NOOP("Previous Frame"), "frame-prev")
			.shortcut("Shift+Z")
			.onTriggered(m_dockTimeline, &docks::Timeline::setPreviousFrame)
		);

	m_dockLayers->setLayerEditActions(layerAdd, groupAdd, layerDupe, layerMerge, layerProperties, layerDelete);

	// Session menu
	m_admintools = new QActionGroup(this);
	m_admintools->setExclusive(false);

	m_modtools = new QActionGroup(this);
	m_modtools->setEnabled(false);

	QAction* gainop = makeAction(QT_TR_NOOP("Become Operator..."), "gainop")
		.disabled()
		.onTriggered(this, &MainWindow::tryToGainOp);
	connect(m_doc, &Document::sessionOpwordChanged, [=](bool hasOpword) {
		gainop->setEnabled(hasOpword && !m_doc->canvas()->aclState()->amOperator());
	});

	makeMenu(QT_TR_NOOP("&Session"), menuBar())
		.action(makeAction(QT_TR_NOOP("&Host..."), "hostsession")
			.statusTip(QT_TR_NOOP("Share your drawingboard with others"))
			.onTriggered(this, &MainWindow::host)
		)
		.action(makeAction(QT_TR_NOOP("&Join..."), "joinsession")
			.statusTip(QT_TR_NOOP("Join another user's drawing session"))
			.onTriggered([=] { join(); })
		)
		.action(makeAction(QT_TR_NOOP("&Leave"), "leavesession")
			.statusTip(QT_TR_NOOP("Leave this drawing session"))
			.disabled()
			.onTriggered(this, &MainWindow::leave)
		)
		.separator()
		.submenu([=](MenuBuilder menu) {
			menu
			.title(QT_TR_NOOP("Moderation"))
			.action(gainop)
			.action(makeAction(QT_TR_NOOP("Terminate"), "terminatesession")
				.addTo(m_modtools)
				.onTriggered(this, &MainWindow::terminateSession)
			)
			.action(makeAction(QT_TR_NOOP("Report..."), "reportabuse")
				.disabled()
				.onTriggered(this, &MainWindow::reportAbuse)
			);
		})
		.action(makeAction(QT_TR_NOOP("&Reset..."), "resetsession")
			.onTriggered(this, &MainWindow::resetSession)
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("Event Log"), "viewserverlog")
			.noDefaultShortcut()
			.onTriggered(m_serverLogDialog, &dialogs::ServerLogDialog::show)
		)
		.action(makeAction(QT_TR_NOOP("Settings..."), "sessionsettings")
			.noDefaultShortcut()
			.menuRole(QAction::NoRole)
			.disabled()
			.onTriggered(this, &MainWindow::showSessionSettings)
		)
		.action(makeAction(QT_TR_NOOP("Lock Everything"), "locksession")
			.statusTip(QT_TR_NOOP("Prevent changes to the drawing board"))
			.shortcut("F12")
			.checkable()
			.addTo(m_admintools)
			.onTriggered(m_doc, &Document::sendLockSession)
		);

	m_admintools->setEnabled(false);

	// Tools menu and toolbar
	QToolBar *drawtools = makeToolBar(QT_TR_NOOP("Drawing tools"));
	drawtools->setObjectName("drawtoolsbar");
	toggletoolbarmenu->addAction(drawtools->toggleViewAction());
	addToolBar(Qt::TopToolBarArea, drawtools);

	m_drawingtools = new QActionGroup(this);
	connect(m_drawingtools, &QActionGroup::triggered, this, &MainWindow::selectTool);

	makeMenu(QT_TR_NOOP("&Tools"), menuBar())
		.action(makeAction(QT_TR_NOOP("Freehand"), "toolbrush")
			.icon("draw-brush")
			.statusTip(QT_TR_NOOP("Freehand brush tool"))
			.shortcut("B")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("Eraser"), "tooleraser")
			.icon("draw-eraser")
			.statusTip(QT_TR_NOOP("Freehand eraser brush"))
			.shortcut("E")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Line"), "toolline")
			.icon("draw-line")
			.statusTip(QT_TR_NOOP("Draw straight lines"))
			.shortcut("U")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Rectangle"), "toolrect")
			.icon("draw-rectangle")
			.statusTip(QT_TR_NOOP("Draw unfilled squares and rectangles"))
			.shortcut("R")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Ellipse"), "toolellipse")
			.icon("draw-ellipse")
			.statusTip(QT_TR_NOOP("Draw unfilled circles and ellipses"))
			.shortcut("O")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("Bezier Curve"), "toolbezier")
			.icon("draw-bezier-curves")
			.statusTip(QT_TR_NOOP("Draw bezier curves"))
			.shortcut("Ctrl+B")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Flood Fill"), "toolfill")
			.icon("fill-color")
			.statusTip(QT_TR_NOOP("Fill areas"))
			.shortcut("F")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Annotation"), "tooltext")
			.icon("draw-text")
			.statusTip(QT_TR_NOOP("Add text to the picture"))
			.shortcut("A")
			.checked()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Color Picker"), "toolpicker")
			.icon("color-picker")
			.statusTip(QT_TR_NOOP("Pick colors from the image"))
			.shortcut("I")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Laser Pointer"), "toollaser")
			.icon("cursor-arrow")
			.statusTip(QT_TR_NOOP("Point out things on the canvas"))
			.shortcut("L")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Select (Rectangular)"), "toolselectrect")
			.icon("select-rectangular")
			.statusTip(QT_TR_NOOP("Select area for copying"))
			.shortcut("S")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("&Select (Free-Form)"), "toolselectpolygon")
			.icon("edit-select-lasso")
			.statusTip(QT_TR_NOOP("Select a free-form area for copying"))
			.shortcut("D")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("Zoom"), "toolzoom")
			.icon("zoom-select")
			.statusTip(QT_TR_NOOP("Zoom the canvas view"))
			.shortcut("Z")
			.checkable()
			.addTo(m_drawingtools)
		)
		.action(makeAction(QT_TR_NOOP("Inspector"), "toolinspector")
			.icon("help-whatsthis")
			.statusTip(QT_TR_NOOP("Find out who did it"))
			.shortcut("Ctrl+I")
			.checkable()
			.addTo(m_drawingtools)
		)
		.submenu([=](MenuBuilder menu) {
			menu
			.title(QT_TR_NOOP("&Shortcuts"))
			.action(makeAction(QT_TR_NOOP("Toggle eraser mode"), "currenterasemode")
				.shortcut("Ctrl+E")
				.onTriggered(m_dockToolSettings, &docks::ToolSettings::toggleEraserMode)
			)
			.action(makeAction(QT_TR_NOOP("Swap Last Colors"), "swapcolors")
				.shortcut("X")
				.onTriggered(m_dockToolSettings, &docks::ToolSettings::swapLastUsedColors)
			)
			.action(makeAction(QT_TR_NOOP("&Decrease Brush Size"), "ensmallenbrush")
				.shortcut(Qt::Key_BracketLeft)
				.autoRepeat(true)
				.onTriggered([=] {
					m_dockToolSettings->quickAdjustCurrent1(-1);
				})
			)
			.action(makeAction(QT_TR_NOOP("&Increase Brush Size"), "embiggenbrush")
				.shortcut(Qt::Key_BracketRight)
				.autoRepeat(true)
				.onTriggered([=] {
					m_dockToolSettings->quickAdjustCurrent1(1);
				})
			);
		});

	for(auto *dt : m_drawingtools->actions()) {
		// Add a separator before color picker to separate brushes from
		// non-destructive tools
		if(dt->objectName() == "pickertool")
			drawtools->addSeparator();
		drawtools->addAction(dt);
	}

	// Window menu (Mac only)
#ifdef Q_OS_MACOS
	menuBar()->addMenu(MacMenu::instance()->windowMenu());
#endif

	// Help menu
	makeMenu(QT_TR_NOOP("&Help"), menuBar())
		.action(makeAction(QT_TR_NOOP("&Homepage"), "dphomepage")
			.statusTip(WEBSITE)
			.onTriggered(&MainWindow::homepage)
		)
		.action(makeAction(QT_TR_NOOP("Tablet Tester"), "tablettester")
			.onTriggered([] {
				dialogs::TabletTestDialog *ttd=nullptr;
				// Check if dialog is already open
				for(QWidget *toplevel : qApp->topLevelWidgets()) {
					ttd = qobject_cast<dialogs::TabletTestDialog*>(toplevel);
					if(ttd)
						break;
				}
				if(!ttd) {
					ttd = new dialogs::TabletTestDialog;
					ttd->setAttribute(Qt::WA_DeleteOnClose);
				}
				ttd->show();
				ttd->raise();
			})
		)
		.action(makeAction(QT_TR_NOOP("Log File"), "showlogfile")
			.onTriggered([] {
				QDesktopServices::openUrl(QUrl::fromLocalFile(utils::logFilePath()));
			})
		)
#ifndef Q_OS_MACOS
		// Qt shunts the About menus into the Application menu on macOS, so this
		// would cause two separators to be placed side by side
		.separator()
#endif
		.action(makeAction(QT_TR_NOOP("&About Drawpile"), "dpabout")
			.menuRole(QAction::AboutRole)
			.onTriggered(&MainWindow::about)
		)
		.action(makeAction(QT_TR_NOOP("About &Qt"), "aboutqt")
			.menuRole(QAction::AboutQtRole)
			.onTriggered(&QApplication::aboutQt)
		)
		.separator()
		.action(makeAction(QT_TR_NOOP("Check For Updates"), "versioncheck")
			.onTriggered([=] {
				auto *dlg = new dialogs::VersionCheckDialog(this);
				dlg->show();
				dlg->queryNewVersions();
			})
		);

	// Brush slot shortcuts
	m_brushSlots = new QActionGroup(this);
	for(int i=0;i<6;++i) {
		ActionBuilder(this, tr)
			.text([i=i+1] {
				return tr("Brush slot #%1").arg(i);
			})
			.autoRepeat(false)
			.objectName(QStringLiteral("quicktoolslot-%1").arg(i))
			.shortcut(QString::number(i + 1))
			.property("toolslotidx", i)
			.addTo(m_brushSlots)
			.addTo(this);
	}
	connect(m_brushSlots, &QActionGroup::triggered, [=](QAction *a) {
		m_dockToolSettings->setToolSlot(a->property("toolslotidx").toInt());
		m_toolChangeTime.start();
	});

	// Add temporary tool change shortcut detector
	for(QAction *act : m_drawingtools->actions())
		act->installEventFilter(m_tempToolSwitchShortcut);

	for(QAction *act : m_brushSlots->actions())
		act->installEventFilter(m_tempToolSwitchShortcut);

	// Other shortcuts
	makeAction(QT_TR_NOOP("Finish action"), "finishstroke")
		.shortcut(Qt::Key_Return)
		.onTriggered(m_doc->toolCtrl(), &tools::ToolController::finishMultipartDrawing);

	makeAction(QT_TR_NOOP("Cancel action"), "cancelaction")
		.shortcut(Qt::Key_Escape)
		.onTriggered(m_doc->toolCtrl(), &tools::ToolController::cancelMultipartDrawing);
}

void MainWindow::createDocks()
{
	Q_ASSERT(m_doc);
	Q_ASSERT(m_view);
	Q_ASSERT(m_canvasscene);

	// Create tool settings
	m_dockToolSettings = new docks::ToolSettings(m_doc->toolCtrl(), this);
	m_dockToolSettings->setObjectName("ToolSettings");
	m_dockToolSettings->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_dockToolSettings);
	static_cast<tools::SelectionSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::SELECTION))->setView(m_view);
	static_cast<tools::AnnotationSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::ANNOTATION))->setScene(m_canvasscene);
	m_view->setPressureMapping(static_cast<tools::BrushSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::FREEHAND))->getPressureMapping());

	// Create brush palette
	m_dockBrushPalette = new docks::BrushPalette(this);
	m_dockBrushPalette->setObjectName("BrushPalette");
	m_dockBrushPalette->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_dockBrushPalette);

	tools::BrushSettings *brushSettings = static_cast<tools::BrushSettings*>(m_dockToolSettings->getToolSettingsPage(tools::Tool::FREEHAND));
	m_dockBrushPalette->connectBrushSettings(brushSettings);

	// Create color docks
	m_dockColorSpinner = new docks::ColorSpinnerDock(this);
	m_dockColorSpinner->setObjectName("colorspinnerdock");
	addDockWidget(Qt::RightDockWidgetArea, m_dockColorSpinner);

	m_dockColorPalette = new docks::ColorPaletteDock(this);
	m_dockColorPalette->setObjectName("colorpalettedock");
	addDockWidget(Qt::RightDockWidgetArea, m_dockColorPalette);

	m_dockColorSliders = new docks::ColorSliderDock(tr("Color Sliders"), this);
	m_dockColorSliders->setObjectName("colorsliderdock");
	addDockWidget(Qt::RightDockWidgetArea, m_dockColorSliders);

	tabifyDockWidget(m_dockColorPalette, m_dockColorSliders);
	tabifyDockWidget(m_dockColorSliders, m_dockColorSpinner);

	// Create layer list
	m_dockLayers = new docks::LayerList(this);
	m_dockLayers->setObjectName("LayerList");
	m_dockLayers->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_dockLayers);

	// Create navigator
	m_dockNavigator = new docks::Navigator(this);
	m_dockNavigator->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, m_dockNavigator);
	m_dockNavigator->hide(); // hidden by default

	// Create timeline
	m_dockTimeline = new docks::Timeline(this);
	m_dockTimeline->setObjectName("Timeline");
	m_dockTimeline->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::TopDockWidgetArea, m_dockTimeline);
}
