// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef ANIMATIONSAVERRUNNABLE_H
#define ANIMATIONSAVERRUNNABLE_H

#include <QObject>
#include <QRunnable>

namespace canvas { class PaintEngine; }
namespace rustpile { enum class AnimationExportMode; }

/**
 * @brief A runnable for saving the canvas content as an animation in a background thread
 */
class AnimationSaverRunnable final : public QObject, public QRunnable
{
	Q_OBJECT
public:
	AnimationSaverRunnable(const canvas::PaintEngine *pe, rustpile::AnimationExportMode mode, const QString &filename, QObject *parent = nullptr);

	void run() override;

public slots:
	void cancelExport();

signals:
	void progress(int progress);
	void saveComplete(const QString &error, const QString &errorDetail);

private:
	friend bool animationSaverProgressCallback(void *ctx, float progress);
	const canvas::PaintEngine *m_pe;
	QString m_filename;
	rustpile::AnimationExportMode m_mode;

	bool m_cancelled;
};

#endif
