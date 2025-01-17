// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAWPILE_DOCUMENT_H
#define DRAWPILE_DOCUMENT_H

extern "C" {
#include <dpmsg/acl.h>
#include <dpmsg/blend_mode.h>
#include <dpengine/load.h>
}

#include "libclient/drawdance/message.h"
#include "libclient/drawdance/paintengine.h"
#include "libclient/net/announcementlist.h"
#include "libclient/net/banlistmodel.h"
#include "libshared/util/qtcompat.h"

#include <QObject>
#include <QStringListModel>

class QString;
class QTimer;

namespace canvas {
	class CanvasModel;
}
namespace net {
	class Client;
	class BanlistModel;
	class AnnouncementListModel;
}

namespace tools { class ToolController; }

/**
 * @brief An active document and its associated data, including the network connection
 *
 * This is an UI agnostic class that should be usable from both a widget
 * based application or a pure QML app.
 *
 */
class Document final : public QObject
{
	Q_PROPERTY(canvas::CanvasModel* canvas READ canvas() NOTIFY canvasChanged)
	Q_PROPERTY(net::BanlistModel* banlist READ banlist() CONSTANT)
	Q_PROPERTY(net::AnnouncementListModel* announcementList READ announcementList() CONSTANT)
	Q_PROPERTY(QStringListModel* serverLog READ serverLog() CONSTANT)
	Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyCanvas)
	Q_PROPERTY(bool autosave READ isAutosave WRITE setAutosave NOTIFY autosaveChanged)
	Q_PROPERTY(bool canAutosave READ canAutosave NOTIFY canAutosaveChanged)
	Q_PROPERTY(QString sessionTitle READ sessionTitle NOTIFY sessionTitleChanged)
	Q_PROPERTY(QString currentFilename READ currentFilename() NOTIFY currentFilenameChanged)
	Q_PROPERTY(bool recording READ isRecording() NOTIFY recorderStateChanged)

	Q_PROPERTY(bool sessionPersistent READ isSessionPersistent NOTIFY sessionPersistentChanged)
	Q_PROPERTY(bool sessionClosed READ isSessionClosed NOTIFY sessionClosedChanged)
	Q_PROPERTY(bool sessionAuthOnly READ isSessionAuthOnly NOTIFY sessionAuthOnlyChanged)
	Q_PROPERTY(bool sessionPreserveChat READ isSessionPreserveChat NOTIFY sessionPreserveChatChanged)
	Q_PROPERTY(bool sessionPasswordProtected READ isSessionPasswordProtected NOTIFY sessionPasswordChanged)
	Q_PROPERTY(bool sessionHasOpword READ isSessionOpword NOTIFY sessionOpwordChanged)
	Q_PROPERTY(bool sessionNsfm READ isSessionNsfm NOTIFY sessionNsfmChanged)
	Q_PROPERTY(bool sessionForceNsfm READ isSessionForceNsfm NOTIFY sessionForceNsfmChanged)
	Q_PROPERTY(bool sessionDeputies READ isSessionDeputies NOTIFY sessionDeputiesChanged)
	Q_PROPERTY(int sessionMaxUserCount READ sessionMaxUserCount NOTIFY sessionMaxUserCountChanged)
	Q_PROPERTY(double sessionResetThreshold READ sessionResetThreshold NOTIFY sessionResetThresholdChanged)
	Q_PROPERTY(double baseResetThreshold READ baseResetThreshold NOTIFY baseResetThresholdChanged)
	Q_PROPERTY(QString roomcode READ roomcode NOTIFY sessionRoomcodeChanged)

	Q_OBJECT
