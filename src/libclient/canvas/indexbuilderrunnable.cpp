// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/canvas/indexbuilderrunnable.h"
#include "libclient/canvas/paintengine.h"
#include "rustpile/rustpile.h"

namespace canvas {

IndexBuilderRunnable::IndexBuilderRunnable(const PaintEngine *pe)
	: QObject(), m_paintengine(pe)
{

}

static void emit_progress(void *runnable, uint32_t progress)
{
	emit static_cast<IndexBuilderRunnable*>(runnable)->progress(progress);
}

void IndexBuilderRunnable::run()
{
	const bool result = rustpile::paintengine_build_index(
		m_paintengine->engine(),
		this,
		&emit_progress
	);

	emit indexingComplete(result);
}

}
