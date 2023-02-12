// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef PASSWORDHASH_H
#define PASSWORDHASH_H

#include <QtGlobal>
#include "libshared/util/qtcompat.h"

class QByteArray;
class QString;

namespace server {
namespace passwordhash {

enum Algorithm {
	PLAINTEXT,
	SALTED_SHA1, // deprecated
	PBKDF2,
	SODIUM, // the best algorithm offered by libsodium

#if defined(HAVE_LIBSODIUM)
	BEST_ALGORITHM = SODIUM
#elif defined(HAVE_QT_COMPAT_PBKDF2)
	BEST_ALGORITHM = PBKDF2
#else
   BEST_ALGORITHM = SALTED_SHA1
#endif
};

/**
 * @brief Check the given password against the hash
 * @param password
 * @param hash
 * @return true if password matches
 */
bool check(const QString &password, const QByteArray &hash);

/**
 * @brief Generate a password hash
 *
 * If an empty string is given, the returned hash is also empty.
 *
 * @param password the password to hash
 * @param algorithm the algorithm to use
 * @return password hash that can be given to the check function
 */
QByteArray hash(const QString &password, Algorithm algorithm=BEST_ALGORITHM);

/**
 * @brief Check if the given password hash is valid and uses a supported algorithm.
 */
bool isValidHash(const QByteArray &hash);

}
}

#endif // PASSWORDHASH_H
