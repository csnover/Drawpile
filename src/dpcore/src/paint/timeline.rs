// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::LayerID;
use std::fmt;

/// The timeline stores information about animation frames
///
/// A frame is a list of layers to be shown. The following kinds
/// of layers can be included in a frame
///
///  - individual bitmap or group layers
///  - layers from non-isolated groups
///
/// Non-isolated groups can be used to create logical groups of subframes,
/// such as individual character animations.
#[derive(Clone, Eq, PartialEq)]
pub struct Timeline {
    /// The timeline frames
    ///
    /// The length of the timeline vector is the length of the animation.
    pub frames: Vec<Frame>,
}

/// The number of layers that can be included in a frame is fixed.
/// This is done purely for efficiency: a Vec is 24 bytes long
/// (assuming 64bit OS,) so we can fit 12 layer IDs in the same
/// space without using any allocations.
///
/// Unused indices should be zerod. No more layers should be listed
/// after the first zero.
#[derive(Clone, Copy, Eq, PartialEq)]
#[repr(transparent)]
pub struct Frame(pub [LayerID; 12]);

impl Timeline {
    pub fn new() -> Self {
        Self { frames: Vec::new() }
    }
}

impl Default for Timeline {
    fn default() -> Self {
        Self::new()
    }
}

impl TryFrom<&[LayerID]> for Frame {
    type Error = ();

    fn try_from(layers: &[LayerID]) -> Result<Self, Self::Error> {
        if layers.len() > 12 {
            Err(())
        } else {
            let mut f = Frame::empty();
            f.0[..layers.len()].copy_from_slice(layers);
            Ok(f)
        }
    }
}

impl Frame {
    pub fn single(layer: LayerID) -> Self {
        Self([layer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])
    }

    pub fn empty() -> Self {
        Self([0; 12])
    }

    pub fn contains(&self, layer: LayerID) -> bool {
        self.iter().any(|l| l == layer)
    }

    pub fn iter(&self) -> impl Iterator<Item = LayerID> + '_ {
        self.0.iter().copied().take_while(|&f| f != 0)
    }

    /// Find the given layer in the list of frames
    /// Returns the earliest frame found
    pub fn find(layer: LayerID, frames: &[Frame]) -> Option<usize> {
        for (i, f) in frames.iter().enumerate() {
            if f.contains(layer) {
                return Some(i);
            }
        }
        None
    }
}

impl Default for Frame {
    fn default() -> Self {
        Self::empty()
    }
}

impl fmt::Debug for Frame {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Frame(")?;
        for l in self.0 {
            if l == 0 {
                break;
            }
            write!(f, "0x{l:04x}, ")?;
        }
        write!(f, ")")
    }
}