// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use dpcore::paint::tile::Tile;
use dpcore::paint::{
    editlayer, BitmapLayer, Blendmode, BrushMask, Color, LayerInsertion, LayerStack, BIT15_F32,
};
use std::f32::consts::PI;

mod utils;

fn brush_stroke(layer: &mut BitmapLayer, y: i32, color: &Color) {
    for x in (10..246).step_by(5) {
        let w = 16 + ((x as f32 / 40.0 * PI).sin() * 15.0) as i32;
        let brush = BrushMask::new_round_pixel(w as u32);
        editlayer::draw_brush_dab(
            layer,
            0,
            x - w / 2,
            y - w / 2,
            &brush,
            color,
            Blendmode::Normal,
            (0.4 * BIT15_F32) as u16,
        );
    }
}

fn main() {
    let mut layerstack = LayerStack::new(256, 256);
    layerstack.background = Tile::new_solid(&Color::rgb8(255, 255, 255), 0);

    layerstack
        .root_mut()
        .add_bitmap_layer(1, Color::TRANSPARENT, LayerInsertion::Top);
    layerstack
        .root_mut()
        .add_bitmap_layer(2, Color::TRANSPARENT, LayerInsertion::Top);

    layerstack
        .root_mut()
        .add_group_layer(3, LayerInsertion::Top);
    layerstack
        .root_mut()
        .add_bitmap_layer(4, Color::TRANSPARENT, LayerInsertion::Into(3));

    // Second stroke layer opacity to 50%
    layerstack
        .root_mut()
        .get_bitmaplayer_mut(2)
        .unwrap()
        .metadata_mut()
        .opacity = 0.5;

    // Third stroke layer and parent group opacity to 50% (so effective opacity is 25%)
    layerstack
        .root_mut()
        .get_layer_mut(3)
        .unwrap()
        .metadata_mut()
        .opacity = 0.5;
    layerstack
        .root_mut()
        .get_layer_mut(4)
        .unwrap()
        .metadata_mut()
        .opacity = 0.5;

    brush_stroke(
        layerstack.root_mut().get_bitmaplayer_mut(1).unwrap(),
        60,
        &Color::rgb8(255, 0, 0),
    );
    brush_stroke(
        layerstack.root_mut().get_bitmaplayer_mut(2).unwrap(),
        80,
        &Color::rgb8(255, 0, 0),
    );
    brush_stroke(
        layerstack.root_mut().get_bitmaplayer_mut(4).unwrap(),
        100,
        &Color::rgb8(255, 0, 0),
    );

    utils::save_layerstack(&layerstack, "example_layerstack.png");
}