public:
	explicit Document(QObject *parent=nullptr);

	QString title() const;

	canvas::CanvasModel *canvas() const { return m_canvas; }
	tools::ToolController *toolCtrl() const { return m_toolctrl; }
	net::Client *client() const { return m_client; }
	net::BanlistModel *banlist() const { return m_banlist; }
	net::AnnouncementListModel *announcementList() const { return m_announcementlist; }
	QStringListModel *serverLog() const { return m_serverLog; }

	/**
	 * @brief (Re)initialize the canvas
	 *
	 * This deletes the old canvas (if it exists) and creates a fresh one.
	 */
	void initCanvas();

	bool loadBlank(const QSize &size, const QColor &background);
	DP_LoadResult loadFile(const QString &path);
	DP_LoadResult loadRecording(
		const QString &path, bool debugDump, bool *outIsTemplate = nullptr);

	/**
	 * @brief Save the canvas content
	 *
	 * Saving is done in a background thread. The signal `canvasSaved`
	 * is emitted when saving completes.
	 *
	 * @param filename the file to save to
	 * @param errorMessage if not null, error message is stored here
	 */
	void saveCanvasAs(const QString &filename);
	void saveCanvasStateAs(
		const QString &filename, const drawdance::CanvasState &canvasState,
		bool isCurrentState);
	void exportTemplate(const QString &path);
	bool saveSelection(const QString &path);
	bool isSaveInProgress() const { return m_saveInProgress; }

	void setAutosave(bool autosave);
	bool isAutosave() const { return m_autosave; }
	bool canAutosave() const { return m_canAutosave; }

	void setWantCanvasHistoryDump(bool wantCanvasHistoryDump);
	bool wantCanvasHistoryDump() const { return m_wantCanvasHistoryDump; }

	QString sessionTitle() const;

	QString currentFilename() const { return m_currentFilename; }

	bool isRecording() const;
	drawdance::RecordStartResult startRecording(const QString &filename);
	bool stopRecording();

	bool isDirty() const { return m_dirty; }

	bool isSessionPersistent() const { return m_sessionPersistent; }
	bool isSessionClosed() const { return m_sessionClosed; }
	bool isSessionAuthOnly() const { return m_sessionAuthOnly; }
	bool isSessionPreserveChat() const { return m_sessionPreserveChat; }
	bool isSessionPasswordProtected() const { return m_sessionPasswordProtected; }
	bool isSessionOpword() const { return m_sessionOpword; }
	bool isSessionNsfm() const { return m_sessionNsfm; }
	bool isSessionForceNsfm() const { return m_sessionForceNsfm; }
	bool isSessionDeputies() const { return m_sessionDeputies; }
	int sessionMaxUserCount() const { return m_sessionMaxUserCount; }
	double sessionResetThreshold() const { return m_sessionResetThreshold/double(1024*1024); }
	double baseResetThreshold() const { return m_baseResetThreshold/double(1024*1024); }

	QString roomcode() const { return m_roomcode; }

	void setRecordOnConnect(const QString &filename) { m_recordOnConnect = filename; }

	qulonglong pasteId() const { return reinterpret_cast<uintptr_t>(this); }

	bool isCompatibilityMode() const;

signals:
	//! Connection opened, but not yet logged in
	void serverConnected(const QString &address, int port);
	void serverLoggedIn(bool join);
	void serverDisconnected(const QString &message, const QString &errorcode, bool localDisconnect);
	void compatibilityModeChanged(bool compatibilityMode);

	void canvasChanged(canvas::CanvasModel *canvas);
	void dirtyCanvas(bool isDirty);
	void autosaveChanged(bool autosave);
	void canAutosaveChanged(bool canAutosave);
	void currentFilenameChanged(const QString &filename);
	void recorderStateChanged(bool recording);

	void sessionTitleChanged(const QString &title);
	void sessionPreserveChatChanged(bool pc);
	void sessionPersistentChanged(bool p);
	void sessionClosedChanged(bool closed);
	void sessionAuthOnlyChanged(bool closed);
	void sessionPasswordChanged(bool passwordProtected);
	void sessionOpwordChanged(bool opword);
	void sessionNsfmChanged(bool nsfm);
	void sessionForceNsfmChanged(bool forceNsfm);
	void sessionDeputiesChanged(bool deputies);
	void sessionMaxUserCountChanged(int count);
	void sessionRoomcodeChanged(const QString &code);
	void sessionResetThresholdChanged(double threshold);
	void baseResetThresholdChanged(double threshold);
	void autoResetTooLarge(int maxSize);
	void sessionResetState(const drawdance::CanvasState &canvasState);

	void catchupProgress(int percent);

	void canvasSaveStarted();
	void canvasSaved(const QString &errorMessage);
	void templateExported(const QString &errorMessage);

	void justInTimeSnapshotGenerated();

