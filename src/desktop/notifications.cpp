// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/notifications.h"
#include "desktop/notifications_p.h"
#include "libshared/util/paths.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QSettings>

namespace notification {

static QMap<Event, Sound*> sounds;
static qint64 lasttime = 0;

void playSound(Event event)
{
	// Notification rate limiting
	const qint64 t = QDateTime::currentMSecsSinceEpoch();
	if(t - lasttime < 1500)
		return;
	lasttime = t;

	// Check if this notification is enabled
	QSettings cfg;
	cfg.beginGroup("notifications");
	int volume = qBound(0, cfg.value("volume", 40).toInt(), 100);
	if(volume==0)
		return;

	bool enabled = false;
	switch(event) {
	case Event::CHAT: enabled = cfg.value("chat", true).toBool(); break;
	case Event::MARKER: enabled = cfg.value("marker", true).toBool(); break;
	case Event::LOCKED:
	case Event::UNLOCKED:
		enabled = cfg.value("lock", true).toBool();
		break;
	case Event::LOGIN:
	case Event::LOGOUT:
		enabled = cfg.value("login", true).toBool();
		break;
	}
	if(!enabled)
		return;

	// Lazily load the sound effect
	if(!sounds.contains(event)) {
		QString filename;
		switch(event) {
		case Event::CHAT: filename = QStringLiteral("sounds/chat.wav"); break;
		case Event::MARKER: filename = QStringLiteral("sounds/marker.wav"); break;
		case Event::LOCKED: filename = QStringLiteral("sounds/lock.wav"); break;
		case Event::UNLOCKED: filename = QStringLiteral("sounds/unlock.wav"); break;
		case Event::LOGIN: filename = QStringLiteral("sounds/login.wav"); break;
		case Event::LOGOUT: filename = QStringLiteral("sounds/logout.wav"); break;
		}

		Q_ASSERT(!filename.isEmpty());
		if(filename.isEmpty()) {
			qWarning("Sound effect %d not defined!", int(event));
			return;
		}

		const QString fullpath = utils::paths::locateDataFile(filename);
		if(fullpath.isEmpty()) {
			qWarning() << filename << "not found!";
			return;
		}

		sounds[event] = new Sound(QUrl::fromLocalFile(fullpath));
	}

	// We have a sound effect... play it now
	sounds[event]->play(volume);
}

}
