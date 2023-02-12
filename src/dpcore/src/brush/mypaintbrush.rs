// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: askmeaboutloom

use crate::paint::Color;

#[derive(Clone, Copy)]
#[repr(C)]
pub struct ControlPoints {
    pub xvalues: [f32; 64],
    pub yvalues: [f32; 64],
    pub n: i32,
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct MyPaintMapping {
    pub base_value: f32,
    // This 18 must be hard-coded because of cbindgen limitations. It's really
    // referring to the value of MYPAINT_BRUSH_INPUTS_COUNT. There's a static
    // assertion in the C++ code to make sure that these match up.
    pub inputs: [ControlPoints; 18usize],
}

// Settings are split off from the brush because they're huge and blow up the
// stack if allocated there and then cloned a bunch. So instead we keep them
// as separate, heap-allocated objects and only pass pointers to them around.
#[derive(Clone, Copy)]
#[repr(C)]
pub struct MyPaintSettings {
    // The 64 must also be hard-coded because of cbindgen. This one's referring
    // to MYPAINT_BRUSH_SETTINGS_COUNT.
    pub mappings: [MyPaintMapping; 64usize],
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct MyPaintBrush {
    // Color is only stored as HSV in the brush, we track the RGB separately.
    pub color: Color,
    // Forces locked alpha on all dabs, acting as Recolor mode.
    pub lock_alpha: bool,
    // Forces transparent color on all dabs, acting as Erase mode.
    pub erase: bool,
}