public slots:
	// Convenience slots
	void sendPointerMove(const QPointF &point);
	void sendSessionConf(const QJsonObject &sessionconf);
	void sendFeatureAccessLevelChange(const uint8_t[DP_FEATURE_COUNT]);
	void sendLockSession(bool lock=true);
	void sendOpword(const QString &opword);
	void sendResetSession(const drawdance::MessageList &resetImage = {});
	void sendResizeCanvas(int top, int right, int bottom, int left);
	void sendUnban(int entryId);
	void sendAnnounce(const QString &url, bool privateMode);
	void sendUnannounce(const QString &url);
	void sendTerminateSession();
	void sendCanvasBackground(const QColor &color);
	void sendAbuseReport(int userId, const QString &message);

	// Tool related functions
	void undo();
	void redo();

	void selectAll(); // Note: selection tool should be activated before calling this
	void selectNone();
	void cancelSelection();

	void copyVisible();
	void copyMerged();
	void copyLayer();
	void cutLayer();
	void pasteImage(const QImage &image, const QPoint &point, bool forcePoint); // Note: selection tool should be activated before calling this
	void stamp();

	void removeEmptyAnnotations();
	void clearArea();
	void fillArea(const QColor &color, DP_BlendMode mode, bool source = false);

	void addServerLogEntry(const QString &log);

	void updateSettings();

private slots:
	void onServerLogin(bool join, bool compatibilityMode);
	void onServerDisconnect();
	void onSessionResetted();

	void onSessionConfChanged(const QJsonObject &config);
	void onAutoresetRequested(int maxSize, bool query);
	void onMoveLayerRequested(int sourceId, int targetId, bool intoGroup, bool below);

	void snapshotNeeded();
	void markDirty();
	void unmarkDirty();

	void autosaveNow();
	void onCanvasSaved(const QString &errorMessage);

private:
	void saveCanvasState(const drawdance::CanvasState &canvasState, bool isCurrentState);
	void setCurrentFilename(const QString &filename);
	void setSessionPersistent(bool p);
	void setSessionClosed(bool closed);
	void setSessionAuthOnly(bool authOnly);
	void setSessionPreserveChat(bool pc);
	void setSessionPasswordProtected(bool pp);
	void setSessionOpword(bool ow);
	void setSessionMaxUserCount(int count);
	void setSessionResetThreshold(int threshold);
	void setBaseResetThreshold(int threshold);
	void setSessionNsfm(bool nsfm);
	void setSessionForceNsfm(bool forceNsfm);
	void setSessionDeputies(bool deputies);
	void setRoomcode(const QString &roomcode);

	void copyFromLayer(int layer);

	void autosave();

	void generateJustInTimeSnapshot();
	void sendResetSnapshot();

	QString m_currentFilename;

	drawdance::MessageList m_resetstate;
	drawdance::MessageList m_messageBuffer;

	canvas::CanvasModel *m_canvas;
	tools::ToolController *m_toolctrl;
	net::Client *m_client;
	net::BanlistModel *m_banlist;
	net::AnnouncementListModel *m_announcementlist;
	QStringListModel *m_serverLog;

	QString m_originalRecordingFilename;
	QString m_recordOnConnect;

	bool m_dirty;
	bool m_autosave;
	bool m_canAutosave;
	bool m_saveInProgress;
	bool m_wantCanvasHistoryDump;
	QTimer *m_autosaveTimer;

	QString m_roomcode;

	bool m_sessionPersistent;
	bool m_sessionClosed;
	bool m_sessionAuthOnly;
	bool m_sessionPreserveChat;
	bool m_sessionPasswordProtected;
	bool m_sessionOpword;
	bool m_sessionNsfm;
	bool m_sessionForceNsfm;
	bool m_sessionDeputies;

	int m_sessionMaxUserCount;
	int m_sessionHistoryMaxSize;
	int m_sessionResetThreshold;
	int m_baseResetThreshold;
};

#endif // DOCUMENT_H

