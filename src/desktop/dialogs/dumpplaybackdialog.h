/*
 * Drawpile - a collaborative drawing program.
 *
 * Copyright (C) 2023 askmeaboutloom
 *
 * Drawpile is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drawpile is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DUMPPLAYBACKDIALOG_H
#define DUMPPLAYBACKDIALOG_H

#include <QDialog>

namespace canvas {
class CanvasModel;
}

namespace drawdance {
class CanvasHistorySnapshot;
}

namespace dialogs {

class DumpPlaybackDialog final : public QDialog {
	Q_OBJECT
public:
	explicit DumpPlaybackDialog(
		canvas::CanvasModel *canvas, QWidget *parent = nullptr);

	~DumpPlaybackDialog() override;

protected:
	void closeEvent(QCloseEvent *) override;

private slots:
	void onDumpPlaybackAt(
		long long pos, const drawdance::CanvasHistorySnapshot &chs);
	void playPause();
	void singleStep();
	void jumpToPreviousReset();
	void jumpToNextReset();
	void jump();

private:
	void updateUi();
	void updateTables();
	void updateHistoryTable();
	void updateForkTable();
	void updateStatus(
		int historyCount, int historyOffset, int forkCount, int forkStart,
		int forkFallbehind);

	struct Private;
	Private *d;
};

}

#endif
