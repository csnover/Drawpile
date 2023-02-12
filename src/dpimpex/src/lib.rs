// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use dpcore::paint::LayerStack;
use image::error::ImageError;
use std::path::Path;
use std::{fmt, io};
use xml::writer::Error as XmlError;
use zip::result::ZipError;

pub mod animation;
pub mod conv;
mod flat;
mod ora;
pub mod rec;
pub mod rec_index;

#[derive(Debug)]
pub enum ImpexError {
    IoError(io::Error),
    CodecError(ImageError),
    UnsupportedFormat,
    XmlError(XmlError),
    NoContent,
}

impl fmt::Display for ImpexError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ImpexError::IoError(e) => e.fmt(f),
            ImpexError::CodecError(e) => e.fmt(f),
            ImpexError::UnsupportedFormat => write!(f, "unsupported format"),
            ImpexError::NoContent => write!(f, "no content"),
            ImpexError::XmlError(e) => e.fmt(f),
        }
    }
}

impl std::error::Error for ImpexError {
    fn cause(&self) -> Option<&dyn std::error::Error> {
        match self {
            ImpexError::IoError(e) => Some(e),
            ImpexError::CodecError(e) => Some(e),
            ImpexError::XmlError(e) => Some(e),
            _ => None,
        }
    }
}

impl From<io::Error> for ImpexError {
    fn from(err: io::Error) -> Self {
        Self::IoError(err)
    }
}

impl From<ImageError> for ImpexError {
    fn from(err: ImageError) -> Self {
        Self::CodecError(err)
    }
}

impl From<ZipError> for ImpexError {
    fn from(err: ZipError) -> Self {
        match err {
            ZipError::Io(io) => Self::IoError(io),
            _ => Self::UnsupportedFormat,
        }
    }
}

impl From<XmlError> for ImpexError {
    fn from(err: XmlError) -> Self {
        Self::XmlError(err)
    }
}

pub type ImageImportResult = Result<LayerStack, ImpexError>;
pub type ImageExportResult = Result<(), ImpexError>;

pub fn load_image<P>(path: P) -> ImageImportResult
where
    P: AsRef<Path>,
{
    fn inner(path: &Path) -> ImageImportResult {
        let ext = path
            .extension()
            .and_then(|s| s.to_str().map(|s| s.to_ascii_lowercase()));
        match ext.as_deref() {
            Some("ora") => ora::load_openraster_image(path),
            Some("gif") => flat::load_gif_animation(path),
            Some(_) => flat::load_flat_image(path),
            None => Err(ImpexError::UnsupportedFormat),
        }
    }
    inner(path.as_ref())
}

pub fn save_image<P>(path: P, layerstack: &LayerStack) -> ImageExportResult
where
    P: AsRef<Path>,
{
    fn inner(path: &Path, layerstack: &LayerStack) -> ImageExportResult {
        let ext = path
            .extension()
            .and_then(|s| s.to_str().map(|s| s.to_ascii_lowercase()));
        match ext.as_deref() {
            Some("ora") => ora::save_openraster_image(path, layerstack),
            Some(_) => flat::save_flat_image(path, layerstack),
            None => Err(ImpexError::UnsupportedFormat),
        }
    }
    inner(path.as_ref(), layerstack)
}
