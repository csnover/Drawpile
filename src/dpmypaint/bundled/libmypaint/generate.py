#!/usr/bin/env python
# libmypaint - The MyPaint Brush Library
# Copyright (C) 2007-2012 Martin Renold <martinxyz@gmx.ch>
# Copyright (C) 2012-2016 by the MyPaint Development Team.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


"Code generator, part of the build process."

from __future__ import absolute_import, division, print_function

import os
import sys
from os.path import basename
import json
from collections import namedtuple

PY3 = sys.version_info >= (3,)

# A basic translator comment is generated for each string,
# noting whether it is an input or a setting, and for tooltips
# stating which input/setting it belongs to.
#
# In addition to that, more descriptive addendums can be added
# for individual strings using the tcomment_x attributes, where
# x is either 'name' or 'tooltip'.

_SETTINGS = []  # brushsettings.settings
_SETTING_ORDER = [
    "internal_name",  # cname
    "tcomment_name",  # comment for translators (optional)
    "displayed_name",   # name
    "constant",
    "minimum",  # min
    "default",
    "maximum",  # max
    "tcomment_tooltip",  # comment for translators (optional)
    "tooltip",
]
_INPUTS = []  # brushsettings.inputs
_INPUT_ORDER = [
    "id",   # name
    "hard_minimum",   # hard_min
    "soft_minimum",   # soft_min
    "normal",
    "soft_maximum",   # soft_max
    "hard_maximum",     # hard_max
    "tcomment_name",  # comment for translators (optional)
    "displayed_name",  # dname
    "tcomment_tooltip",  # comment for translators (optional)
    "tooltip",
]
_STATES = []   # brushsettings.states


class _BrushSetting (namedtuple("_BrushSetting", _SETTING_ORDER)):

    def validate(self):
        msg = "Failed to validate %s: %r" % (self.internal_name, self)
        if self.minimum and self.maximum:
            assert (self.minimum <= self.default), msg
            assert (self.maximum >= self.default), msg
            assert (self.minimum < self.maximum), msg
        assert self.default is not None


class _BrushInput (namedtuple("_BrushInput", _INPUT_ORDER)):

    def validate(self):
        msg = "Failed to validate %s: %r" % (self.id, self)
        if self.hard_maximum is not None:
            assert (self.hard_maximum >= self.soft_maximum), msg
            assert (self.hard_maximum >= self.normal), msg
        if self.hard_minimum is not None:
            assert (self.hard_minimum <= self.soft_minimum), msg
            assert (self.hard_minimum <= self.normal), msg
        if None not in (self.hard_maximum, self.hard_minimum):
            assert (self.hard_minimum < self.hard_maximum), msg
        assert self.normal is not None
        assert (self.soft_minimum < self.soft_maximum), msg
        assert (self.soft_minimum <= self.normal), msg
        assert (self.soft_maximum >= self.normal), msg


def _init_globals_from_json(filename):
    """Populate global variables above from the canonical JSON definition."""

    def with_comments(d):
        d.setdefault('tcomment_name', None)
        d.setdefault('tcomment_tooltip', None)
        return d

    flag = "r" if PY3 else "rb"
    with open(filename, flag) as fp:
        defs = json.load(fp)
    for input_def in defs["inputs"]:
        input = _BrushInput(**with_comments(input_def))
        input.validate()
        _INPUTS.append(input)
    for setting_def in defs["settings"]:
        setting = _BrushSetting(**with_comments(setting_def))
        setting.validate()
        _SETTINGS.append(setting)
    for state_name in defs["states"]:
        _STATES.append(state_name)


def writefile(filename, s):
    """Write generated code if changed."""
    bn = basename(sys.argv[0])
    s = '// DO NOT EDIT - autogenerated by ' + bn + '\n\n' + s
    if os.path.exists(filename) and open(filename).read() == s:
        print('Checked {}: up to date, not rewritten'.format(filename))
    else:
        print('Writing {}'.format(filename))
        open(filename, 'w').write(s)


def generate_enum(enum_name, enum_prefix, count_name, items):

    indent = " " * 4
    begin = "typedef enum {\n"
    end = "} %s;\n" % enum_name

    entries = []
    for idx, name in items:
        entries.append(indent + enum_prefix + name)
    entries.append(indent + count_name)

    return begin + ",\n".join(entries) + "\n" + end


