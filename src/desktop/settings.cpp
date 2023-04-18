// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/settings.h"

namespace desktop {
namespace settings {

using libclient::settings::SettingMeta;
using libclient::settings::findKey;
namespace any = libclient::settings::any;

namespace onionSkinsFrames {
	QVariant get(const SettingMeta &meta, const QSettings &settings)
	{
		if (const auto key = findKey(settings, meta.baseKey, meta.version)) {
			return any::getGroup(settings, key->key);
		} else if (findKey(settings, "onionskins/frame1", SettingMeta::Version::V0)) {
			Settings::OnionSkinsFramesType frames;
			const auto size = settings.value("onionskins/framecount").toInt();
			for (auto i = 0; i < size; ++i) {
				frames.insert(i, settings.value(QStringLiteral("frame%1").arg(i)).toInt());
			}
			return QVariant::fromValue(frames);
		} else {
			return meta.defaultValue;
		}
	}

	void set(const SettingMeta &meta, QSettings &settings, QVariant value)
	{
		any::set(meta, settings, value);

		if (findKey(settings, "onionskins/frame1", SettingMeta::Version::V0)) {
			const auto frames = value.value<Settings::OnionSkinsFramesType>();
			for (auto entry = frames.cbegin(); entry != frames.cend(); ++entry) {
				settings.setValue(QStringLiteral("frame%1").arg(entry.key()), entry.value());
			}
		}
	}
}

namespace newCanvasSize {
	QVariant get(const SettingMeta &meta, const QSettings &settings)
	{
		return any::get(meta, settings)
			.toSize()
			.expandedTo({ 100, 100 })
			.boundedTo({ 65535, 65535 });
	}
}

namespace newCanvasBackColor {
	QVariant get(const SettingMeta &meta, const QSettings &settings)
	{
		const auto color = any::get(meta, settings).value<QColor>();
		if (!color.isValid()) {
			return meta.defaultValue;
		}
		return color;
	}
}

namespace themePalette {
	QVariant get(const SettingMeta &meta, const QSettings &settings)
	{
		if (findKey(settings, meta.baseKey, meta.version)) {
			return any::get(meta, settings);
		} else if (const auto key = findKey(settings, "settings/theme", SettingMeta::Version::V1)) {
			const auto value = settings.value(key->key).toInt();
			if (value > 0 && value <= 5) {
				return "Fusion";
			} else {
				return "";
			}
		} else {
			return meta.defaultValue;
		}
	}

	void set(const SettingMeta &meta, QSettings &settings, QVariant value)
	{
		any::set(meta, settings, value);

		if (const auto oldKey = findKey(settings, "settings/theme", SettingMeta::Version::V1)) {
			const auto palette = value.value<ThemePalette>();
			int oldValue = 0;
			switch (oldKey->version) {
			case SettingMeta::Version::V1:
				switch (palette) {
				case ThemePalette::Light: oldValue = 1; break;
				case ThemePalette::Dark: oldValue = 2; break;
				case ThemePalette::KritaBright: oldValue = 3; break;
				case ThemePalette::KritaDark: oldValue = 4; break;
				case ThemePalette::KritaDarker: oldValue = 5; break;
				default: {}
				}
				break;
			case SettingMeta::Version::V0:
				switch (palette) {
				case ThemePalette::KritaBright:
				case ThemePalette::Light:
					oldValue = 1;
					break;
				case ThemePalette::KritaDark:
				case ThemePalette::KritaDarker:
				case ThemePalette::Dark:
					oldValue = 2;
					break;
				default: {}
				}
			}
			settings.setValue(oldKey->key, oldValue);
		}
	}
}

namespace themeStyle {
	QVariant get(const SettingMeta &meta, const QSettings &settings)
	{
		if (findKey(settings, meta.baseKey, meta.version)) {
			return any::get(meta, settings);
		} else if (const auto key = findKey(settings, "settings/theme", SettingMeta::Version::V1)) {
			const auto value = settings.value(key->key).toInt();
			if (value > 0 && value <= 5) {
				return "Fusion";
			} else {
				return "";
			}
		} else {
			return meta.defaultValue;
		}
	}

	// Changing the theme style can cause the theme palette to change too
	void notify(const SettingMeta &, libclient::settings::Settings &base) {
		auto &settings = static_cast<Settings &>(base);
		settings.themeStyleChanged(settings.themeStyle());
		settings.themePaletteChanged(settings.themePalette());
	}
}

#define DP_SETTINGS_BODY
#include "desktop/settings_table.h"
#undef DP_SETTINGS_BODY

} // namespace settings
} // namespace desktop
