// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::aoe::{AoE, TileMap};
use super::bitmaplayer::BitmapLayer;
use super::blendmode::Blendmode;
use super::grouplayer::GroupLayer;

use super::tile::TileData;
use super::{LayerID, UserID};

#[derive(Clone)]
pub enum Layer {
    Group(GroupLayer),
    Bitmap(BitmapLayer),
}

/// Common layer properties
#[derive(Clone, PartialEq)]
pub struct LayerMetadata {
    pub id: LayerID,
    pub title: String,
    pub opacity: f32,
    pub hidden: bool,
    pub censored: bool,
    pub blendmode: Blendmode,
    pub isolated: bool, // applies to group layers only
}

impl LayerMetadata {
    /// A layer is visible when it's not explicitly hidden and it's opacity is greater than zero
    ///
    /// The `hidden` flag is typically used as a local-only flag that allows a layer to be
    /// hidden just for the current user.
    pub fn is_visible(&self) -> bool {
        !self.hidden && self.opacity > 0.0
    }
}

impl Layer {
    pub fn as_group(&self) -> Option<&GroupLayer> {
        match self {
            Self::Group(g) => Some(g),
            _ => None,
        }
    }

    pub fn as_group_mut(&mut self) -> Option<&mut GroupLayer> {
        match self {
            Self::Group(g) => Some(g),
            _ => None,
        }
    }

    pub fn is_group(&self) -> bool {
        matches!(self, Self::Group(_))
    }

    pub fn as_bitmap(&self) -> Option<&BitmapLayer> {
        match self {
            Self::Bitmap(b) => Some(b),
            _ => None,
        }
    }

    pub fn as_bitmap_mut(&mut self) -> Option<&mut BitmapLayer> {
        match self {
            Self::Bitmap(b) => Some(b),
            _ => None,
        }
    }

    pub fn is_visible(&self) -> bool {
        match self {
            Layer::Group(g) => g.metadata.is_visible(),
            Layer::Bitmap(b) => b.metadata.is_visible(),
        }
    }

    pub fn id(&self) -> LayerID {
        match self {
            Layer::Group(g) => g.metadata.id,
            Layer::Bitmap(b) => b.metadata.id,
        }
    }

    pub fn metadata(&self) -> &LayerMetadata {
        match self {
            Layer::Group(g) => &g.metadata,
            Layer::Bitmap(b) => &b.metadata,
        }
    }

    pub fn metadata_mut(&mut self) -> &mut LayerMetadata {
        match self {
            Layer::Group(g) => &mut g.metadata,
            Layer::Bitmap(b) => &mut b.metadata,
        }
    }

    pub fn compare(&self, other: &Self) -> AoE {
        match (self, other) {
            (Layer::Group(s), Layer::Group(o)) => s.compare(o),
            (Layer::Bitmap(s), Layer::Bitmap(o)) => s.compare(o),
            _ => AoE::Everything,
        }
    }

    pub fn resized(&self, top: i32, right: i32, bottom: i32, left: i32) -> Option<Self> {
        Some(match self {
            Layer::Group(g) => Layer::Group(g.resized(top, right, bottom, left)?),
            Layer::Bitmap(b) => Layer::Bitmap(b.resized(top, right, bottom, left)?),
        })
    }

    pub fn nonblank_tilemap(&self) -> TileMap {
        match self {
            Layer::Group(g) => g.nonblank_tilemap(),
            Layer::Bitmap(b) => b.nonblank_tilemap(),
        }
    }

    pub fn flatten_tile(
        &self,
        destination: &mut TileData,
        i: u32,
        j: u32,
        parent_opacity: f32,
        censor: bool,
        highlight_id: UserID,
    ) {
        match self {
            Layer::Group(l) => {
                l.flatten_tile(destination, i, j, parent_opacity, censor, highlight_id)
            }
            Layer::Bitmap(l) => {
                l.flatten_tile(destination, i, j, parent_opacity, censor, highlight_id)
            }
        }
    }
}
