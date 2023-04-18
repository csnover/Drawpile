// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmake-config/config.h"
#include "desktop/utils/recentfiles.h"
#include "libclient/canvas/paintengine.h"
#include "libclient/parentalcontrols/parentalcontrols.h"

#include <QColor>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QtColorWidgets/ColorWheel>
#include <dpmsg/messages.h>
#include <initializer_list>

/// @brief Creates a versioned serialisation key for the given version and key.
/// @param version The version of the setting as an integer.
/// @param baseKey The base key.
/// @return The versioned key.
static QString formatSettingKey(int version, const char *baseKey)
{
	if (version == 0) {
		return baseKey;
	}

	return QStringLiteral("v%1/%2").arg(version).arg(baseKey);
}

namespace meta {

using AngleEnum = color_widgets::ColorWheel::AngleEnum;
using ColorSpaceEnum = color_widgets::ColorWheel::ColorSpaceEnum;
using PaintEngine = canvas::PaintEngine;
using ShapeEnum = color_widgets::ColorWheel::ShapeEnum;
using S = Settings;
using V = S::MetaVersion;

/// @brief Metadata about a setting that persists to storage.
struct PersistentSetting {
	/// @brief Gets metadata about a persistent setting for the given row.
	/// @param row The setting to look up.
	/// @return Persistence metadata for the setting, if it exists, or nullptr
	/// if the setting row is static, a synthesised view of another setting, or
	/// non-persistent.
	static inline const PersistentSetting *fromRow(S::Row row)
	{
		using Index = std::array<const PersistentSetting *, S::RowCount>;
		static const Index index = [&]() {
			Index index = {};
			for (const auto &meta : g_metadata) {
				index[meta.row] = &meta;
			}
			return index;
		}();

		return index[row];
	}

	/// @brief The current serialisation version for this setting.
	V version;
	/// @brief The key used when serialising this setting to QSettings.
	const char *baseKey;
	/// @brief The settings model row that corresponds to this setting.
	S::Row row;
	/// @brief The default value to use when no persisted value exists.
	QVariant defaultValue;
	/// @brief A list of model rows that are synthesised from this setting’s
	/// data, if any.
	std::initializer_list<S::Row> views;

