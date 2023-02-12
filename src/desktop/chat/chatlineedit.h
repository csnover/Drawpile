// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef CHATLINEEDIT_H
#define CHATLINEEDIT_H

#include <QStringList>
#include <QLineEdit>

/**
 * @brief A specialized line edit widget for chatting, with history
  */
class ChatLineEdit : public QLineEdit
{
Q_OBJECT
public:
	explicit ChatLineEdit(QWidget *parent = 0);

	//! Push text to history
	void pushHistory(const QString& text);

	//! Get the current text with trailing whitespace removed
	QString trimmedText() const;

signals:
	void returnPressed(const QString &text);

public slots:

protected:
	void keyPressEvent(QKeyEvent *event);

private:
	QStringList _history;
	QString _current;
	int _historypos;
};

#endif
