// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef ACL_FEATURES_H
#define ACL_FEATURES_H

namespace canvas {

// Access levels
enum class Tier : unsigned char {
	Op,      // operators
	Trusted, // + users marked as trusted
	Auth,    // + registered users
	Guest    // everyone
};

static const int TierCount = 4;

}

#endif
