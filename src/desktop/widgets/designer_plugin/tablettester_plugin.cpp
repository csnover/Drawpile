// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include <QtPlugin>

#include "../tablettest.h"
#include "tablettester_plugin.h"

TabletTesterPlugin::TabletTesterPlugin(QObject *parent)
	: QObject(parent)
{
	initialized = false;
}

void TabletTesterPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if (initialized)
		return;

	initialized = true;
}

bool TabletTesterPlugin::isInitialized() const
{
	return initialized;
}

QWidget *TabletTesterPlugin::createWidget(QWidget *parent)
{
	return new widgets::TabletTester(parent);
}

QString TabletTesterPlugin::name() const
{
	return "widgets::TabletTester";
}

QString TabletTesterPlugin::group() const
{
	return "Drawpile Widgets";
}

QIcon TabletTesterPlugin::icon() const
{
	return QIcon();
}

QString TabletTesterPlugin::toolTip() const
{
	return "A widget for testing tablet stylus events";
}

QString TabletTesterPlugin::whatsThis() const
{
	return "";
}

bool TabletTesterPlugin::isContainer() const
{
	return false;
}

QString TabletTesterPlugin::domXml() const
{
	return "<ui language=\"c++\" displayname=\"TabletTester\">\n"
		"<widget class=\"widgets::TabletTester\" name=\"resizer\">\n"
		" <property name=\"geometry\">\n"
		"  <rect>\n"
		"   <x>0</x>\n"
		"   <y>0</y>\n"
		"   <width>300</width>\n"
		"   <height>300</height>\n"
		"  </rect>\n"
		" </property>\n"
		"</widget>\n"
		"</ui>";
}

QString TabletTesterPlugin::includeFile() const
{
	return "widgets/tablettest.h";
}
