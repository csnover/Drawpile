// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::color::{Pixel15, Pixel8};
use super::rectiter::{MutableRectIterator, RectIterator};
use super::{Rectangle, Size};

/// A flat image buffer
#[derive(Default)]
pub struct Image<T>
where
    T: Clone + Default + Eq,
{
    pub pixels: Vec<T>,
    pub width: usize,
    pub height: usize,
}

pub type Image8 = Image<Pixel8>;
pub type Image15 = Image<Pixel15>;

impl<T> Image<T>
where
    T: Clone + Default + Eq,
{
    pub fn new(width: usize, height: usize) -> Image<T> {
        Image {
            pixels: vec![T::default(); width * height],
            width,
            height,
        }
    }

    pub fn is_null(&self) -> bool {
        assert!(self.pixels.len() == self.width * self.height);
        self.pixels.is_empty()
    }

    pub fn size(&self) -> Size {
        Size::new(self.width as i32, self.height as i32)
    }

    /// Find the bounding rectangle of the opaque pixels in this image
    /// If the image is entirely transparent, None is returned.
    pub fn opaque_bounds(&self) -> Option<Rectangle> {
        assert!(self.pixels.len() == self.width * self.height);
        if self.pixels.is_empty() {
            return None;
        }

        let mut top = self.height;
        let mut btm = 0;
        let mut left = self.width;
        let mut right = 0;

        let zero_pixel = T::default();
        for y in 0..self.height {
            let row = y * self.width;
            for (x, px) in self.pixels[row..row + self.width].iter().enumerate() {
                if *px != zero_pixel {
                    left = left.min(x);
                    right = right.max(x);
                    top = top.min(y);
                    btm = btm.max(y);
                }
            }
        }

        if top > btm {
            return None;
        }

        Some(Rectangle {
            x: left as i32,
            y: top as i32,
            w: (right - left + 1) as i32,
            h: (btm - top + 1) as i32,
        })
    }

    pub fn rect_iter(&self, rect: &Rectangle) -> RectIterator<T> {
        RectIterator::from_rectangle(&self.pixels, self.width, rect)
    }

    pub fn rect_iter_mut(&mut self, rect: &Rectangle) -> MutableRectIterator<T> {
        MutableRectIterator::from_rectangle(&mut self.pixels, self.width, rect)
    }

    /// Return a cropped version of the image
    pub fn cropped(&self, rect: &Rectangle) -> Image<T> {
        let rect = match rect.cropped(self.size()) {
            Some(r) => r,
            None => return Image::default(),
        };

        let mut cropped_pixels: Vec<T> = Vec::with_capacity((rect.w * rect.h) as usize);
        self.rect_iter(&rect)
            .for_each(|p| cropped_pixels.extend_from_slice(p));

        Image {
            pixels: cropped_pixels,
            width: rect.w as usize,
            height: rect.h as usize,
        }
    }
}
