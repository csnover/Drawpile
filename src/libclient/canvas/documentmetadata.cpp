// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/canvas/documentmetadata.h"
#include "rustpile/rustpile.h"
#include "libclient/canvas/paintengine.h"

namespace canvas {

DocumentMetadata::DocumentMetadata(PaintEngine *engine, QObject *parent)
	: QObject{parent}, m_engine(engine), m_framerate(15), m_useTimeline(false)
{
	Q_ASSERT(engine);
	refreshMetadata();
	connect(engine, &PaintEngine::metadataChanged, this, &DocumentMetadata::refreshMetadata);
}

void DocumentMetadata::refreshMetadata()
{
	const auto e = m_engine->engine();

	// Note: dpix and dpiy are presently not used in the GUI.
	// To be included here when needed.

	const int framerate = rustpile::paintengine_get_metadata_int(e, rustpile::MetadataInt::Framerate);
	if(framerate != m_framerate) {
		m_framerate = framerate;
		emit framerateChanged(framerate);
	}

	const bool useTimeline = rustpile::paintengine_get_metadata_int(e, rustpile::MetadataInt::UseTimeline);
	if(useTimeline != m_useTimeline) {
		m_useTimeline = useTimeline;
		emit useTimelineChanged(useTimeline);
	}
}

}
