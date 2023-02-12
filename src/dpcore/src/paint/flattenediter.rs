// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::aoe::{AoE, TileMap};
use super::layerstack::{LayerStack, LayerViewOptions};
use super::tile::{Tile, TileData, TILE_SIZE};

pub struct FlattenedTileIterator<'a> {
    layerstack: &'a LayerStack,
    opts: &'a LayerViewOptions,
    i: u32,
    j: u32,
    left: u32,
    right: u32,
    bottom: u32,
    tilemap: Option<TileMap>,
}

impl<'a> FlattenedTileIterator<'a> {
    pub fn new(layerstack: &'a LayerStack, opts: &'a LayerViewOptions, area: AoE) -> Self {
        let right = Tile::div_up(layerstack.root().width());
        let bottom = Tile::div_up(layerstack.root().height());
        match area {
            AoE::Resize(_, _, _) | AoE::Everything => FlattenedTileIterator {
                layerstack,
                opts,
                i: 0,
                j: 0,
                left: 0,
                right,
                bottom,
                tilemap: None,
            },
            AoE::Bitmap(tilemap) => FlattenedTileIterator {
                layerstack,
                opts,
                i: 0,
                j: 0,
                left: 0,
                right,
                bottom,
                tilemap: Some(tilemap),
            },
            AoE::Bounds(rect) => FlattenedTileIterator {
                layerstack,
                opts,
                i: rect.x as u32 / TILE_SIZE,
                j: rect.y as u32 / TILE_SIZE,
                left: rect.x as u32 / TILE_SIZE,
                right: Tile::div_up(rect.right() as u32),
                bottom: Tile::div_up(rect.bottom() as u32),
                tilemap: None,
            },
            AoE::Nothing => FlattenedTileIterator {
                layerstack,
                opts,
                i: 0,
                j: 1,
                left: 0,
                right: 0,
                bottom: 0,
                tilemap: None,
            },
        }
    }
}

impl<'a> Iterator for FlattenedTileIterator<'a> {
    type Item = (i32, i32, TileData);
    fn next(&mut self) -> Option<Self::Item> {
        loop {
            let (i, j) = (self.i, self.j);

            if j >= self.bottom {
                return None;
            }

            self.i += 1;
            if self.i >= self.right {
                self.i = self.left;
                self.j += 1;
            }

            if self.tilemap.as_ref().map_or(true, |tm| tm.is_set(i, j)) {
                return Some((
                    i as i32,
                    j as i32,
                    self.layerstack.flatten_tile(i, j, self.opts),
                ));
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::super::Rectangle;
    use super::*;

    #[test]
    fn test_everything() {
        let ls = LayerStack::new(100, 100);
        let opts = LayerViewOptions::default();
        let mut i = FlattenedTileIterator::new(&ls, &opts, AoE::Everything);

        let expected = [(0, 0), (1, 0), (0, 1), (1, 1)];

        for ex in &expected {
            let r = i.next().unwrap();
            assert_eq!((r.0, r.1), *ex);
        }
        assert!(i.next().is_none());
    }

    #[test]
    fn test_bounds() {
        let ls = LayerStack::new(1000, 1000);
        let opts = LayerViewOptions::default();
        let mut i =
            FlattenedTileIterator::new(&ls, &opts, AoE::Bounds(Rectangle::new(100, 100, 64, 64)));

        let expected = [(1, 1), (2, 1), (1, 2), (2, 2)];

        for ex in &expected {
            let r = i.next().unwrap();
            assert_eq!((r.0, r.1), *ex);
        }
        assert!(i.next().is_none());
    }

    #[test]
    fn test_tilemap() {
        let ls = LayerStack::new(5 * TILE_SIZE, 5 * TILE_SIZE);
        let opts = LayerViewOptions::default();
        let mut tm = TileMap::new(ls.root().width(), ls.root().height());
        tm.tiles.set(0, true);
        tm.tiles.set(6, true);
        tm.tiles.set(12, true);
        let mut i = FlattenedTileIterator::new(&ls, &opts, AoE::Bitmap(tm));

        let expected = [(0, 0), (1, 1), (2, 2)];

        for ex in &expected {
            let r = i.next().unwrap();
            assert_eq!((r.0, r.1), *ex);
        }
        assert!(i.next().is_none());
    }

    #[test]
    fn test_nothing() {
        let ls = LayerStack::new(100, 100);
        let opts = LayerViewOptions::default();
        let mut i = FlattenedTileIterator::new(&ls, &opts, AoE::Nothing);
        assert!(i.next().is_none());
    }
}
