// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use dpcore::paint::{
    editlayer, BitmapLayer, Blendmode, BrushMask, Color, Rectangle, Tile, BIT15_U16,
};

mod utils;

fn main() {
    let mut layer = BitmapLayer::new(0, 256, 512, Tile::Blank);

    // Draw a nice background
    let colors = [
        0xff845ec2, 0xffd65db1, 0xffff6f91, 0xffff9671, 0xffffc75f, 0xfff9f871,
    ];

    for (i, &c) in colors.iter().enumerate() {
        editlayer::fill_rect(
            &mut layer,
            0,
            &Color::from_argb32(c),
            Blendmode::Normal,
            &Rectangle::new(i as i32 * 40, 0, 40, 512),
        );
    }

    // Draw dabs using all the blend modes
    let modes = [
        Blendmode::Erase,
        Blendmode::Normal,
        Blendmode::Multiply,
        Blendmode::Divide,
        Blendmode::Burn,
        Blendmode::Dodge,
        Blendmode::Darken,
        Blendmode::Lighten,
        Blendmode::Subtract,
        Blendmode::Add,
        Blendmode::Recolor,
        Blendmode::Behind,
        Blendmode::ColorErase,
        Blendmode::Screen,
        Blendmode::NormalAndEraser,
        Blendmode::LuminosityShineSai,
        Blendmode::Overlay,
        Blendmode::HardLight,
        Blendmode::SoftLight,
        Blendmode::LinearBurn,
        Blendmode::LinearLight,
        Blendmode::Hue,
        Blendmode::Saturation,
        Blendmode::Luminosity,
        Blendmode::Color,
        Blendmode::Replace,
    ];

    let brush = BrushMask::new_round_pixel(10);
    let dabcolor = Color::rgb8(255, 0, 0);

    for (i, &mode) in modes.iter().enumerate() {
        println!("Mode: {mode:?}");
        for x in (10..250).step_by(8) {
            editlayer::draw_brush_dab(
                &mut layer,
                0,
                x,
                10 + i as i32 * 15,
                &brush,
                &dabcolor,
                mode,
                BIT15_U16,
            );
        }
    }
    utils::save_layer(&layer, "example_layer_blend_dabs.png");
}
