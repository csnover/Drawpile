// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DP_NET_ENVELOPEBUILDER_H
#define DP_NET_ENVELOPEBUILDER_H

#include <cstdint>

#include "libclient/net/envelope.h"

class QImage;

namespace rustpile {
	struct MessageWriter;
	enum class Blendmode : uint8_t;
}

namespace net {

/**
 * @brief A helper class for using Rustpile MessageWriter to create message envelopes
 */
class EnvelopeBuilder {
public:
	EnvelopeBuilder();
	~EnvelopeBuilder();

	EnvelopeBuilder(const EnvelopeBuilder&) = delete;
	EnvelopeBuilder &operator=(const EnvelopeBuilder&) = delete;

	Envelope toEnvelope();

	operator rustpile::MessageWriter*() const { return m_writer; }

	/// Helper function: write a PutImage command using a QImage
	void buildPutQImage(uint8_t ctxid, uint16_t layer, int x, int y, const QImage &image, rustpile::Blendmode mode);

	/// Helper function: write a Undo/Redo message
	void buildUndo(uint8_t ctxid, uint8_t overrideId, bool redo);

	/// Helper function: write an undopoint
	void buildUndoPoint(uint8_t ctxid);

private:
	rustpile::MessageWriter *m_writer;
};
}

#endif
