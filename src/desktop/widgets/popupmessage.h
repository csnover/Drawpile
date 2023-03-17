// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef POPUPMESSAGE_H
#define POPUPMESSAGE_H

#include <QWidget>
#include <QPainterPath>

class QTextDocument;
class QTimer;

namespace widgets {

/**
 * @brief Popup messagebox
 * A simple box that can be popped up to display a message.
 */
class PopupMessage final : public QWidget
{
	Q_OBJECT
public:
	PopupMessage(int maxLines, const QWidget *parent);

	/**
	 * @brief Pop up the message box and show a message
	 *
	 * If the popup is already visible, the message will be appended
	 * to the existing one.
	 *
	 * @param point origin point for the popup (the little arrow will point here)
	 * @param message the message to show.
	 */
	void showMessage(const QPoint& point, const QString &message);

protected:
	void paintEvent(QPaintEvent *) override;

private:
	void setMessage(const QString &message);
	void redrawBubble();

	int m_maxLines;
	qreal m_arrowoffset;
	QPainterPath m_bubble;
	QTimer *m_timer;
	QTextDocument *m_doc;
	const QWidget *m_parentWidget;
};

}

#endif
