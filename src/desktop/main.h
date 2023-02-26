// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DRAWPILEAPP_H
#define DRAWPILEAPP_H

#include <QApplication>
#include <QList>
#include <QString>

class QTranslator;

class DrawpileApp : public QApplication {
Q_OBJECT
public:
	DrawpileApp(int & argc, char ** argv );

	void initTranslations();

	void setDarkTheme(bool dark);
	void setLanguage(QString preferredLang = QString());
	void notifySettingsChanged();

	void openUrl(QUrl url);

	void openBlankDocument();

signals:
	void settingsChanged();
	void eraserNear(bool near);

protected:
	bool event(QEvent *e);

	QList<QTranslator *> m_translators;
	QString m_currentLang;
};

#endif
