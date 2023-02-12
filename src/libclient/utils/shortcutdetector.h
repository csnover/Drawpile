// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SHORTCUTDETECTOR_H
#define SHORTCUTDETECTOR_H

#include <QObject>

/**
 * This is an event filter class used to detect when a shortcut event
 * is sent to the target object.
 *
 */
class ShortcutDetector : public QObject
{
	Q_OBJECT
public:
	explicit ShortcutDetector(QObject *parent = 0);

	bool isShortcutSent() const { return _shortcutSent; }
	void reset() { _shortcutSent = false; }

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private:
	bool _shortcutSent;
};

#endif // SHORTCUTDETECTOR_H
