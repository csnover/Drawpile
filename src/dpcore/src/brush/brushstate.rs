// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use crate::paint::BitmapLayer;
use crate::protocol::message::CommandMessage;
use crate::protocol::MessageWriter;

pub trait BrushState {
    /// Set the target layer
    fn set_layer(&mut self, layer_id: u16);

    /// Draw a brush stroke to this point
    ///
    /// If there is no active stroke, this becomes the starting point
    ///
    /// If a source layer is given, it is used as the source of color smudging pixels
    fn stroke_to(&mut self, x: f32, y: f32, p: f32, delta_msec: i64, source: Option<&BitmapLayer>);

    /// End the current stroke (if any)
    fn end_stroke(&mut self);

    /// Take the dabs computed so far and write them with the given message writer
    /// Doing this will not end the current stroke.
    fn write_dabs(&mut self, user_id: u8, writer: &mut MessageWriter);

    /// Take the dabs computed so far.
    /// Doing this will not end the current stroke.
    fn take_dabs(&mut self, user_id: u8) -> Vec<CommandMessage>;

    /// Add an offset to the current position.
    ///
    /// This is used to correct the active position when the viewport
    /// is moved while the user is still drawing.
    ///
    /// Does nothing if there is no stroke in progress.
    fn add_offset(&mut self, x: f32, y: f32);
}
