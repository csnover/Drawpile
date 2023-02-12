// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

namespace notification {

enum class Event {
	CHAT,     // Chat message received
	MARKER,   // Recording playback stopped on a marker
	LOCKED,   // Active layer was locked
	UNLOCKED, // ...unlocked
	LOGIN,    // A user just logged in
	LOGOUT    // ...logged out
};

void playSound(Event event);

void setVolume(int volume);

}

#endif // NOTIFICATIONS_H
