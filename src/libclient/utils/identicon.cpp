// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/utils/identicon.h"

#include <QPainter>

QImage make_identicon(const QString &name, const QSize &size)
{
	const auto hash = qHash(name);
	const QColor color = QColor::fromHsl(hash % 360, 165, 142);

	QImage image(size, QImage::Format_ARGB32_Premultiplied);
	image.fill(color);

	QPainter painter(&image);
	QFont font;
	font.setPixelSize(size.height() * 0.7);
	painter.setFont(font);
	painter.setPen(Qt::white);
	painter.drawText(QRect(QPoint(), size), Qt::AlignCenter, name.left(1));

	return image;
}
