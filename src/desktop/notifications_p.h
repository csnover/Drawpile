// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Drawpile contributors

#ifndef DESKTOP_NOTIFICATIONS_P_H
#define DESKTOP_NOTIFICATIONS_P_H

#include <QObject>
#include <QSoundEffect>
#include <QThread>
#include <QtGlobal>

namespace notification {

static QThread soundThread;

class Sound : public QObject {
	Q_OBJECT
public:
	Sound(const QUrl &source)
		: QObject()
	{
		m_sound.setSource(source);
		connect(this, &Sound::playSound, &m_sound, &QSoundEffect::play);
		m_sound.moveToThread(&soundThread);

		if (!soundThread.isRunning()) {
			soundThread.start();
		}
	}

	void play(int volume)
	{
		if (!m_sound.isPlaying()) {
			m_sound.setVolume(volume / 100.0f);
			emit playSound();
		}
	}

signals:
	void playSound();

private:
	QSoundEffect m_sound;
};

} // namespace notification

#endif
