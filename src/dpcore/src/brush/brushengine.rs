// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::brushstate::BrushState;
use super::classicbrush::ClassicBrush;
use super::mypaintbrush::{MyPaintBrush, MyPaintSettings};
use super::mypaintbrushstate::MyPaintBrushState;
use super::pixelbrushstate::PixelBrushState;
use super::softbrushstate::SoftBrushState;
use crate::paint::BitmapLayer;
use crate::protocol::message::CommandMessage;
use crate::protocol::MessageWriter;

pub struct BrushEngine {
    pixel: PixelBrushState,
    soft: SoftBrushState,
    mypaint: MyPaintBrushState,
    active: ActiveBrush,
}

enum ActiveBrush {
    ClassicPixel,
    ClassicSoft,
    MyPaint,
}

impl BrushEngine {
    pub fn new() -> BrushEngine {
        BrushEngine {
            pixel: PixelBrushState::new(),
            soft: SoftBrushState::new(),
            mypaint: MyPaintBrushState::new(),
            active: ActiveBrush::ClassicPixel,
        }
    }

    pub fn set_classicbrush(&mut self, brush: ClassicBrush) {
        if brush.is_pixelbrush() {
            self.pixel.set_brush(brush);
            self.active = ActiveBrush::ClassicPixel;
        } else {
            self.soft.set_brush(brush);
            self.active = ActiveBrush::ClassicSoft;
        }
    }

    pub fn set_mypaintbrush(
        &mut self,
        brush: &MyPaintBrush,
        settings: &MyPaintSettings,
        freehand: bool,
    ) {
        self.mypaint.set_brush(brush, settings, freehand);
        self.active = ActiveBrush::MyPaint;
    }

    fn state(&mut self) -> &mut dyn BrushState {
        match self.active {
            ActiveBrush::ClassicPixel => &mut self.pixel,
            ActiveBrush::ClassicSoft => &mut self.soft,
            ActiveBrush::MyPaint => &mut self.mypaint,
        }
    }
}

impl BrushState for BrushEngine {
    fn set_layer(&mut self, layer_id: u16) {
        self.pixel.set_layer(layer_id);
        self.soft.set_layer(layer_id);
        self.mypaint.set_layer(layer_id);
    }

    fn stroke_to(&mut self, x: f32, y: f32, p: f32, delta_msec: i64, source: Option<&BitmapLayer>) {
        self.state().stroke_to(x, y, p, delta_msec, source);
    }

    fn end_stroke(&mut self) {
        self.state().end_stroke();
    }

    fn take_dabs(&mut self, user_id: u8) -> Vec<CommandMessage> {
        self.state().take_dabs(user_id)
    }

    fn write_dabs(&mut self, user_id: u8, writer: &mut MessageWriter) {
        self.state().write_dabs(user_id, writer);
    }

    fn add_offset(&mut self, x: f32, y: f32) {
        self.state().add_offset(x, y);
    }
}

impl Default for BrushEngine {
    fn default() -> Self {
        Self::new()
    }
}
