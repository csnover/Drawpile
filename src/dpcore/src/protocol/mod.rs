// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

pub mod aclfilter;
pub mod message;
mod protover;
mod serialization;
mod textmessage;
pub mod textparser;

pub const DRAWPILE_VERSION: &str = env!("CARGO_PKG_VERSION");

pub use message::{Message, PROTOCOL_VERSION};
pub use protover::ProtocolVersion;
pub use serialization::{DeserializationError, MessageReader, MessageWriter, HEADER_LEN};
