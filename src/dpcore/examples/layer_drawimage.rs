// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use dpcore::paint::{editlayer, BitmapLayer, Blendmode, Color, Rectangle, Tile};

mod utils;

fn main() {
    let mut layer = BitmapLayer::new(0, 256, 256, Tile::new(&Color::rgb8(255, 255, 255), 0));

    let (image, w, h) = utils::load_image_data(include_bytes!("testdata/logo.png"));

    editlayer::draw_image8(
        &mut layer,
        0,
        &image,
        &Rectangle::new(256 - w * 2 / 3, 256 - h * 2 / 3, w, h),
        1.0,
        Blendmode::Replace,
    );

    editlayer::draw_image8(
        &mut layer,
        0,
        &image,
        &Rectangle::new(256 / 2 - w / 2, 256 / 2 - h / 2, w, h),
        0.5,
        Blendmode::Normal,
    );

    editlayer::draw_image8(
        &mut layer,
        0,
        &image,
        &Rectangle::new(-w / 2, -h / 2, w, h),
        1.0,
        Blendmode::Normal,
    );

    utils::save_layer(&layer, "example_layer_drawimage.png");
}
