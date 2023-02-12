// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include <QKeyEvent>

#include "desktop/chat/chatlineedit.h"

ChatLineEdit::ChatLineEdit(QWidget *parent) :
	QLineEdit(parent), _historypos(0)
{
}

void ChatLineEdit::pushHistory(const QString &text)
{
	// Disallow consecutive duplicates
	if(!_history.isEmpty() && _history.last() == text)
		return;

	// Add to history list while limiting its size
	_history.append(text);
	if(_history.count() > 50)
		_history.removeFirst();

	++_historypos;
}

void ChatLineEdit::keyPressEvent(QKeyEvent *event)
{
	if(event->key() == Qt::Key_Up) {
		if(_historypos>0) {
			if(_historypos==_history.count())
				_current = text();
			--_historypos;
			setText(_history[_historypos]);
		}
	} else if(event->key() == Qt::Key_Down) {
		if(_historypos<_history.count()-1) {
			++_historypos;
			setText(_history[_historypos]);
		} else if(_historypos==_history.count()-1) {
			++_historypos;
			setText(_current);
		}
	} else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
		QString txt = trimmedText();
		if(!txt.isEmpty()) {
			pushHistory(txt);
			_historypos = _history.count();
			setText(QString());
			emit returnPressed(txt);
		}
	} else {
		QLineEdit::keyPressEvent(event);
	}
}

QString ChatLineEdit::trimmedText() const
{
	QString str = text();

	// Remove trailing whitespace
	int chop = str.length();
	while(chop>0 && str.at(chop-1).isSpace())
		--chop;

	if(chop==0)
		return QString();
	str.truncate(chop);

	return str;
}
