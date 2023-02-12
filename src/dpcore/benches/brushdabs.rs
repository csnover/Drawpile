// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use brunch::{benches, Bench};
use dpcore::paint::{BrushMask, ClassicBrushCache};

fn main() {
    const HARDNESS: f32 = 0.5;

    let mut cache = ClassicBrushCache::new();
    // Pre-fill LUT to avoid benchmarking the cache
    BrushMask::new_gimp_style_v2(0.0, 0.0, 1.0, HARDNESS, &mut cache);

    benches!(inline:
        Bench::new("tiny v2 GIMP style dab")
            .run(|| BrushMask::new_gimp_style_v2(0.0, 0.0, 1.0, HARDNESS, &mut cache)),
        Bench::new("small v2 GIMP style dab")
            .run(|| BrushMask::new_gimp_style_v2(0.0, 0.0, 15.0, HARDNESS, &mut cache)),
        Bench::new("big v2 GIMP style dab")
            .run(|| BrushMask::new_gimp_style_v2(0.0, 0.0, 30.0, HARDNESS, &mut cache)),
        Bench::spacer(),
        Bench::new("round pixel dab")
            .run(|| BrushMask::new_round_pixel(15)),
    );
}
