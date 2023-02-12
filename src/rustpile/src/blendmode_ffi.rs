// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use dpcore::paint::Blendmode;
use std::slice;

/// Find a blending mode matching the given SVG name
///
/// If no match is found, the default (normal) mode is returned.
#[no_mangle]
pub extern "C" fn blendmode_from_svgname(name: *const u16, name_len: usize) -> Blendmode {
    let name = String::from_utf16_lossy(unsafe { slice::from_raw_parts(name, name_len) });

    Blendmode::from_svg_name(&name).unwrap_or_default()
}

#[no_mangle]
pub extern "C" fn blendmode_svgname(mode: Blendmode, name_len: &mut usize) -> *const u8 {
    let name = mode.svg_name();
    *name_len = name.len();
    name.as_ptr()
}
