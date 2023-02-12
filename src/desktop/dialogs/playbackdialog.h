// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef PLAYBACKDIALOG_H
#define PLAYBACKDIALOG_H

#include <QDialog>
#include <QPointer>
#include <QElapsedTimer>

namespace canvas {
	class CanvasModel;
	class PaintEngine;
}

namespace rustpile {
	struct RecordingIndex;
}

class Ui_PlaybackDialog;
class QMenu;
class VideoExporter;

namespace dialogs {

class PlaybackDialog : public QDialog
{
	Q_OBJECT
public:
	explicit PlaybackDialog(canvas::CanvasModel *canvas, QWidget *parent=nullptr);
	~PlaybackDialog();

	void centerOnParent();

	bool isPlaying() const;
	void setPlaying(bool playing);

signals:
	void playbackToggled(bool play);

protected:
	void closeEvent(QCloseEvent *);
	void keyPressEvent(QKeyEvent *);

private slots:
	void onPlaybackAt(qint64 pos, qint32 interval);
	void stepNext();
	void autoStepNext(qint32 interval);
	void jumpTo(int pos);

	void loadIndex();

	void onBuildIndexClicked();
	void onVideoExportClicked();

	void exportFrame(int count=1);

private:
	Ui_PlaybackDialog *m_ui;
	canvas::PaintEngine *m_paintengine;
	rustpile::RecordingIndex *m_index;
	QPointer<VideoExporter> m_exporter;

	QTimer *m_autoStepTimer;
	QElapsedTimer m_lastInterval;
	float m_speedFactor;

	qint32 m_intervalAfterExport;
	bool m_autoplay;
	bool m_awaiting;
};

}

#endif // PLAYBACKDIALOG_H
