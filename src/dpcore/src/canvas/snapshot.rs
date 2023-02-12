// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use super::compression::compress_tile;
use crate::paint::annotation::VAlign;
use crate::paint::{BitmapLayer, GroupLayer, Layer, LayerID, LayerStack, LayerTileSet, UserID};
use crate::protocol::aclfilter::{userbits_to_vec, AclFilter};
use crate::protocol::message::*;

/// Create a sequence of commands that reproduces the current state of the canvas
pub fn make_canvas_snapshot(
    user: UserID,
    layerstack: &LayerStack,
    default_layer: LayerID,
    aclfilter: Option<&AclFilter>,
) -> Vec<Message> {
    // First, initialize the canvas
    let mut msgs = vec![
        Message::Command(CommandMessage::CanvasResize(
            user,
            CanvasResizeMessage {
                top: 0,
                right: layerstack.root().width() as i32,
                bottom: layerstack.root().height() as i32,
                left: 0,
            },
        )),
        Message::Command(CommandMessage::CanvasBackground(
            user,
            compress_tile(&layerstack.background),
        )),
    ];

    if default_layer > 0 {
        msgs.push(Message::ClientMeta(ClientMetaMessage::DefaultLayer(
            user,
            default_layer,
        )));
    }

    // Create layers (recursive)
    create_layers(user, layerstack.root().inner_ref(), aclfilter, &mut msgs);

    // Create annotations
    for a in layerstack.get_annotations().iter() {
        msgs.push(Message::Command(CommandMessage::AnnotationCreate(
            user,
            AnnotationCreateMessage {
                id: a.id,
                x: a.rect.x,
                y: a.rect.y,
                w: a.rect.w as u16,
                h: a.rect.h as u16,
            },
        )));

        msgs.push(Message::Command(CommandMessage::AnnotationEdit(
            user,
            AnnotationEditMessage {
                id: a.id,
                bg: a.background.as_argb32(),
                flags: if a.protect {
                    AnnotationEditMessage::FLAGS_PROTECT
                } else {
                    0
                } | match a.valign {
                    VAlign::Top => 0,
                    VAlign::Center => AnnotationEditMessage::FLAGS_VALIGN_CENTER,
                    VAlign::Bottom => AnnotationEditMessage::FLAGS_VALIGN_BOTTOM,
                },
                border: 0,
                text: a.text.clone(),
            },
        )));
    }

    // Metadata
    let md = layerstack.metadata();
    msgs.push(Message::Command(CommandMessage::SetMetadataInt(
        user,
        SetMetadataIntMessage {
            field: u8::from(MetadataInt::Dpix),
            value: md.dpix,
        },
    )));
    msgs.push(Message::Command(CommandMessage::SetMetadataInt(
        user,
        SetMetadataIntMessage {
            field: u8::from(MetadataInt::Dpiy),
            value: md.dpiy,
        },
    )));
    msgs.push(Message::Command(CommandMessage::SetMetadataInt(
        user,
        SetMetadataIntMessage {
            field: u8::from(MetadataInt::Framerate),
            value: md.framerate,
        },
    )));
    if md.use_timeline {
        msgs.push(Message::Command(CommandMessage::SetMetadataInt(
            user,
            SetMetadataIntMessage {
                field: u8::from(MetadataInt::UseTimeline),
                value: 1,
            },
        )));
    }

    // Timeline
    for (i, f) in layerstack.timeline().frames.iter().enumerate() {
        msgs.push(Message::Command(CommandMessage::SetTimelineFrame(
            user,
            SetTimelineFrameMessage {
                frame: i as u16,
                insert: false,
                layers: f.iter().collect(),
            },
        )));
    }

    // ACLs
    if let Some(acl) = aclfilter {
        msgs.push(Message::ClientMeta(ClientMetaMessage::FeatureAccessLevels(
            user,
            vec![
                acl.feature_tiers().put_image.into(),
                acl.feature_tiers().move_rect.into(),
                acl.feature_tiers().resize.into(),
                acl.feature_tiers().background.into(),
                acl.feature_tiers().edit_layers.into(),
                acl.feature_tiers().own_layers.into(),
                acl.feature_tiers().create_annotation.into(),
                acl.feature_tiers().laser.into(),
                acl.feature_tiers().undo.into(),
                acl.feature_tiers().metadata.into(),
                acl.feature_tiers().timeline.into(),
            ],
        )));

        msgs.push(Message::ClientMeta(ClientMetaMessage::UserACL(
            user,
            userbits_to_vec(&acl.users().locked),
        )));
    }

    msgs
}

