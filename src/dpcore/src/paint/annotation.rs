// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::{Color, Rectangle};
use num_enum::{IntoPrimitive, TryFromPrimitive};

pub type AnnotationID = u16;

/// A floating text box over the image
///
/// These are not strictly part of the canvas pixel data,
/// but they belong to the layerstack.
///
/// The core paint engine has no opinion on how annotations
/// are rasterized. To merge an annotation, it must be converted
/// to a bitmap on the client side, using fonts available there,
/// then merged using the PutImage command.
#[derive(Clone)]
pub struct Annotation {
    pub id: AnnotationID,
    pub text: String,
    pub rect: Rectangle,
    pub background: Color,
    pub protect: bool,
    pub valign: VAlign,
    pub border: u8,
}

#[derive(Clone, Copy, IntoPrimitive, TryFromPrimitive)]
#[repr(u8)]
pub enum VAlign {
    Top,
    Center,
    Bottom,
}
