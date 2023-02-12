// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef USERNAMEVALIDATOR_H
#define USERNAMEVALIDATOR_H

#include <QValidator>

class UsernameValidator : public QValidator
{
	Q_OBJECT
public:
	explicit UsernameValidator(QObject *parent = 0);

	State validate(QString &input, int &pos) const;

	static bool isValid(const QString &username);
};

#endif // USERNAMEVALIDATOR_H
