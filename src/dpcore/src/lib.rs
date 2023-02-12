// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

// lint: We often want to group hex color constants like "AA_RRGGBB"
#![allow(clippy::unusual_byte_groupings)]
#![allow(clippy::too_many_arguments)]

pub mod brush;
pub mod canvas;
pub mod paint;
pub mod protocol;
