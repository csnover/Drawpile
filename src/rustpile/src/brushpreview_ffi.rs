// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::brushpreview::{BrushPreview, BrushPreviewShape};
use dpcore::brush::{ClassicBrush, MyPaintBrush, MyPaintSettings};
use dpcore::paint::color::{pixels15_to_8, ZERO_PIXEL8};
use dpcore::paint::tile::{TILE_LENGTH, TILE_SIZEI};
use dpcore::paint::{AoE, Color, FlattenedTileIterator, LayerViewOptions};

use core::ffi::c_void;

#[no_mangle]
pub unsafe extern "C" fn brushpreview_new(width: u32, height: u32) -> *mut BrushPreview {
    Box::into_raw(Box::new(BrushPreview::new(width, height)))
}

#[no_mangle]
pub unsafe extern "C" fn brushpreview_free(bp: *mut BrushPreview) {
    if !bp.is_null() {
        drop(Box::from_raw(bp))
    }
}

#[no_mangle]
pub extern "C" fn brushpreview_render_classic(
    bp: &mut BrushPreview,
    brush: &ClassicBrush,
    shape: BrushPreviewShape,
) {
    bp.render_classic(brush, shape);
}

#[no_mangle]
pub extern "C" fn brushpreview_render_mypaint(
    bp: &mut BrushPreview,
    brush: &MyPaintBrush,
    settings: &MyPaintSettings,
    shape: BrushPreviewShape,
) {
    bp.render_mypaint(brush, settings, shape);
}

#[no_mangle]
pub extern "C" fn brushpreview_floodfill(
    bp: &mut BrushPreview,
    color: &Color,
    tolerance: f32,
    expansion: i32,
    fill_under: bool,
) {
    bp.floodfill(*color, tolerance, expansion, fill_under);
}

#[no_mangle]
pub extern "C" fn brushpreview_paint(
    bp: &BrushPreview,
    ctx: *mut c_void,
    paint_func: extern "C" fn(ctx: *mut c_void, x: i32, y: i32, pixels: *const u8),
) {
    let opts = LayerViewOptions::default();
    let mut pixels = [ZERO_PIXEL8; TILE_LENGTH];
    FlattenedTileIterator::new(&bp.layerstack, &opts, AoE::Everything).for_each(|(i, j, t)| {
        pixels15_to_8(&mut pixels, &t.pixels);
        paint_func(
            ctx,
            i * TILE_SIZEI,
            j * TILE_SIZEI,
            pixels.as_ptr() as *const u8,
        )
    });
}
