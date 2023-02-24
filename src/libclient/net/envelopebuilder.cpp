// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/net/envelopebuilder.h"
#include "libclient/net/envelope.h"
#include "rustpile/rustpile.h"

#include <QImage>

namespace net {

EnvelopeBuilder::EnvelopeBuilder()
	: m_writer(rustpile::messagewriter_new())
{
}

EnvelopeBuilder::EnvelopeBuilder(EnvelopeBuilder&& other) :
	m_writer(other.m_writer)
{
	other.m_writer = nullptr;
}

EnvelopeBuilder &EnvelopeBuilder::operator=(EnvelopeBuilder&& other)
{
	if(this == &other)
		return *this;
	if(m_writer)
		rustpile::messagewriter_free(m_writer);
	m_writer = other.m_writer;
	other.m_writer = nullptr;
	return *this;
}

EnvelopeBuilder::~EnvelopeBuilder()
{
	if(m_writer)
		rustpile::messagewriter_free(m_writer);
}

Envelope EnvelopeBuilder::toEnvelope()
{
	uintptr_t len;
	const uchar *data = rustpile::messagewriter_content(m_writer, &len);

	if(len == 0)
		return Envelope();

	return Envelope(data, len);
}

void EnvelopeBuilder::buildPutQImage(uint8_t ctxid, uint16_t layer, int x, int y, const QImage &image, rustpile::Blendmode mode)
{
	// Crop image if target coordinates are negative, since the protocol
	// does not support negative coordites.
	QImage img = image;
	if(x<0 || y<0) {
		if(x < -image.width() || y < -image.height()) {
			// the entire image is outside the canvas
			return;
		}

		int xoffset = x<0 ? -x : 0;
		int yoffset = y<0 ? -y : 0;
		img = image.copy(xoffset, yoffset, image.width()-xoffset, image.height()-yoffset);
		x += xoffset;
		y += yoffset;
	}

	img = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

	rustpile::write_putimage(
		m_writer,
		ctxid,
		layer,
		x,
		y,
		img.width(),
		img.height(),
		mode,
		img.constBits()
	);
}

void EnvelopeBuilder::buildUndo(uint8_t ctxid, uint8_t overrideId, bool redo)
{
	rustpile::write_undo(m_writer, ctxid, overrideId, redo);
}

void EnvelopeBuilder::buildUndoPoint(uint8_t ctxid)
{
	rustpile::write_undopoint(m_writer, ctxid);
}

}
