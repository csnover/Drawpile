// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/settingsmodel.h"
#include "desktop/settingsmodel_meta.cpp"

#include <optional>
#include <utility>

Settings::Settings(QObject *parent)
	: QAbstractItemModel(parent)
{
#ifdef Q_OS_WIN
	// For whatever reason Qt does not provide a QSettings constructor that only
	// takes the format, so it is necessary to do this global state swap
	const auto oldFormat = QSettings::defaultFormat();
	QSettings::setDefaultFormat(QSettings::IniFormat);
	m_settings = QSettings();
	QSettings::setDefaultFormat(oldFormat);
#endif
	// We never use or want fallbacks to any system settings, especially on
	// macOS where a bunch of extra stuff from the OS is exposed in fallback
	m_settings.setFallbacksEnabled(false);
}

struct FoundKey {
	Settings::MetaVersion version;
	QString key;
};

static std::optional<FoundKey> findKey(const QSettings &settings, const SettingsMeta &meta, bool ignoreLatest = false)
{
	auto candidate = int(meta.version);
	if (ignoreLatest) {
		if (candidate == 0) {
			return {};
		} else {
			--candidate;
		}
	}

	for (; candidate > 0; --candidate) {
		if (ignoreLatest && meta.version == Settings::MetaVersion::Max) {
			continue;
		}

		const auto versionedKey = formatSettingKey(candidate, meta.baseKey);
		if (settings.contains(versionedKey)) {
			return {{ Settings::MetaVersion(candidate), versionedKey }};
		}
	}

	if (settings.contains(meta.baseKey)) {
		return {{ Settings::MetaVersion::V0, meta.baseKey }};
	}

	return {};
}

QVariant Settings::data(const QModelIndex &index, int role) const
{
	if (!isValidIndex(index) || role != ValueRole) {
		return QVariant();
	}

	const auto row = Row(index.row());

	switch (row) {
	case TabletDriver:
	case OneFingerTouch:
		Q_ASSERT_X(false, "", "TODO");
		break;
	case NewCanvasSize:
		return dataFromStorage(row)
			.toSize()
			.expandedTo({ 100, 100 })
			.boundedTo({ 65535, 65535 });
	case NewCanvasBackColor: {
		auto color = dataFromStorage(row).value<QColor>();
		if (color.isValid()) {
			return color;
		} else {
			qWarning() << "invalid NewCanvasBackColor";
			return QColor(Qt::white);
		}
	}
	default:
		return dataFromStorage(row);
	}

	return QVariant();
}

QVariant Settings::dataFromStorage(Row row) const
{
	const auto *meta = SettingsMeta::fromRow(row);
	if (!meta) {
		return QVariant();
	}

	if (m_pending.contains(row)) {
		return m_pending[row];
	}

	if (const auto found = findKey(m_settings, *meta)) {
		auto data = m_settings.value(found->key);
		if (found->version == meta->version) {
			return data;
		} else {
			return upgradeData(row, found->version, std::move(data));
		}
	} else if (meta->defaultValue.isValid()) {
		return meta->defaultValue;
	}

	return QVariant();
}

QModelIndex Settings::index(int row, int column, const QModelIndex &parent) const
{
	if (isValidIndex(row, column, parent)) {
		return createIndex(row, column);
	}

	return QModelIndex();
}

bool Settings::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!isValidIndex(index) || role != ValueRole) {
		return false;
	}

	const auto row = Row(index.row());

	switch (row) {
	case TabletDriver:
		Q_ASSERT_X(false, "", "TODO");
		break;
	case OneFingerTouch:
		Q_ASSERT_X(false, "", "TODO");
		break;
	default: {
		const auto *meta = SettingsMeta::fromRow(row);
		if (!meta) {
			return false;
		}

		// TODO: Constrain value type to match that of defaultValue if it
		// exists, or emit an error if it does not match?
		if (!m_pending.contains(row) || m_pending[row] != value) {
			m_pending[row] = value;
			emit dataChanged(index, index, { Qt::DisplayRole, ValueRole });
		}
		return true;
	}
	}

	return false;
}

void Settings::revert()
{
	const auto rows = m_pending.keys();

	// Changes must be cleared before emitting events or else they will be
	// used when a listener queries for updated values
	m_pending.clear();

	for (const auto row : rows) {
		const auto index = createIndex(row, 0);
		emit dataChanged(index, index, { Qt::DisplayRole, ValueRole });

		const auto *meta = SettingsMeta::fromRow(row);
		Q_ASSERT(meta);
		for (const auto metaRow : meta->views) {
			const auto viewIndex = createIndex(metaRow, 0);
			emit dataChanged(viewIndex, viewIndex, { Qt::DisplayRole, ValueRole });
		}
	}
}

bool Settings::submit()
{
	for (auto entry = m_pending.cbegin(); entry != m_pending.cend(); ++entry) {
		const auto row = entry.key();
		const auto &value = entry.value();

		const auto *meta = SettingsMeta::fromRow(row);
		Q_ASSERT(meta);

		// If the settings were used with an earlier version, retain the
		// ability to go back to the earlier version. This supports only
		// the n-1 version of a setting, but that is probably all anyone
		// needs.
		if (const auto old = findKey(m_settings, *meta, true)) {
			if (value.isValid()) {
				m_settings.setValue(
					old->key,
					downgradeData(row, old->version, value)
				);
			} else {
				m_settings.remove(old->key);
			}
		}

		if (value.isValid()) {
			m_settings.setValue(meta->key(), value);
		} else {
			m_settings.remove(meta->key());
		}
	}

	m_settings.sync();
	m_pending.clear();
	return m_settings.status() == QSettings::NoError;
}

QVariant Settings::downgradeData(Row row, MetaVersion version, const QVariant &value) const
{
	if (row == Theme && version == MetaVersion::V0) {
		switch (value.value<ThemeEnum>()) {
		case ThemeEnum::System:
		case ThemeEnum::SystemLight:
		case ThemeEnum::HotdogStand:
			return 0;
		case ThemeEnum::FusionLight:
		case ThemeEnum::KritaBright:
			return 1;
		case ThemeEnum::SystemDark:
		case ThemeEnum::FusionDark:
		case ThemeEnum::KritaDark:
		case ThemeEnum::KritaDarker:
			return 2;
		case ThemeEnum::Count:
			Q_UNREACHABLE();
		}
	}

	return value;
}

QVariant Settings::upgradeData(Row row, MetaVersion version, QVariant &&value) const
{
	if (row == Theme && version == MetaVersion::V0) {
		switch (value.toInt()) {
		case 1: return QVariant::fromValue(ThemeEnum::FusionLight);
		case 2: return QVariant::fromValue(ThemeEnum::FusionDark);
		default: return QVariant::fromValue(ThemeEnum::System);
		}
	}

	return std::move(value);
}
