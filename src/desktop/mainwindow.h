// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QMenu>
#include <QUrl>
#include <QPointer>
#include <QVariant>

#include "desktop/utils/dynamicui.h"
#include "libclient/tools/tool.h"
#include "libclient/canvas/acl.h"

class QActionGroup;
class QLabel;
class QSplitter;
class QToolButton;

class Document;
class ActionBuilder;
class MenuBuilder;

namespace widgets {
	class CanvasView;
	class NetStatus;
	class NotificationBar;
	class ChatBox;
	class ViewStatus;
}
namespace docks {
	class ToolSettings;
	class BrushPalette;
	class InputSettings;
	class LayerList;
	class PaletteBox;
	class ColorPaletteDock;
	class ColorSpinnerDock;
	class ColorSliderDock;
	class Navigator;
	class Timeline;
}
namespace dialogs {
	class PlaybackDialog;
	class HostDialog;
	class SessionSettingsDialog;
	class ServerLogDialog;
}
namespace drawingboard {
	class CanvasScene;
}
namespace canvas {
	class CanvasModel;
}
namespace rustpile {
	enum class CanvasIoError;
	enum class AnimationExportMode;
}

#ifdef Q_OS_MACOS
class MacMenu;
#endif

class ShortcutDetector;

//! The application main window
class MainWindow final : public QMainWindow {
	Q_OBJECT
public:
	static ActionBuilder makeAction(const char *text, const char *name, QObject *parent);
	static MenuBuilder makeMenu(const char *title, QWidget *parent);

	static void showErrorMessage(QWidget *parent, const QString& message, const QString& details=QString());
	static void showJoinDialog(MainWindow *parent, const QUrl &defaultUrl=QUrl());

	MainWindow(bool restoreWindowPosition=true);
	~MainWindow() override;

	//! Host a session using the settings from the given dialog
	void hostSession(dialogs::HostDialog *dlg);

	//! Connect to a host and join a session if full URL is provided.
	void joinSession(const QUrl& url, const QString &autoRecordFilename=QString());

	//! Check if the current board can be replaced
	bool canReplace() const;

	//! Save settings and exit
	void exit();

public slots:
	// Triggerable actions
	void showNew();
	void open();
	void open(const QUrl &url);
	void save();
	void saveas();
	void saveSelection();
	void showFlipbook();

	static void showSettings();
	void reportAbuse();
	void tryToGainOp();
	void resetSession();
	void terminateSession();

	void host();
	void join(const QUrl &defaultUrl=QUrl());
	void leave();

	void toggleFullscreen();
	void setShowAnnotations(bool show);
	void setShowLaserTrails(bool show);

	void selectTool(QAction *tool);

	static void about();
	static void homepage();

	//! Create a blank new document
	void newDocument(const QSize &size, const QColor &background);

private slots:
	void toggleRecording();

	void exportGifAnimation();
	void exportAnimationFrames();

	void onOperatorModeChange(bool op);
	void onFeatureAccessChange(canvas::Feature feature, bool canUse);

	void onServerConnected();
	void onServerLogin();
	void onServerDisconnected(const QString &message, const QString &errorcode, bool localDisconnect);
	void onNsfmChanged(bool nsfm);

	void updateLockWidget();
	void setRecorderStatus(bool on);

	void loadShortcuts();
	void updateSettings();

	void updateLayerViewMode();

	void copyText();
	void paste();
	void pasteFile();
	void pasteFile(const QUrl &url);
	void pasteImage(const QImage &image, const QPoint *point=nullptr);
	void dropUrl(const QUrl &url);

	void clearOrDelete();

	void resizeCanvas();
	void changeCanvasBackground();

	void toolChanged(tools::Tool::Type tool);

	void selectionRemoved();

	void hotBorderMenubar(bool show);

	void setFreezeDocks(bool freeze);
	void setDocksHidden(bool hidden);

	void updateTitle();

	void onCanvasChanged(canvas::CanvasModel *canvas);
	void onCanvasSaveStarted();
	void onCanvasSaved(const QString &errorMessage);

	void recoverFromError();

protected:
	void closeEvent(QCloseEvent *event) override;
	bool event(QEvent *event) override;

private:
	enum class NotificationError {
		None,
		Disconnected,
		PaintEngineCrashed
	};

	MainWindow *replaceableWindow();

	//! Confirm saving of image in a format that doesn't support all required features
	bool confirmFlatten(QString& file) const;

	void exportAnimation(const QString &path, rustpile::AnimationExportMode mode);

	ActionBuilder makeAction(const char *text, const char *name);
	QToolBar *makeToolBar(const char *title, QWidget *parent = nullptr);
	QAction *getAction(const QString &name);

	//! Add a new entry to recent files list
	void addRecentFile(const QString& file);

	//! Enable or disable drawing tools
	void setDrawingToolsEnabled(bool enable);

	//! Display an error message
	void showErrorMessage(const QString& message, const QString& details=QString());
	void showErrorMessage(rustpile::CanvasIoError error);

	void readSettings(bool windowpos=true);
	void writeSettings();

	void createDocks();
	void setupActions();

	void updateSessionSettings();
	void showSessionSettings();

	void toggleChat(bool show);

	QSplitter *m_splitter;

	docks::ToolSettings *m_dockToolSettings;
	docks::BrushPalette *m_dockBrushPalette;
	docks::InputSettings *m_dockInput;
	docks::LayerList *m_dockLayers;
	docks::ColorPaletteDock *m_dockColorPalette;
	docks::ColorSpinnerDock *m_dockColorSpinner;
	docks::ColorSliderDock *m_dockColorSliders;
	docks::Navigator *m_dockNavigator;
	docks::Timeline *m_dockTimeline;
	widgets::ChatBox *m_chatbox;

	widgets::CanvasView *m_view;
	widgets::NotificationBar *m_notificationBar;

	QStatusBar *m_viewStatusBar;
	QLabel *m_lockstatus;
	widgets::NetStatus *m_netstatus;
	widgets::ViewStatus *m_viewstatus;
	QToolButton *m_statusChatButton;

	QPointer<dialogs::PlaybackDialog> m_playbackDialog;
	dialogs::SessionSettingsDialog *m_sessionSettings;
	dialogs::ServerLogDialog *m_serverLogDialog;

	QMenu *m_recentMenu;

	QActionGroup *m_currentdoctools; // general tools that require no special permissions
	QActionGroup *m_admintools;      // session operator actions
	QActionGroup *m_modtools;        // session moderator tools
	QActionGroup *m_canvasbgtools;   // tools related to canvas background feature
	QActionGroup *m_resizetools;     // tools related to canvas resizing feature
	QActionGroup *m_putimagetools;   // Cut&Paste related tools
	QActionGroup *m_undotools;       // undo&redo related tools
	QActionGroup *m_drawingtools;    // drawing tool selection
	QActionGroup *m_brushSlots;      // tool slot shortcuts

	int m_lastToolBeforePaste; // Last selected tool before Paste was used

	QMetaObject::Connection m_textCopyConnection;

	// Remember window state to return from fullscreen mode
	QRect m_fullscreenOldGeometry;
	bool m_fullscreenOldMaximized;

	QElapsedTimer m_toolChangeTime; // how long the user has held down the tool change button
	ShortcutDetector *m_tempToolSwitchShortcut;

	bool m_exitAfterSave;

	Document * const m_doc;
	drawingboard::CanvasScene * const m_canvasscene;

	Translator<QString, QString> m_windowTitle;
	Translator<bool> m_recordAction;

	NotificationError m_error = NotificationError::None;
};

#endif
