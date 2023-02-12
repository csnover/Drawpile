// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/export/animationsaverrunnable.h"
#include "libclient/canvas/paintengine.h"
#include "rustpile/rustpile.h"

AnimationSaverRunnable::AnimationSaverRunnable(const canvas::PaintEngine *pe, rustpile::AnimationExportMode mode, const QString &filename, QObject *parent)
	: QObject(parent),
	  m_pe(pe),
	  m_filename(filename),
	  m_mode(mode),
	  m_cancelled(false)
{
}

bool animationSaverProgressCallback(void *ctx, float progress) {
	auto *a = static_cast<AnimationSaverRunnable*>(ctx);
	if(a->m_cancelled)
		return false;

	emit a->progress(qRound(progress * 100));
	return true;
}

void AnimationSaverRunnable::run()
{
	const auto result = rustpile::paintengine_save_animation(
		m_pe->engine(),
		reinterpret_cast<const uint16_t*>(m_filename.constData()),
		m_filename.length(),
		m_mode,
		this,
		&animationSaverProgressCallback
	);

	QString msg;
	switch(result) {
	case rustpile::CanvasIoError::NoError: break;
	case rustpile::CanvasIoError::FileOpenError: msg = tr("Couldn't open file for writing"); break;
	default: msg = tr("An error occurred while saving image");
	}

	emit saveComplete(msg, QString());
}

void AnimationSaverRunnable::cancelExport()
{
	m_cancelled = true;
}