fn create_layers(
    user: UserID,
    group: &GroupLayer,
    aclfilter: Option<&AclFilter>,
    msgs: &mut Vec<Message>,
) {
    for layer in group.iter_layers().rev() {
        match layer {
            Layer::Group(g) => {
                create_group(user, g, group.metadata().id, msgs);
                create_layers(user, g, aclfilter, msgs);
            }
            Layer::Bitmap(b) => {
                create_layer(user, b, group.metadata().id, msgs);
            }
        }

        // Set Layer ACLs (if found)
        if let Some(acl) = aclfilter {
            if let Some(layeracl) = acl.layers().get(&layer.id()) {
                msgs.push(Message::ClientMeta(ClientMetaMessage::LayerACL(
                    user,
                    LayerACLMessage {
                        id: layer.id(),
                        flags: layeracl.flags(),
                        exclusive: userbits_to_vec(&layeracl.exclusive),
                    },
                )))
            }
        }
    }
}

fn create_group(user: UserID, layer: &GroupLayer, into: LayerID, msgs: &mut Vec<Message>) {
    let metadata = layer.metadata();

    msgs.push(Message::Command(CommandMessage::LayerCreate(
        user,
        LayerCreateMessage {
            id: metadata.id,
            source: 0,
            target: into,
            flags: LayerCreateMessage::FLAGS_GROUP
                | if into > 0 {
                    LayerCreateMessage::FLAGS_INTO
                } else {
                    0
                },
            fill: 0,
            name: metadata.title.clone(),
        },
    )));

    msgs.push(Message::Command(CommandMessage::LayerAttributes(
        user,
        LayerAttributesMessage {
            id: metadata.id,
            sublayer: 0,
            flags: if metadata.censored {
                LayerAttributesMessage::FLAGS_CENSOR
            } else {
                0
            } | if metadata.isolated {
                LayerAttributesMessage::FLAGS_ISOLATED
            } else {
                0
            },
            opacity: (metadata.opacity * 255.0) as u8,
            blend: metadata.blendmode.into(),
        },
    )));
}

fn create_layer(user: UserID, layer: &BitmapLayer, into: LayerID, msgs: &mut Vec<Message>) {
    let tileset = LayerTileSet::from(layer);
    let metadata = layer.metadata();

    msgs.push(Message::Command(CommandMessage::LayerCreate(
        user,
        LayerCreateMessage {
            id: metadata.id,
            source: 0,
            target: into,
            flags: if into > 0 {
                LayerCreateMessage::FLAGS_INTO
            } else {
                0
            },
            fill: tileset.background,
            name: metadata.title.clone(),
        },
    )));

    msgs.push(Message::Command(CommandMessage::LayerAttributes(
        user,
        LayerAttributesMessage {
            id: metadata.id,
            sublayer: 0,
            flags: if metadata.censored {
                LayerAttributesMessage::FLAGS_CENSOR
            } else {
                0
            },
            opacity: (metadata.opacity * 255.0) as u8,
            blend: metadata.blendmode.into(),
        },
    )));

    tileset.to_puttiles(user, metadata.id, 0, msgs);

    // Put active sublayer content (if any)
    for sl in layer.iter_sublayers() {
        if sl.metadata().id > 0 && sl.metadata().id < 256 {
            LayerTileSet::from(sl).to_puttiles(user, metadata.id, sl.metadata().id as u8, msgs);
        }
    }
}
