// This file is part of Drawpile.
// Copyright (C) 2020 Calle Laakkonen
//
// Drawpile is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// As additional permission under section 7, you are allowed to distribute
// the software through an app store, even if that store has restrictive
// terms and conditions that are incompatible with the GPL, provided that
// the source is also available under the GPL with or without this permission
// through a channel without those restrictive terms and conditions.
//
// Drawpile is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Drawpile.  If not, see <https://www.gnu.org/licenses/>.

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
