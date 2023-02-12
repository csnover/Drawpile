// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use dpcore::paint::{editlayer, BitmapLayer, Blendmode, BrushMask, Color, Tile, BIT15_F32};
use std::f32::consts::PI;

mod utils;

fn brush_stroke(layer: &mut BitmapLayer, y: i32) {
    let black = Color {
        r: 0.0,
        g: 0.0,
        b: 0.0,
        a: 1.0,
    };

    for x in (10..246).step_by(5) {
        let w = 16 + ((x as f32 / 40.0 * PI).sin() * 15.0) as i32;
        let brush = BrushMask::new_round_pixel(w as u32);
        editlayer::draw_brush_dab(
            layer,
            0,
            x - w / 2,
            y - w / 2,
            &brush,
            &black,
            Blendmode::Normal,
            (0.4 * BIT15_F32) as u16,
        );
    }
}

fn main() {
    let mut layer = BitmapLayer::new(0, 256, 256, Tile::new(&Color::rgb8(255, 255, 255), 0));

    // The brush stroke drawn in direct mode
    brush_stroke(&mut layer, 60);

    // Indirect mode using a sublayer
    let sublayer = layer.get_or_create_sublayer(1);
    sublayer.metadata_mut().opacity = 0.5;
    brush_stroke(sublayer, 120);
    editlayer::merge_sublayer(&mut layer, 1);

    utils::save_layer(&layer, "example_layer_indirect.png");
}
