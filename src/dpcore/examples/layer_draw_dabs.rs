// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use dpcore::paint::{
    editlayer, BitmapLayer, Blendmode, BrushMask, ClassicBrushCache, Color, Tile, BIT15_U16,
};

mod utils;

fn main() {
    let mut layer = BitmapLayer::new(0, 256, 256, Tile::new(&Color::rgb8(255, 255, 255), 0));
    let black = Color::rgb8(0, 0, 0);

    let mut x = 10;
    let mut w = 1i32;

    let mut cache = ClassicBrushCache::new();

    while x < layer.width() as i32 - w {
        let brush = BrushMask::new_square_pixel(w as u32);
        editlayer::draw_brush_dab(
            &mut layer,
            0,
            x - w / 2,
            30 - w / 2,
            &brush,
            &black,
            Blendmode::Normal,
            BIT15_U16,
        );

        let brush = BrushMask::new_round_pixel(w as u32);
        editlayer::draw_brush_dab(
            &mut layer,
            0,
            x - w / 2,
            60 - w / 2,
            &brush,
            &black,
            Blendmode::Normal,
            BIT15_U16,
        );

        let (bx, by, brush) =
            BrushMask::new_gimp_style_v2(x as f32, 90.0, w as f32, 0.0, &mut cache);
        editlayer::draw_brush_dab(
            &mut layer,
            0,
            bx,
            by,
            &brush,
            &black,
            Blendmode::Normal,
            BIT15_U16,
        );

        let (bx, by, brush) =
            BrushMask::new_gimp_style_v2(x as f32, 120.0, w as f32, 1.0, &mut cache);
        editlayer::draw_brush_dab(
            &mut layer,
            0,
            bx,
            by,
            &brush,
            &black,
            Blendmode::Normal,
            BIT15_U16,
        );

        w += 1;
        x += w + 2;
    }

    utils::save_layer(&layer, "example_layer_draw_dabs.png");
}
