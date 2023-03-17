// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef USERNAMEVALIDATOR_H
#define USERNAMEVALIDATOR_H

#include <QValidator>

class UsernameValidator final : public QValidator
{
	Q_OBJECT
public:
	explicit UsernameValidator(QObject *parent = nullptr);

	State validate(QString &input, int &pos) const override;

	static bool isValid(const QString &username);
};

#endif // USERNAMEVALIDATOR_H
