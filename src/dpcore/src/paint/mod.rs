// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

pub mod annotation;
pub mod aoe;
pub mod color;
pub mod editlayer;
pub mod editstack;
pub mod floodfill;
pub mod layerstack;
pub mod rasterop;
pub mod rectiter;
pub mod tile;
pub mod tileiter;
pub mod tilevec;

mod idgenerator;

pub type UserID = u8;
pub type LayerID = u16;

/// The sub-layer ID used for local preview drawing (shape tools, etc.)
pub const PREVIEW_SUBLAYER_ID: LayerID = 0xffff;

// Re-export types most commonly used from the outside
mod bitmaplayer;
mod blendmode;
mod brushmask;
mod flattenediter;
mod grouplayer;
mod image;
mod layer;
mod rect;
mod timeline;

pub use self::image::{Image15, Image8};
pub use aoe::AoE;
pub use bitmaplayer::BitmapLayer;
pub use blendmode::Blendmode;
pub use brushmask::{BrushMask, ClassicBrushCache, MyPaintBrushCache};
pub use color::{
    channel15_to_8, channel8_to_15, pixel15_to_8, pixel8_to_15, pixels15_to_8, pixels8_to_15,
    Color, Pixel15, Pixel8, BIT15_F32, BIT15_U16,
};
pub use flattenediter::FlattenedTileIterator;
pub use grouplayer::{GroupLayer, LayerInsertion, RootGroup};
pub use layer::{Layer, LayerMetadata};
pub use layerstack::{DocumentMetadata, LayerStack, LayerViewMode, LayerViewOptions};
pub use rect::{Rectangle, Size};
pub use tile::Tile;
pub use tilevec::LayerTileSet;
pub use timeline::{Frame, Timeline};
