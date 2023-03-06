// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use dpcore::protocol::Message;
use std::collections::HashMap;
use std::io;
use std::io::Write;

pub trait RecordingWriter {
    /// Write the file header, including the metadata
    ///
    /// This should be called before the first write_message call
    fn write_header(&mut self, metadata: &HashMap<String, String>) -> io::Result<()>;

    /// Write a message into the file
    fn write_message(&mut self, m: &Message) -> io::Result<()>;
}

pub struct BinaryWriter<W> {
    file: W,
}

impl<W: Write> BinaryWriter<W> {
    pub fn open(file: W) -> BinaryWriter<W> {
        BinaryWriter { file }
    }

    pub fn into_inner(self) -> W {
        self.file
    }
}

impl<W: Write> RecordingWriter for BinaryWriter<W> {
    fn write_header(&mut self, metadata: &HashMap<String, String>) -> io::Result<()> {
        self.file.write_all(&b"DPREC\0"[..])?;

        let metadata = serde_json::to_string(&metadata)
            .map_err(|e| io::Error::new(io::ErrorKind::Other, e.to_string()))?;
        assert!(metadata.len() <= 0xffff);
        let metadata_len = (metadata.len() as u16).to_be_bytes();
        self.file.write_all(&metadata_len)?;
        self.file.write_all(metadata.as_bytes())?;

        Ok(())
    }

    fn write_message(&mut self, m: &Message) -> io::Result<()> {
        self.file.write_all(&m.serialize())
    }
}

pub struct TextWriter<W> {
    file: W,
}

impl<W: Write> TextWriter<W> {
    pub fn open(file: W) -> Self {
        TextWriter { file }
    }

    pub fn into_inner(self) -> W {
        self.file
    }
}

impl<W: Write> RecordingWriter for TextWriter<W> {
    fn write_header(&mut self, metadata: &HashMap<String, String>) -> io::Result<()> {
        for (k, v) in metadata.iter() {
            writeln!(self.file, "!{k}={v}")?;
        }
        Ok(())
    }

    fn write_message(&mut self, m: &Message) -> io::Result<()> {
        writeln!(self.file, "{}", m.as_text())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use dpcore::protocol::message::*;
    use std::io::Cursor;
    use std::str;

    fn test_writer(writer: &mut dyn RecordingWriter) {
        let mut header = HashMap::<String, String>::new();
        header.insert("version".to_string(), "1.0".to_string());

        writer.write_header(&header).unwrap();
        writer
            .write_message(
                &ServerMetaMessage::Join(
                    1,
                    JoinMessage {
                        flags: 0x03,
                        name: "XYZ".to_string(),
                        avatar: Vec::new(),
                    },
                )
                .into(),
            )
            .unwrap();
    }

    #[test]
    fn test_binary_writer() {
        let mut writer = BinaryWriter::open(Cursor::new(Vec::<u8>::new()));
        test_writer(&mut writer);
        let buf = writer.into_inner().into_inner();

        assert_eq!(
            buf,
            &b"DPREC\0\0\x11{\"version\":\"1.0\"}\0\x05\x20\x01\x03\x03XYZ"[..]
        );
    }

    #[test]
    fn test_text_writer() {
        let mut writer = TextWriter::open(Cursor::new(Vec::<u8>::new()));
        test_writer(&mut writer);
        let buf = str::from_utf8(&writer.into_inner().into_inner())
            .unwrap()
            .to_string();

        assert_eq!(buf, "!version=1.0\n1 join flags=auth,mod name=XYZ\n");
    }
}