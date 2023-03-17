// SPDX-License-Identifier: GPL-3.0-or-later WITH LicenseRef-AppStore
// SPDX-FileCopyrightText: Calle Laakkonen

use num_enum::IntoPrimitive;
use num_enum::TryFromPrimitive;

#[derive(Copy, Clone, Debug, Default, Eq, PartialEq, IntoPrimitive, TryFromPrimitive)]
#[repr(u8)]
pub enum Blendmode {
    Erase = 0,
    #[default]
    Normal,
    Multiply,
    Divide,
    Burn,
    Dodge,
    Darken,
    Lighten,
    Subtract,
    Add,
    Recolor,
    Behind,
    ColorErase,
    Screen,
    NormalAndEraser,
    LuminosityShineSai,
    Overlay,
    HardLight,
    SoftLight,
    LinearBurn,
    LinearLight,
    Hue,
    Saturation,
    Luminosity,
    Color,
    Replace = 255,
}

impl Blendmode {
    pub fn can_decrease_opacity(self) -> bool {
        matches!(
            self,
            Blendmode::Erase
                | Blendmode::ColorErase
                | Blendmode::NormalAndEraser
                | Blendmode::Replace
        )
    }

    pub fn can_increase_opacity(self) -> bool {
        matches!(
            self,
            Blendmode::Normal | Blendmode::Behind | Blendmode::NormalAndEraser | Blendmode::Replace
        )
    }

    pub fn is_eraser_mode(self) -> bool {
        matches!(self, Blendmode::Erase | Blendmode::ColorErase)
    }

    pub fn svg_name(self) -> &'static str {
        use Blendmode::*;
        match self {
            Erase => "-dp-erase",
            Normal => "svg:src-over",
            Multiply => "svg:multiply",
            Divide => "-dp-divide",
            Burn => "svg:color-burn",
            Dodge => "svg:color-dodge",
            Darken => "svg:darken",
            Lighten => "svg:lighten",
            Subtract => "-dp-minus", // not in SVG spec
            Add => "svg:plus",
            Recolor => "svg:src-atop",
            Behind => "svg:dst-over",
            ColorErase => "-dp-cerase",
            Screen => "svg:screen",
            NormalAndEraser => "-dp-normal-and-eraser",
            LuminosityShineSai => "krita:luminosity_sai",
            Overlay => "svg:overlay",
            HardLight => "svg:hard-light",
            SoftLight => "svg:soft-light",
            LinearBurn => "krita:linear_burn",
            LinearLight => "krita:linear light",
            Hue => "hue",
            Saturation => "saturation",
            Luminosity => "luminosity",
            Color => "color",
            Replace => "-dp-replace",
        }
    }

    pub fn from_svg_name(name: &str) -> Option<Self> {
        let name = name.strip_prefix("svg:").unwrap_or(name);

        use Blendmode::*;
        Some(match name {
            "-dp-erase" => Erase,
            "src-over" => Normal,
            "multiply" => Multiply,
            "-dp-divide" => Divide,
            "color-burn" => Burn,
            "color-dodge" => Dodge,
            "darken" => Darken,
            "lighten" => Lighten,
            "-dp-minus" => Subtract,
            "plus" => Add,
            "src-atop" => Recolor,
            "dst-over" => Behind,
            "-dp-cerase" => ColorErase,
            "screen" => Screen,
            "-dp-replace" => Replace,
            "-dp-normal-and-eraser" => NormalAndEraser,
            "krita:luminosity_sai" => LuminosityShineSai,
            "overlay" => Overlay,
            "hard-light" => HardLight,
            "soft-light" => SoftLight,
            "krita:linear_burn" => LinearBurn,
            "krita:linear light" => LinearLight,
            "hue" => Hue,
            "saturation" => Saturation,
            "luminosity" => Luminosity,
            "color" => Color,
            _ => {
                return None;
            }
        })
    }
}
