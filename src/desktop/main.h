// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DRAWPILEAPP_H
#define DRAWPILEAPP_H

#include <QApplication>

class DrawpileApp : public QApplication {
Q_OBJECT
public:
	DrawpileApp(int & argc, char ** argv );

	void setDarkTheme(bool dark);
	void notifySettingsChanged();

	void openUrl(QUrl url);

	void openBlankDocument();

signals:
	void settingsChanged();
	void eraserNear(bool near);

protected:
	bool event(QEvent *e);
};

#endif
