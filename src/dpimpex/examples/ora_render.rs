// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use std::env;

// A sample program that can read an OpenRaster file (or any other supported file)
// and save it again (as a PNG, for example.)
// This can be used to compare Drawpile's interpretation of the OpenRaster file
// with the original.
fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 3 {
        println!("Usage: ora_render <source.ora> <target.png>");
    } else {
        let source = dpimpex::load_image(&args[1]).unwrap();
        dpimpex::save_image(&args[2], &source).unwrap();
    }
}
