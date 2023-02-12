// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef SESSIONIDVALIDATOR_H
#define SESSIONIDVALIDATOR_H

#include <QValidator>

class SessionIdAliasValidator : public QValidator
{
	Q_OBJECT
public:
	explicit SessionIdAliasValidator(QObject *parent=nullptr);

	State validate(QString &input, int &pos) const;
};

#endif // SESSIONIDVALIDATOR_H
