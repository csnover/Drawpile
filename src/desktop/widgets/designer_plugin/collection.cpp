// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "collection.h"
#include "colorbutton_plugin.h"
#include "brushpreview_plugin.h"
#include "groupedtoolbutton_plugin.h"
#include "filmstrip_plugin.h"
#include "resizer_plugin.h"
#include "tablettester_plugin.h"
#include "spinner_plugin.h"
#include "presetselector_plugin.h"
#include "modifierkeys_plugin.h"

DrawpileWidgetCollection::DrawpileWidgetCollection(QObject *parent) :
	QObject(parent)
{
	widgets
		<< new ColorButtonPlugin(this)
		<< new BrushPreviewPlugin(this)
		<< new GroupedToolButtonPlugin(this)
		<< new FilmstripPlugin(this)
		<< new ResizerPlugin(this)
		<< new TabletTesterPlugin(this)
		<< new SpinnerPlugin(this)
		<< new PresetSelectorPlugin(this)
		<< new ModifierKeysPlugin(this)
		;
}

QList<QDesignerCustomWidgetInterface *> DrawpileWidgetCollection::customWidgets() const
{
	return widgets;
}
