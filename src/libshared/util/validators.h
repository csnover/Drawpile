// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DP_VALIDATORS_H
#define DP_VALIDATORS_H

class QString;

bool validateSessionIdAlias(const QString &alias);
bool validateUsername(const QString &username);

#endif
