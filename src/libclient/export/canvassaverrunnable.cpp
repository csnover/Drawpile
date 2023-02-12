// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/export/canvassaverrunnable.h"
#include "libclient/canvas/paintengine.h"
#include "rustpile/rustpile.h"

CanvasSaverRunnable::CanvasSaverRunnable(const canvas::PaintEngine *pe, const QString &filename, QObject *parent)
	: QObject(parent),
	  m_pe(pe),
	  m_filename(filename)
{
}

void CanvasSaverRunnable::run()
{
	const auto result = rustpile::paintengine_save_file(
		m_pe->engine(),
		reinterpret_cast<const uint16_t*>(m_filename.constData()),
		m_filename.length()
	);

	switch(result) {
	case rustpile::CanvasIoError::NoError: emit saveComplete(QString()); break;
	case rustpile::CanvasIoError::FileOpenError: emit saveComplete(tr("Couldn't open file for writing")); break;
	default: emit saveComplete(tr("An error occurred while saving image"));
	}
}
