// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use std::env;

use dpcore::paint::floodfill::{expand_floodfill, floodfill};
use dpcore::paint::{editlayer, Blendmode, Color, LayerID, LayerInsertion, LayerStack, Rectangle};

mod utils;

fn main() {
    let mut expansion = 0;
    let args: Vec<String> = env::args().collect();
    if args.len() > 1 {
        expansion = args[1].parse::<i32>().unwrap();
    }

    let mut layerstack = LayerStack::new(768 * 2, 832);

    // Set up the canvas
    layerstack
        .root_mut()
        .add_bitmap_layer(1, Color::TRANSPARENT, LayerInsertion::Top);

    layerstack
        .root_mut()
        .add_bitmap_layer(2, Color::TRANSPARENT, LayerInsertion::Top);

    let (bgimage, w, h) = utils::load_image_data(include_bytes!("testdata/filltest.png"));

    editlayer::draw_image8(
        layerstack.root_mut().get_bitmaplayer_mut(1).unwrap(),
        0,
        &bgimage,
        &Rectangle::new(0, 0, w, h),
        1.0,
        Blendmode::Replace,
    );
    editlayer::draw_image8(
        layerstack.root_mut().get_bitmaplayer_mut(1).unwrap(),
        0,
        &bgimage,
        &Rectangle::new(768, 0, w, h),
        1.0,
        Blendmode::Replace,
    );

    // Test on-layer floodfill
    do_floodfill(
        &mut layerstack,
        86,
        86,
        Color {
            r: 1.0,
            g: 0.0,
            b: 0.0,
            a: 1.0,
        },
        1,
        false,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        86,
        150,
        Color {
            r: 0.0,
            g: 1.0,
            b: 0.0,
            a: 1.0,
        },
        1,
        false,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        86,
        286,
        Color {
            r: 0.0,
            g: 0.0,
            b: 1.0,
            a: 1.0,
        },
        1,
        false,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        86,
        544,
        Color {
            r: 1.0,
            g: 1.0,
            b: 0.0,
            a: 1.0,
        },
        1,
        false,
        expansion,
    );

    do_floodfill(
        &mut layerstack,
        480,
        86,
        Color {
            r: 1.0,
            g: 0.0,
            b: 0.0,
            a: 1.0,
        },
        1,
        false,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        480,
        150,
        Color {
            r: 0.0,
            g: 1.0,
            b: 0.0,
            a: 1.0,
        },
        1,
        false,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        480,
        286,
        Color {
            r: 0.0,
            g: 0.0,
            b: 1.0,
            a: 1.0,
        },
        1,
        false,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        480,
        544,
        Color {
            r: 1.0,
            g: 1.0,
            b: 0.0,
            a: 1.0,
        },
        1,
        false,
        expansion,
    );

    // Test merged floodfill
    do_floodfill(
        &mut layerstack,
        854,
        86,
        Color {
            r: 1.0,
            g: 0.0,
            b: 0.0,
            a: 1.0,
        },
        2,
        true,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        854,
        150,
        Color {
            r: 0.0,
            g: 1.0,
            b: 0.0,
            a: 1.0,
        },
        2,
        true,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        854,
        286,
        Color {
            r: 0.0,
            g: 0.0,
            b: 1.0,
            a: 1.0,
        },
        2,
        true,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        854,
        544,
        Color {
            r: 1.0,
            g: 1.0,
            b: 0.0,
            a: 1.0,
        },
        2,
        true,
        expansion,
    );

    do_floodfill(
        &mut layerstack,
        1248,
        86,
        Color {
            r: 1.0,
            g: 0.0,
            b: 0.0,
            a: 1.0,
        },
        2,
        true,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        1248,
        150,
        Color {
            r: 0.0,
            g: 1.0,
            b: 0.0,
            a: 1.0,
        },
        2,
        true,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        1248,
        286,
        Color {
            r: 0.0,
            g: 0.0,
            b: 1.0,
            a: 1.0,
        },
        2,
        true,
        expansion,
    );
    do_floodfill(
        &mut layerstack,
        1248,
        544,
        Color {
            r: 1.0,
            g: 1.0,
            b: 0.0,
            a: 1.0,
        },
        2,
        true,
        expansion,
    );

    utils::save_layerstack(&layerstack, "example_floodfill.png");
}

fn do_floodfill(
    image: &mut LayerStack,
    x: i32,
    y: i32,
    color: Color,
    layer: LayerID,
    sample_merged: bool,
    expansion: i32,
) {
    let result = floodfill(image, x, y, color, 0.2, layer, sample_merged, 1000000);
    assert!(!result.oversize);

    let result = expand_floodfill(result, expansion);

    editlayer::draw_image8(
        image.root_mut().get_bitmaplayer_mut(1).unwrap(),
        0,
        &result.image.pixels,
        &Rectangle::new(
            result.x,
            result.y,
            result.image.width as i32,
            result.image.height as i32,
        ),
        0.5,
        Blendmode::Normal,
    );
}
