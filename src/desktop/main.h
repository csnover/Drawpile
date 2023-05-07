// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAWPILEAPP_H
#define DRAWPILEAPP_H

#include <QApplication>
#include <QMap>
#include "desktop/notifications.h"

class MainWindow;
class QSoundEffect;

class DrawpileApp final : public QApplication {
Q_OBJECT
   friend void notification::playSoundNow(notification::Event, int);
public:
	// A config marker to indicate what themes the user last saw. 0 or missing
	// was Drawpile 2.1 with only System, Fusion and Fusion Dark.
	static constexpr int THEME_VERSION = 1;

	enum Theme {
		THEME_SYSTEM,
		THEME_FUSION_LIGHT,
		THEME_FUSION_DARK,
		THEME_KRITA_BRIGHT,
		THEME_KRITA_DARK,
		THEME_KRITA_DARKER,
		THEME_SYSTEM_LIGHT,
		THEME_SYSTEM_DARK,
		THEME_HOTDOG_STAND,
		THEME_COUNT,
#ifdef Q_OS_MACOS
		THEME_DEFAULT = THEME_SYSTEM,
#else
		THEME_DEFAULT = THEME_KRITA_DARK,
#endif
	};

	DrawpileApp(int & argc, char ** argv );
	~DrawpileApp() override;

	void setTheme(int theme);
	void initTheme();

	void notifySettingsChanged();

	void openUrl(QUrl url);

	void openBlankDocument();

	void deleteAllMainWindowsExcept(MainWindow *win);

signals:
	void settingsChanged();
	void eraserNear(bool near);
	void setDockTitleBarsHidden(bool hidden);

protected:
	bool event(QEvent *e) override;

private:
	QMap<notification::Event, QSoundEffect*> m_sounds;
	void updateThemeIcons();

	QPalette loadPalette(const QString &file);
};

#endif
