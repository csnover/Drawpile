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
use dpcore::paint::{rasterop, Blendmode, BrushMask, Pixel15};

fn mask_blend(mask: &[u16], mode: Blendmode) {
    let mut base = [[128, 128, 128, 128]; 64 * 64];
    rasterop::mask_blend(&mut base, [255, 255, 255, 255], mask, mode, 127);
}

fn pixel_blend(over: &[Pixel15], mode: Blendmode) {
    let mut base = [[128, 128, 128, 128]; 64 * 64];
    rasterop::pixel_blend(&mut base, over, 128, mode);
}

fn main() {
    let mask = BrushMask::new_round_pixel(64);
    let over = vec![[255, 255, 255, 255]; 64 * 64];

    benches!(
        inline:

        Bench::new("mask normal").run(|| mask_blend(&mask.mask, Blendmode::Normal)),
        Bench::new("mask erase").run(|| mask_blend(&mask.mask, Blendmode::Erase)),
        Bench::new("mask multiply").run(|| mask_blend(&mask.mask, Blendmode::Multiply)),
        Bench::new("mask behind").run(|| mask_blend(&mask.mask, Blendmode::Behind)),
        Bench::new("mask colorerase").run(|| mask_blend(&mask.mask, Blendmode::ColorErase)),

        Bench::spacer(),

        Bench::new("pixel normal").run(|| pixel_blend(&over, Blendmode::Normal)),
        Bench::new("pixel erase").run(|| pixel_blend(&over, Blendmode::Erase)),
        Bench::new("pixel multiply").run(|| pixel_blend(&over, Blendmode::Multiply)),
        Bench::new("pixel behind").run(|| pixel_blend(&over, Blendmode::Behind)),
        Bench::new("pixel colorerase").run(|| pixel_blend(&over, Blendmode::ColorErase)),
    );
}
