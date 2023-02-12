// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: askmeaboutloom

use crate::paint::{BitmapLayer, Color};
use dpmypaint as c;
use std::option::Option;

pub trait MyPaintTarget {
    fn add_dab(
        &mut self,
        x: f32,
        y: f32,
        radius: f32,
        hardness: f32,
        opacity: f32,
        r: f32,
        g: f32,
        b: f32,
        a: f32,
        aspect_ratio: f32,
        angle: f32,
        lock_alpha: f32,
        colorize: f32,
    );
}

#[repr(C)]
pub struct MyPaintSurface<'a> {
    parent: c::MyPaintSurface,
    source: Option<&'a BitmapLayer>,
    target: &'a mut dyn MyPaintTarget,
}

unsafe extern "C" fn draw_dab(
    parent: *mut c::MyPaintSurface,
    x: f32,
    y: f32,
    radius: f32,
    color_r: f32,
    color_g: f32,
    color_b: f32,
    opaque: f32,
    hardness: f32,
    alpha_eraser: f32,
    aspect_ratio: f32,
    angle: f32,
    lock_alpha: f32,
    colorize: f32,
) -> ::std::os::raw::c_int {
    let surface = &mut *(parent as *mut MyPaintSurface);
    surface
        .draw_dab(
            x,
            y,
            radius,
            color_r,
            color_g,
            color_b,
            opaque,
            hardness,
            alpha_eraser,
            aspect_ratio,
            angle,
            lock_alpha,
            colorize,
        )
        .into()
}

unsafe extern "C" fn get_color(
    parent: *mut c::MyPaintSurface,
    x: f32,
    y: f32,
    radius: f32,
    color_r: *mut f32,
    color_g: *mut f32,
    color_b: *mut f32,
    color_a: *mut f32,
) {
    let surface = &mut *(parent as *mut MyPaintSurface);
    let color = surface.get_color(x, y, radius);
    *color_r = color.r;
    *color_g = color.g;
    *color_b = color.b;
    *color_a = color.a;
}

unsafe extern "C" fn begin_atomic(_parent: *mut c::MyPaintSurface) {
    // Nothing to do
}

unsafe extern "C" fn end_atomic(_parent: *mut c::MyPaintSurface, _roi: *mut c::MyPaintRectangle) {
    // Nothing to do
}

impl<'a> MyPaintSurface<'a> {
    pub fn new(source: Option<&'a BitmapLayer>, target: &'a mut dyn MyPaintTarget) -> Self {
        // The compiler gets confused between fn pointers and fn items here, so
        // all the functions have to be cast to their own type to make it work.
        let mut surface = MyPaintSurface {
            parent: c::MyPaintSurface {
                draw_dab: Option::from(
                    draw_dab
                        as unsafe extern "C" fn(
                            *mut c::MyPaintSurface,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                            f32,
                        ) -> ::std::os::raw::c_int,
                ),
                get_color: Option::from(
                    get_color
                        as unsafe extern "C" fn(
                            *mut c::MyPaintSurface,
                            f32,
                            f32,
                            f32,
                            *mut f32,
                            *mut f32,
                            *mut f32,
                            *mut f32,
                        ),
                ),
                begin_atomic: Option::from(
                    begin_atomic as unsafe extern "C" fn(*mut c::MyPaintSurface),
                ),
                end_atomic: Option::from(
                    end_atomic
                        as unsafe extern "C" fn(
                            *mut c::MyPaintSurface,
                            roi: *mut c::MyPaintRectangle,
                        ),
                ),
                destroy: Option::None,
                save_png: Option::None,
                refcount: 1,
            },
            source,
            target,
        };
        unsafe { c::mypaint_surface_init(&mut surface.parent) }
        surface
    }

    pub unsafe fn get_raw(&mut self) -> *mut c::MyPaintSurface {
        &mut self.parent
    }

    fn draw_dab(
        &mut self,
        x: f32,
        y: f32,
        radius: f32,
        color_r: f32,
        color_g: f32,
        color_b: f32,
        opaque: f32,
        hardness: f32,
        alpha_eraser: f32,
        aspect_ratio: f32,
        angle: f32,
        lock_alpha: f32,
        colorize: f32,
    ) -> bool {
        self.target.add_dab(
            x,
            y,
            radius,
            hardness,
            opaque,
            color_r,
            color_g,
            color_b,
            alpha_eraser,
            aspect_ratio,
            angle,
            lock_alpha,
            colorize,
        );
        true
    }

    fn get_color(&mut self, x: f32, y: f32, radius: f32) -> Color {
        match self.source {
            Some(bitmap_layer) => {
                bitmap_layer.sample_color(x as i32, y as i32, (radius * 2.0) as i32)
            }
            None => Color::TRANSPARENT,
        }
    }
}
