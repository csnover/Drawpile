// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::{BitmapLayer, LayerID, Tile, UserID};
use crate::canvas::compression::compress_tiledata;
use crate::protocol::message::{CommandMessage, Message, PutTileMessage};
use std::collections::HashMap;

pub struct TileRun {
    /// The tile to place
    pub tile: Tile,

    /// Column of the first tile
    col: i32,

    /// Row of the first tile
    row: i32,

    /// How many times to repeat this tile (at least 1)
    /// This may be longer than the width of the layer, the run
    /// will just wrap to the next row.
    len: i32,

    /// The tile's color, if solid
    solid_color: Option<u32>,
}

/// A layers tile content with adjacent identical tiles collapsed together.
///
/// This is used to efficiently set up a layer's content using PutTile commands.
/// The most common background color
pub struct LayerTileSet {
    pub tiles: Vec<TileRun>,
    pub background: u32,
}

impl From<&BitmapLayer> for LayerTileSet {
    fn from(layer: &BitmapLayer) -> Self {
        let tiles = layer.tilevec();
        assert!(!tiles.is_empty());

        let cols = Tile::div_up(layer.width()) as i32;

        let mut runs: Vec<TileRun> = Vec::new();

        // First, Run Length Encode the tile vector
        let mut last = TileRun {
            tile: tiles[0].clone(),
            col: 0,
            row: 0,
            len: 1,
            solid_color: tiles[0].solid_color().map(|c| c.as_argb32()),
        };

        for (i, tile) in tiles.iter().enumerate().skip(1) {
            if last.len < 0xffff && last.tile == *tile {
                last.len += 1;
            } else {
                runs.push(last);
                last = TileRun {
                    tile: tile.clone(),
                    col: i as i32 % cols,
                    row: i as i32 / cols,
                    len: 1,
                    solid_color: tile.solid_color().map(|c| c.as_argb32()),
                };
            }
        }
        runs.push(last);

        // Count the number of solid color tiles
        let mut colors: HashMap<u32, i32> = HashMap::new();

        for tr in runs.iter() {
            if let Some(c) = tr.solid_color {
                let count = colors.entry(c).or_insert(0);
                *count += 1;
            }
        }

        // If half of the tiles are of the same
        // solid color, use that as the background color.
        // Otherwise, transparency is a safe default choice.
        let mut background: u32 = 0;
        let threshold = tiles.len() as i32 / 2;
        for (c, count) in colors {
            if count >= threshold {
                background = c;
                break;
            }
        }

        // Remove solid color tiles that match the background color
        runs.retain(|tr| match tr.solid_color {
            Some(c) => c != background,
            None => true,
        });

        LayerTileSet {
            tiles: runs,
            background,
        }
    }
}

impl LayerTileSet {
    pub fn to_puttiles(
        &self,
        user: UserID,
        layer: LayerID,
        sublayer: UserID,
        msgs: &mut Vec<Message>,
    ) {
        for tr in self.tiles.iter() {
            msgs.push(Message::Command(CommandMessage::PutTile(
                user,
                PutTileMessage {
                    layer,
                    sublayer,
                    last_touch: tr.tile.last_touched_by(),
                    col: tr.col as u16,
                    row: tr.row as u16,
                    repeat: (tr.len - 1) as u16,
                    image: match tr.solid_color {
                        Some(c) => c.to_be_bytes().to_vec(),
                        None => compress_tiledata(tr.tile.data()),
                    },
                },
            )))
        }
    }
}
