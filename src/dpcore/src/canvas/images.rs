// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::compression::ImageCompressor;
use crate::paint::{Blendmode, Image8, LayerID, Rectangle, UserID};
use crate::protocol::message::{CommandMessage, PutImageMessage};

const MAX_IMAGE_LEN: usize = 0xffff - 19;

/// Make a PutImage command.
///
/// If the image is too big to fit into a single command,
/// multiple PutImages will be written.
pub fn make_putimage(
    user: UserID,
    layer: LayerID,
    x: u32,
    y: u32,
    image: &Image8,
    mode: Blendmode,
) -> Vec<CommandMessage> {
    let mut messages = Vec::new();
    make_putimage_crop(
        &mut messages,
        user,
        layer,
        x,
        y,
        image,
        mode,
        &Rectangle::new(0, 0, image.width as i32, image.height as i32),
    );
    messages
}

#[allow(clippy::too_many_arguments)]
fn make_putimage_crop(
    messages: &mut Vec<CommandMessage>,
    user: UserID,
    layer: LayerID,
    x: u32,
    y: u32,
    image: &Image8,
    mode: Blendmode,
    rect: &Rectangle,
) {
    // Try compressing this subregion of the image
    {
        let mut compressor = ImageCompressor::new();
        image.rect_iter(rect).for_each(|row| compressor.add(row));
        let compressed = compressor.finish();

        if compressed.len() <= MAX_IMAGE_LEN {
            messages.push(CommandMessage::PutImage(
                user,
                PutImageMessage {
                    layer,
                    mode: mode.into(),
                    x: x + rect.x as u32,
                    y: y + rect.y as u32,
                    w: rect.w as u32,
                    h: rect.h as u32,
                    image: compressed,
                },
            ));
            return;
        }
    }

    // Image too big to fit into a message. Split into four pieces and
    // try again
    // TODO: a smarter approach would be to split at tile boundaries to
    // make the PutImage operations more efficient
    let splitx = rect.w / 2;
    let splity = rect.h / 2;

    make_putimage_crop(
        messages,
        user,
        layer,
        x,
        y,
        image,
        mode,
        &Rectangle::new(rect.x, rect.y, splitx, splity),
    );
    make_putimage_crop(
        messages,
        user,
        layer,
        x,
        y,
        image,
        mode,
        &Rectangle::new(rect.x + splitx, rect.y, rect.w - splitx, splity),
    );
    make_putimage_crop(
        messages,
        user,
        layer,
        x,
        y,
        image,
        mode,
        &Rectangle::new(rect.x, rect.y + splity, splitx, rect.h - splity),
    );
    make_putimage_crop(
        messages,
        user,
        layer,
        x,
        y,
        image,
        mode,
        &Rectangle::new(
            rect.x + splitx,
            rect.y + splity,
            rect.w - splitx,
            rect.h - splity,
        ),
    );
}