	/// @return The correct key to use when serialising the latest version of
	/// this setting.
	QString key() const { return formatSettingKey(int(version), baseKey); }

private:
	using Table = const std::initializer_list<PersistentSetting>;
	static Table g_metadata;
};

// On most platforms, tablet input comes at a very high precision and frequency,
// so some smoothing is sensible by default. On Android (at least on a Samsung
// Galaxy S6 Lite, a Samsung Galaxy S8 Ultra and reports from unknown devices)
// the input is already pretty smooth though, so we'll leave it at zero there.
constexpr int smoothing()
{
#ifdef Q_OS_ANDROID
	return 0;
#else
	return 3;
#endif
}

inline const QVariant NoDefault = QVariant();
inline constexpr std::initializer_list<S::Row> NoViews = {};

/// @brief All persistent settings.
PersistentSetting::Table PersistentSetting::g_metadata = {
// Setting | QSettings key                          | Data model row key          | Default   | Dependent
// version |                                        |                             | value     | views
	{ V::V1, "settings/theme"                       , S::Theme,
		QVariant::fromValue(S::ThemeEnum::Default)      , { S::ThemeStyle, S::ThemePalette } },
	{ V::V0, "settings/language"                    , S::Language                 , NoDefault , NoViews },
	{ V::V0, "settings/logfile"                     , S::LogFile                  , false     , NoViews },
	{ V::V0, "settings/autosave"                    , S::AutoSaveInterval         , 5000      , NoViews },
	{ V::V0, "settings/confirmlayerdelete"          , S::ConfirmLayerDelete       , true      , NoViews },
	{ V::V0, "settings/canvasscrollbars"            , S::CanvasScrollBars         , true      , NoViews },
	{ V::V0, "settings/input/tabletevents"          , S::TabletEvents             , true      , NoViews },
	{ V::V0, "settings/input/tableteraser"          , S::TabletEraser             , true      , NoViews },
	{ V::V0, "settings/input/windowsink"            , S::TabletWindowsInk         , false     ,
		{ S::TabletDriver } },
	{ V::V0, "settings/input/relativepenhack"       , S::TabletRelativePenMode    , false     ,
		{ S::TabletDriver } },
	{ V::V0, "settings/input/smooth"                , S::Smoothing,
		smoothing()                                     , NoViews },
	{ V::V0, "settings/input/globalcurve"           , S::GlobalPressureCurve      , NoDefault , NoViews },
	{ V::V0, "curves/presets"                       , S::CurvesPresets            , NoDefault , NoViews },
	{ V::V0, "curves/inputpresetsconverted"         , S::CurvesPresetsConverted   , NoDefault , NoViews },
	{ V::V0, "settings/input/touchdraw"             , S::OneFingerDraw            , false     ,
		{ S::OneFingerTouch } },
	{ V::V0, "settings/input/touchscroll"           , S::OneFingerScroll          , true      ,
		{ S::OneFingerTouch } },
	{ V::V0, "settings/input/touchpinch"            , S::TwoFingerZoom            , true      , NoViews },
	{ V::V0, "settings/input/touchtwist"            , S::TwoFingerRotate          , true      , NoViews },
	{ V::V0, "settings/tooltoggle"                  , S::ToolToggle               , true      , NoViews },
	{ V::V0, "settings/sharebrushslotcolor"         , S::ShareBrushSlotColor      , false     , NoViews },
	{ V::V0, "settings/brushcursor"                 , S::BrushCursor,
		QVariant::fromValue(S::BrushCursorEnum::TriangleRight), NoViews },
	{ V::V0, "settings/brushoutlinewidth"           , S::BrushOutlineWidth        , 1.0       , NoViews },
	{ V::V0, "settings/colorwheel/shape"            , S::ColorWheelShape          ,
		ShapeEnum::ShapeTriangle                        , NoViews },
	{ V::V0, "settings/colorwheel/rotate"           , S::ColorWheelAngle          ,
		AngleEnum::AngleRotating                        , NoViews },
	{ V::V0, "settings/colorwheel/space"            , S::ColorWheelSpace          ,
		ColorSpaceEnum::ColorHSL                        , NoViews },
	{ V::V0, "settings/shortcuts"                   , S::Shortcuts                , NoDefault , NoViews },
	{ V::V0, "settings/insecurepasswordstorage"     , S::InsecurePasswordStorage  , false     , NoViews },
	{ V::V0, "settings/recording/recordpause"       , S::RecordPause              , true      , NoViews },
	{ V::V0, "settings/recording/minimumpause"      , S::RecordMinimumPause       , 0.5       , NoViews },
	{ V::V0, "settings/recording/recordtimestamp"   , S::RecordTimestamp          , false     , NoViews },
	{ V::V0, "settings/recording/timestampinterval" , S::RecordTimestampInterval  , 15        , NoViews },
	{ V::V0, "settings/server/port"                 , S::ServerPort,
		cmake_config::proto::port()                     , NoViews },
	{ V::V0, "settings/server/autoreset"            , S::ServerAutoReset          , true      , NoViews },
	{ V::V0, "settings/server/timeout"              , S::ServerTimeout            , 60        , NoViews },
	{ V::V0, "settings/server/privateUserList"      , S::ServerPrivateUserList    , NoDefault , NoViews },
	{ V::V0, "settings/hideServerIp"                , S::ServerHideIp             , false     , NoViews },
	{ V::V0, "listservers"                          , S::ListServers              , NoDefault , NoViews },
	{ V::V0, "history/newsize"                      , S::NewCanvasSize            ,
		QSize(800, 600)                                 , NoViews },
	{ V::V0, "history/newcolor"                     , S::NewCanvasBackColor       ,
		QColor(Qt::white)                               , NoViews },
	{ V::V0, "history/compactchat"                  , S::CompactChat              , false     , NoViews },
	//V:: TODO: Enum?
	{ V::V0, "history/lastpalette"                  , S::LastPalette              , 0         , NoViews },
	{ V::V0, "history/sessiontitle"                 , S::LastSessionTitle         , NoDefault , NoViews },
	{ V::V0, "history/idalias"                      , S::LastIdAlias              , NoDefault , NoViews },
	{ V::V0, "history/announce"                     , S::LastAnnounce             , false     , NoViews },
	{ V::V0, "history/listingserver"                , S::LastListingServer        , NoDefault , NoViews },
	{ V::V0, "history/hostremote"                   , S::LastHostRemote           , NoDefault , NoViews },
	{ V::V0, "history/joindlgsize"                  , S::LastJoinDialogSize       , NoDefault , NoViews },
	{ V::V0, "history/username"                     , S::LastUsername             , NoDefault , NoViews },
	{ V::V0, "history/avatar"                       , S::LastAvatar               , NoDefault , NoViews },
	{ V::V0, "history/maxrecentfiles"               , S::MaxRecentFiles           ,
		RecentFiles::DEFAULT_MAXFILES                   , NoViews },
	{ V::V0, "history/recentfiles"                  , S::RecentFiles              , NoDefault , NoViews },
	{ V::V0, "history/recenthosts"                  , S::RecentHosts              , NoDefault , NoViews },
	{ V::V0, "history/recentremotehosts"            , S::RecentRemoteHosts        , NoDefault , NoViews },
	{ V::V0, "history/filterlocked"                 , S::FilterLocked             , false     , NoViews },
	{ V::V0, "history/filternsfw"                   , S::FilterNsfm               , false     , NoViews },
	{ V::V0, "history/filterclosed"                 , S::FilterClosed             , false     , NoViews },
	// TODO: Use an enum
	{ V::V0, "history/listsortcol"                  , S::JoinListSortColumn       , 1         , NoViews },
	{ V::V0, "window/size"                          , S::LastWindowSize           ,
		QSize(800, 600)                                 , NoViews },
	{ V::V0, "window/pos"                           , S::LastWindowPosition       , NoDefault , NoViews },
	{ V::V0, "window/maximized"                     , S::LastWindowMaximized      , NoDefault , NoViews },
	{ V::V0, "window/state"                         , S::LastWindowState          , NoDefault , NoViews },
	{ V::V0, "window/viewstate"                     , S::LastWindowViewState      , NoDefault , NoViews },
	{ V::V0, "window/docks"                         , S::LastWindowDocks          , NoDefault , NoViews },
	{ V::V0, "window/actions"                       , S::LastWindowActions        , NoDefault , NoViews },
	{ V::V0, "window/lastpath"                      , S::LastFileOpenPath         , NoDefault , NoViews },
	{ V::V0, "notifications/volume"                 , S::SoundVolume              , 40        , NoViews },
	{ V::V0, "notifications/chat"                   , S::NotificationChat         , true      , NoViews },
	{ V::V0, "notifications/marker"                 , S::NotificationMarker       , true      , NoViews },
	{ V::V0, "notifications/login"                  , S::NotificationLogin        , true      , NoViews },
	{ V::V0, "notifications/logout"                 , S::NotificationLogout       , true      , NoViews },
	{ V::V0, "notifications/lock"                   , S::NotificationLock         , true      , NoViews },
	{ V::V0, "notifications/unlock"                 , S::NotificationUnlock       , true      , NoViews },
	{ V::V0, "flipbook/fps"                         , S::FlipbookFrameRate        , 15        , NoViews },
	{ V::V0, "flipbook/window"                      , S::FlipbookWindow           , NoDefault , NoViews },
	{ V::V0, "flipbook/crop"                        , S::FlipbookCrop             , NoDefault , NoViews },
	{ V::V0, "layouts"                              , S::Layouts                  , NoDefault , NoViews },
	{ V::V0, "pc/locked"                            , S::FeatureControlLocked     , false     , NoViews },
	// TODO: Enum
	{ V::V0, "pc/level"                             , S::FeatureControlLevel      , 0         , NoViews },
	{ V::V0, "pc/tagwords"                          , S::FeatureControlTags       ,
		parentalcontrols::defaultWordList()             , NoViews },
	{ V::V0, "pc/autotag"                           , S::FeatureControlAutoTag    , true      , NoViews },
	{ V::V0, "pc/noUncensoring"                     , S::FeatureControlForceCensor, false     , NoViews },
	{ V::V0, "pc/hidelocked"                        , S::FeatureControlHideLocked , false     , NoViews },
	{ V::V0, "settings/paintengine/fps"             , S::EngineFrameRate,
		PaintEngine::DEFAULT_FPS                        , NoViews },
	{ V::V0, "settings/paintengine/snapshotcount"   , S::EngineSnapshotCount      ,
		PaintEngine::DEFAULT_SNAPSHOT_MAX_COUNT         , NoViews },
	{ V::V0, "settings/paintengine/snapshotinterval", S::EngineSnapshotInterval   ,
		PaintEngine::DEFAULT_SNAPSHOT_MIN_DELAY_MS      , NoViews },
	{ V::V0, "settings/paintengine/undodepthlimit"  , S::EngineUndoDepth          ,
		DP_UNDO_DEPTH_DEFAULT                           , NoViews },
	{ V::V0, "settings/canvasshortcuts2"            , S::CanvasShortcuts          , NoDefault , NoViews },
	{ V::V0, "versioncheck/enabled"                 , S::VersionCheckEnabled      , NoDefault , NoViews },
	{ V::V0, "versioncheck/lastcheck"               , S::VersionCheckLastCheck    , NoDefault , NoViews },
	{ V::V0, "versioncheck/lastsuccess"             , S::VersionCheckLastSuccess  , NoDefault , NoViews },
	{ V::V0, "versioncheck/latest"                  , S::VersionCheckLatest       , NoDefault , NoViews },
	// TODO: Enum
	{ V::V0, "videoexport/formatchoice"             , S::VideoExportFormat        , 0         , NoViews },
	{ V::V0, "videoexport/fps"                      , S::VideoExportFrameRate     , 30        , NoViews },
	{ V::V0, "videoexport/framewidth"               , S::VideoExportFrameWidth    , 1280      , NoViews },
	{ V::V0, "videoexport/frameheight"              , S::VideoExportFrameHeight   , 720       , NoViews },
	// TODO: Enum
	{ V::V0, "videoexport/sizeChoice"               , S::VideoExportSizeChoice    , 0         , NoViews },
	{ V::V0, "videoexport/customffmpeg"             , S::VideoExportCustomFfmpeg  , NoDefault , NoViews },
	{ V::V0, "navigator/showcursors"                , S::NavigatorShowCursors     , true      , NoViews },
	{ V::V0, "navigator/realtime"                   , S::NavigatorRealtime        , false     , NoViews },
	// TODO: Delete from private onionskins namespace
	{ V::V0, "onionskins/framecount"                , S::OnionSkinsFrameCount     , 8         , NoViews },
	{ V::V0, "onionskins/tintbelow"                 , S::OnionSkinsTintBelow      ,
		QColor::fromRgb(0xff, 0x33, 0x33)               , NoViews },
	{ V::V0, "onionskins/tintabove"                 , S::OnionSkinsTintAbove      ,
		QColor::fromRgb(0x33, 0x33, 0xff)               , NoViews },
	{ V::V0, "tools/color"                          , S::LastToolColor            , NoDefault , NoViews },
	{ V::V0, "tools/tool"                           , S::LastTool                 , NoDefault , NoViews },
	{ V::V0, "tools/toolset"                        , S::Toolset                  , NoDefault , NoViews },
	{ V::V0, "ui/trayicon"                          , S::ShowTrayIcon             , true      , NoViews },
	{ V::V0, "guiserver/port"                       , S::GuiServerPort            ,
		cmake_config::proto::port()                     , NoViews },
	{ V::V0, "guiserver/use-ssl"                    , S::ServerUseSsl             , false     , NoViews },
	{ V::V0, "guiserver/sslcert"                    , S::ServerSslCert            , NoDefault , NoViews },
	{ V::V0, "guiserver/sslkey"                     , S::ServerSslKey             , NoDefault , NoViews },
	{ V::V0, "guiserver/local-address"              , S::ServerLocalAddress       , NoDefault , NoViews },
	{ V::V0, "guiserver/extauth"                    , S::ServerExtAuth            , false     , NoViews },
	{ V::V0, "guiserver/session-storage"            , S::ServerSessionStorage     , "file"    , NoViews }
};

} // namespace meta

using SettingsMeta = meta::PersistentSetting;
