// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include <QObject>

/**
 * An event filter that emits hotBorder(bool) when mouse touches or leaves the top edge of the screen
 *
 * Absolute pointer coordinates are used. This can be used to show and hide the menu
 * bar in fullscreen mode. (Note: not needed on macOS, as the OS provides this feature itself.)
 */
class HotBorderEventFilter : public QObject {
	Q_OBJECT
public:
	HotBorderEventFilter(QObject *parent);

signals:
	void hotBorder(bool active);

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

private:
	bool m_hotBorder;
};