def generate_static_struct_array(struct_type, instance_name, entries_list):

    indent = " " * 4
    begin = "static %s %s[] = {\n" % (struct_type, instance_name)
    end = "};\n"

    entries = []
    for entry in entries_list:
        entries.append(indent + "{%s}" % (", ".join(entry)))
    entries.append("\n")

    return begin + ", \n".join(entries) + end


def stringify(value):
    value = value.replace("\n", "\\n")
    value = value.replace('"', '\\"')
    return "\"%s\"" % value


def floatify(value, positive_inf=True):
    if value is None:
        return "FLT_MAX" if positive_inf else "-FLT_MAX"
    return str(value)


def gettextify(value, comment=None):
    result = "N_(%s)" % stringify(value)
    if comment:
        assert isinstance(comment, str) or isinstance(comment, unicode)
        result = "/* %s */ %s" % (comment, result)
    return result


def boolify(value):
    return str("TRUE") if value else str("FALSE")


def tcomment(base_comment, addendum=None):
    comment = base_comment
    if addendum:
        comment = "{c} - {a}".format(c=comment, a=addendum)
    return comment


def tooltip_comment(name, name_type, addendum=None):
    comment = 'Tooltip for the "{n}" brush {t}'.format(
        n=name, t=name_type)
    return tcomment(comment, addendum)


def input_info_struct(i):
    name_comment = tcomment("Brush input", i.tcomment_name)
    _tooltip_comment = tooltip_comment(
        i.displayed_name, "input", i.tcomment_tooltip)
    return (
        stringify(i.id),
        floatify(i.hard_minimum, positive_inf=False),
        floatify(i.soft_minimum, positive_inf=False),
        floatify(i.normal),
        floatify(i.soft_maximum),
        floatify(i.hard_maximum),
        gettextify(i.displayed_name, name_comment),
        gettextify(i.tooltip, _tooltip_comment),
    )


def settings_info_struct(s):
    name_comment = tcomment("Brush setting", s.tcomment_name)
    _tooltip_comment = tooltip_comment(
        s.displayed_name, "setting", s.tcomment_tooltip)
    return (
        stringify(s.internal_name),
        gettextify(s.displayed_name, name_comment),
        boolify(s.constant),
        floatify(s.minimum, positive_inf=False),
        floatify(s.default),
        floatify(s.maximum),
        gettextify(s.tooltip, _tooltip_comment),
    )


def header_guard_name(file_name):
    alfa_num = "".join(map(lambda c: c if c.isalnum() else '_', file_name))
    return alfa_num.upper()


def header_guarded(file_name, header_content):
    guard_name = header_guard_name(file_name)
    guard = '#ifndef {guard}\n#define {guard}\n{content}\n#endif\n'
    return guard.format(guard=guard_name, content=header_content)


def generate_internal_settings_code():
    content = ''
    content += generate_static_struct_array(
        "MyPaintBrushSettingInfo",
        "settings_info_array",
        [settings_info_struct(i) for i in _SETTINGS],
    )
    content += "\n"
    content += generate_static_struct_array(
        "MyPaintBrushInputInfo",
        "inputs_info_array",
        [input_info_struct(i) for i in _INPUTS],
    )
    return content


def generate_public_settings_code():
    content = ''
    content += generate_enum(
        "MyPaintBrushInput",
        "MYPAINT_BRUSH_INPUT_",
        "MYPAINT_BRUSH_INPUTS_COUNT",
        enumerate([i.id.upper() for i in _INPUTS]),
    )
    content += '\n'
    content += generate_enum(
        "MyPaintBrushSetting",
        "MYPAINT_BRUSH_SETTING_",
        "MYPAINT_BRUSH_SETTINGS_COUNT",
        enumerate([i.internal_name.upper() for i in _SETTINGS]),
    )
    content += '\n'
    content += generate_enum(
        "MyPaintBrushState",
        "MYPAINT_BRUSH_STATE_",
        "MYPAINT_BRUSH_STATES_COUNT",
        enumerate([i.upper() for i in _STATES]),
    )
    content += '\n'
    return content


if __name__ == '__main__':
    _init_globals_from_json("brushsettings.json")
    script = sys.argv[0]
    try:
        public_header_file, internal_header_file = sys.argv[1:]
    except Exception:
        msg = "usage: {} PUBLICdotH INTERNALdotH".format(script)
        print(msg, file=sys.stderr)
        sys.exit(2)
    phf = public_header_file
    writefile(phf, header_guarded(phf, generate_public_settings_code()))
    ihf = internal_header_file
    writefile(ihf, header_guarded(ihf, generate_internal_settings_code()))