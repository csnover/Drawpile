// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

// This could be made generic with num_traits
pub struct IDGenerator {
    used_ids: Vec<u8>,
    next_id: Option<u8>,
}

impl IDGenerator {
    pub fn new(first_id: u8, used_ids: Vec<u8>) -> Self {
        assert!(!used_ids.contains(&first_id));
        Self {
            used_ids,
            next_id: Some(first_id),
        }
    }

    pub fn take_next(&mut self) -> Option<u8> {
        if let Some(id) = self.next_id {
            self.used_ids.push(id);

            let mut next_id = id.wrapping_add(1);

            while next_id != id && self.used_ids.contains(&next_id) {
                next_id = next_id.wrapping_add(1);
            }

            if next_id != id {
                self.next_id = Some(next_id);
            } else {
                self.next_id = None;
            }

            Some(id)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_id_generation() {
        let mut idgen = IDGenerator::new(1, vec![2, 4]);
        assert_eq!(idgen.take_next(), Some(1));
        assert_eq!(idgen.take_next(), Some(3));
        assert_eq!(idgen.take_next(), Some(5));
        assert_eq!(idgen.take_next(), Some(6));
    }

    #[test]
    fn test_id_wraparound() {
        let mut idgen = IDGenerator::new(1, vec![2]);
        assert_eq!(idgen.take_next(), Some(1));
        assert_eq!(idgen.take_next(), Some(3));
        idgen.next_id = Some(255);
        assert_eq!(idgen.take_next(), Some(255));
        assert_eq!(idgen.take_next(), Some(0));
        assert_eq!(idgen.take_next(), Some(4));
    }
}
