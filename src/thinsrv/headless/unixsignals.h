// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef UNIXSIGNALS_H
#define UNIXSIGNALS_H

#include <QObject>

/**
 * @brief A class for converting UNIX signals to Qt signals
 *
 * To use, simply connect to the signal you are interested.
 */
class UnixSignals : public QObject
{
	Q_OBJECT
public:
	static UnixSignals *instance();

signals:
	void sigInt();
	void sigTerm();
	void sigUsr1();

protected:
	void connectNotify(const QMetaMethod &signal);

private slots:
	void handleSignal();

private:
	UnixSignals();
};

#endif // UNIXSIGNALS_H
