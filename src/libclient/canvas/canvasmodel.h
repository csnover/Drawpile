// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CANVASMODEL_H
#define CANVASMODEL_H

#include <QObject>
#include <QPointer>

namespace net {
	class Envelope;
}

namespace rustpile {
	enum class CanvasIoError;
}

namespace canvas {

class AclState;
class UserListModel;
class LayerListModel;
class TimelineModel;
class Selection;
class DocumentMetadata;
class PaintEngine;

class CanvasModel final : public QObject
{
	Q_OBJECT
public:
	explicit CanvasModel(uint8_t localUserId, QObject *parent=nullptr);

	PaintEngine *paintEngine() const { return m_paintengine; }

	//! Load canvas content from file
	rustpile::CanvasIoError load(const QString &path);

	//! Load an empty canvas
	bool load(const QSize &size, const QColor &background);

	//! Load a recording and prepare to start playback
	rustpile::CanvasIoError loadRecording(const QString &path);

	QString title() const { return m_title; }
	void setTitle(const QString &title) { if(m_title!=title) { m_title = title; emit titleChanged(title); } }

	QString pinnedMessage() const { return m_pinnedMessage; }

	Selection *selection() const { return m_selection; }
	void setSelection(Selection *selection);

	net::Envelope generateSnapshot() const;

	uint8_t localUserId() const;

	QImage selectionToImage(int layerId) const;
	void pasteFromImage(const QImage &image, const QPoint &defaultPoint, bool forceDefault);

	void connectedToServer(uint8_t myUserId, bool join);
	void disconnectedFromServer();

	AclState *aclState() const { return m_aclstate; }
	UserListModel *userlist() const { return m_userlist; }
	LayerListModel *layerlist() const { return m_layerlist; }
	TimelineModel *timeline() const { return m_timeline; }
	DocumentMetadata *metadata() const { return m_metadata; }

	//! Open a recording file and start recording
	rustpile::CanvasIoError startRecording(const QString &path);

	//! Stop recording
	void stopRecording();

	//! Is recording in progress?
	bool isRecording() const;

	//! Size of the canvas
	QSize size() const;

	/**
	 * Request the view layer to preview a change to an annotation
	 *
	 * This is used to preview the change or creation of an annotation.
	 * If an annotation with the given ID does not exist yet, one will be created.
	 * The annotation only exists in the view layer and will thus be automatically erased
	 * or replaced when the actual change goes through.
	 */
	void previewAnnotation(int id, const QRect &shape);

	/**
	 * Reset the canvas to a blank state, as if the client had just joined a session.
	 *
	 * This is used to prepare the canvas to receive session reset data.
	 */
	void resetCanvas();

public slots:
	//! Handle a meta/command message received from the server
	void handleCommand(const net::Envelope &cmd);

	//! Handle a local drawing command (will be put in the local fork)
	void handleLocalCommand(const net::Envelope &cmd);

	void pickLayer(int x, int y);
	void pickColor(int x, int y, int layer, int diameter=0);
	void inspectCanvas(int x, int y);
	void inspectCanvas(int contextId);
	void stopInspectingCanvas();

	void updateLayerViewOptions();

signals:
	void layerAutoselectRequest(int id);
	void canvasModified();
	void selectionChanged(Selection *selection);
	void selectionRemoved();

	void previewAnnotationRequested(int id, const QRect &shape);

	void titleChanged(QString title);
	void pinnedMessageChanged(QString message);
	void imageSizeChanged();

	void colorPicked(const QColor &color);
	void canvasInspected(int lastEditedBy);
	void canvasInspectionEnded();

	void chatMessageReceived(int sender, int recipient, uint8_t tflags, uint8_t oflags, const QString &message);

	void laserTrail(uint8_t userId, int persistence, const QColor &color);

	void userJoined(int id, const QString &name);
	void userLeft(int id, const QString &name);

	void recorderStateChanged(bool recording);

private slots:
	void onCanvasResize(int xoffset, int yoffset, const QSize &oldsize);

private:
	friend void metaUserJoin(void *canvas, uint8_t user, uint8_t flags, const uint8_t *name, uintptr_t name_len, const uint8_t *avatar, uintptr_t avatar_len);
	friend void metaUserLeave(void *ctx, uint8_t user);
	friend void metaChatMessage(void *ctx, uint8_t sender, uint8_t recipient, uint8_t tflags, uint8_t oflags, const uint8_t *message, uintptr_t message_len);
	friend void metaLaserTrail(void *ctx, uint8_t user, uint8_t persistence, uint32_t color);
	friend void metaDefaultLayer(void *ctx, uint16_t layerId);
	friend void metaAclChange(void *ctx, uint32_t changes);
	friend void metaRecorderStateChanged(void *ctx, bool recording);

	AclState *m_aclstate;
	UserListModel *m_userlist;
	LayerListModel *m_layerlist;
	TimelineModel *m_timeline;
	DocumentMetadata *m_metadata;

	PaintEngine *m_paintengine;
	Selection *m_selection;

	QString m_title;
	QString m_pinnedMessage;
};

}

#endif // CANVASSTATE_H
