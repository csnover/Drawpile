// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

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
