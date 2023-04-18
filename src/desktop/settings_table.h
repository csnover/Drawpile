#include "libclient/settings_table_macros.h"

SETTING(brushCursor               , BrushCursor               , "settings/brushcursor"                  , BrushCursorEnum::TriangleRight)
SETTING(brushOutlineWidth         , BrushOutlineWidth         , "settings/brushoutlinewidth"            , 1.0)
SETTING(canvasScrollBars          , CanvasScrollBars          , "settings/canvasscrollbars"             , true)
SETTING(canvasShortcuts           , CanvasShortcuts           , "settings/canvasshortcuts2"             , QVariantMap())
SETTING(colorWheelAngle           , ColorWheelAngle           , "settings/colorwheel/rotate"            , color_widgets::ColorWheel::AngleEnum::AngleRotating)
SETTING(colorWheelShape           , ColorWheelShape           , "settings/colorwheel/shape"             , color_widgets::ColorWheel::ShapeEnum::ShapeTriangle)
SETTING(colorWheelSpace           , ColorWheelSpace           , "settings/colorwheel/space"             , color_widgets::ColorWheel::ColorSpaceEnum::ColorHSL)
SETTING(compactChat               , CompactChat               , "history/compactchat"                   , false)
SETTING(confirmLayerDelete        , ConfirmLayerDelete        , "settings/confirmlayerdelete"           , true)
SETTING(contentFilterHideLocked   , ContentFilterHideLocked   , "pc/hidelocked"                         , false)
SETTING(curvesPresets             , CurvesPresets             , "curves/presets"                        , QVector<QVariantMap>())
SETTING(curvesPresetsConverted    , CurvesPresetsConverted    , "curves/inputpresetsconverted"          , false)
SETTING(filterClosed              , FilterClosed              , "history/filterclosed"                  , false)
SETTING(filterLocked              , FilterLocked              , "history/filterlocked"                  , false)
SETTING(filterNsfm                , FilterNsfm                , "history/filternsfw"                    , false)
SETTING(flipbookCrop              , FlipbookCrop              , "flipbook/crop"                         , QRect())
SETTING(flipbookFrameRate         , FlipbookFrameRate         , "flipbook/fps"                          , 15)
SETTING(flipbookWindow            , FlipbookWindow            , "flipbook/window"                       , QRect())
SETTING(globalPressureCurve       , GlobalPressureCurve       , "settings/input/globalcurve"            , QString("0,0;1,1;"))
SETTING(ignoreCarrierGradeNat     , IgnoreCarrierGradeNat     , "history/cgnalert"                      , false)
SETTING(inputPresets              , InputPresets              , "inputpresets"                          , QVector<QVariantMap>())
SETTING(insecurePasswordStorage   , InsecurePasswordStorage   , "settings/insecurepasswordstorage"      , false)
SETTING(joinListSortColumn        , JoinListSortColumn        , "history/listsortcol"                   , 1) // TODO: Use an enum
SETTING(language                  , Language                  , "settings/language"                     , QString())
SETTING(lastAnnounce              , LastAnnounce              , "history/announce"                      , false)
SETTING(lastAvatar                , LastAvatar                , "history/avatar"                        , QString())
SETTING(lastFileOpenPath          , LastFileOpenPath          , "window/lastpath"                       , QString())
SETTING(lastFileOpenPaths         , LastFileOpenPaths         , "window/lastpaths"                      , QVariantMap())
SETTING(lastHostRemote            , LastHostRemote            , "history/hostremote"                    , false)
SETTING(lastIdAlias               , LastIdAlias               , "history/idalias"                       , QString())
SETTING(lastJoinDialogSize        , LastJoinDialogSize        , "history/joindlgsize"                   , QSize())
SETTING(lastListingServer         , LastListingServer         , "history/listingserver"                 , -1)
SETTING(lastPalette               , LastPalette               , "history/lastpalette"                   , 0)
SETTING(lastSessionTitle          , LastSessionTitle          , "history/sessiontitle"                  , QString())
SETTING(lastTool                  , LastTool                  , "tools/tool"                            , tools::Tool::Type::FREEHAND)
SETTING(lastToolColor             , LastToolColor             , "tools/color"                           , QColor(Qt::black))
SETTING(lastUsername              , LastUsername              , "history/username"                      , QString())
SETTING(lastWindowActions         , LastWindowActions         , "window/actions"                        , (QMap<QString, bool>()))
SETTING(lastWindowDocks           , LastWindowDocks           , "window/docks"                          , (QVariantMap()))
SETTING(lastWindowMaximized       , LastWindowMaximized       , "window/maximized"                      , false)
SETTING(lastWindowPosition        , LastWindowPosition        , "window/pos"                            , (QPoint()))
SETTING(lastWindowSize            , LastWindowSize            , "window/size"                           , (QSize(800, 600)))
SETTING(lastWindowState           , LastWindowState           , "window/state"                          , QByteArray())
SETTING(lastWindowViewState       , LastWindowViewState       , "window/viewstate"                      , QByteArray())
SETTING(layouts                   , Layouts                   , "layouts"                               , QVector<QVariantMap>())
SETTING(maxRecentFiles            , MaxRecentFiles            , "history/maxrecentfiles"                , 6)
SETTING(navigatorRealtime         , NavigatorRealtime         , "navigator/realtime"                    , false)
SETTING(navigatorShowCursors      , NavigatorShowCursors      , "navigator/showcursors"                 , true)
SETTING_GETSET(newCanvasBackColor , NewCanvasBackColor        , "history/newcolor"                      , (QColor(Qt::white)),
	&newCanvasBackColor::get, &any::set)
SETTING_GETSET(newCanvasSize      , NewCanvasSize             , "history/newsize"                       , (QSize(800, 600)),
	&newCanvasSize::get, &any::set)
SETTING(notificationChat          , NotificationChat          , "notifications/chat"                    , true)
SETTING(notificationLock          , NotificationLock          , "notifications/lock"                    , true)
SETTING(notificationLogin         , NotificationLogin         , "notifications/login"                   , true)
SETTING(notificationLogout        , NotificationLogout        , "notifications/logout"                  , true)
SETTING(notificationMarker        , NotificationMarker        , "notifications/marker"                  , true)
SETTING(notificationUnlock        , NotificationUnlock        , "notifications/unlock"                  , true)
SETTING(oneFingerDraw             , OneFingerDraw             , "settings/input/touchdraw"              , false) // TODO: Has view dependencies
SETTING(oneFingerScroll           , OneFingerScroll           , "settings/input/touchscroll"            , true) // TODO: Has view dependencies
SETTING(onionSkinsFrameCount      , OnionSkinsFrameCount      , "onionskins/framecount"                 , 8)
SETTING_GETSET_V(
	V1, onionSkinsFrames          , OnionSkinsFrames          , "onionskins/frames"                     , (QMap<int, int>()),
	&onionSkinsFrames::get, &onionSkinsFrames::set)
SETTING(onionSkinsTintAbove       , OnionSkinsTintAbove       , "onionskins/tintabove"                  , (QColor::fromRgb(0x33, 0x33, 0xff)))
SETTING(onionSkinsTintBelow       , OnionSkinsTintBelow       , "onionskins/tintbelow"                  , (QColor::fromRgb(0xff, 0x33, 0x33)))
SETTING(recentFiles               , RecentFiles               , "history/recentfiles"                   , QStringList())
SETTING(recentHosts               , RecentHosts               , "history/recenthosts"                   , QStringList())
SETTING(recentRemoteHosts         , RecentRemoteHosts         , "history/recentremotehosts"             , QStringList({ "pub.drawpile.net" }))
SETTING(recordMinimumPause        , RecordMinimumPause        , "settings/recording/minimumpause"       , 0.5)
SETTING(recordPause               , RecordPause               , "settings/recording/recordpause"        , true)
SETTING(recordTimestamp           , RecordTimestamp           , "settings/recording/recordtimestamp"    , false)
SETTING(recordTimestampInterval   , RecordTimestampInterval   , "settings/recording/timestampinterval"  , 15)
SETTING(serverHideIp              , ServerHideIp              , "settings/hideServerIp"                 , false)
SETTING(shareBrushSlotColor       , ShareBrushSlotColor       , "settings/sharebrushslotcolor"          , false)
SETTING(shortcuts                 , Shortcuts                 , "settings/shortcuts"                    , QVariantMap())
SETTING(showTrayIcon              , ShowTrayIcon              , "ui/trayicon"                           , true)
SETTING(soundVolume               , SoundVolume               , "notifications/volume"                  , 40)
SETTING(tabletEraser              , TabletEraser              , "settings/input/tableteraser"           , true)
SETTING(tabletEvents              , TabletEvents              , "settings/input/tabletevents"           , true)
SETTING(tabletRelativePenMode     , TabletRelativePenMode     , "settings/input/relativepenhack"        , false) // TODO: Has view dependencies
SETTING(tabletWindowsInk          , TabletWindowsInk          , "settings/input/windowsink"             , false) // TODO: Has view dependencies
SETTING_GETSET(themePalette       , ThemePalette              , "settings/theme/palette"                , ThemePalette::System
	, &themePalette::get, &themePalette::set)
SETTING_FULL(V0, themeStyle       , ThemeStyle                , "settings/theme/style"                  , QString()
	, &themeStyle::get, &any::set, &themeStyle::notify)
SETTING(toolToggle                , ToolToggle                , "settings/tooltoggle"                   , true)
SETTING(toolset                   , Toolset                   , "tools/toolset"                         , (QMap<QString, QVariantHash>()))
SETTING(twoFingerRotate           , TwoFingerRotate           , "settings/input/touchtwist"             , true)
SETTING(twoFingerZoom             , TwoFingerZoom             , "settings/input/touchpinch"             , true)
SETTING(videoExportCustomFfmpeg   , VideoExportCustomFfmpeg   , "videoexport/customffmpeg"              , QString())
SETTING(videoExportFormat         , VideoExportFormat         , "videoexport/formatchoice"              , VideoExporter::Format::IMAGE_SERIES)
SETTING(videoExportFrameHeight    , VideoExportFrameHeight    , "videoexport/frameheight"               , 720)
SETTING(videoExportFrameRate      , VideoExportFrameRate      , "videoexport/fps"                       , 30)
SETTING(videoExportFrameWidth     , VideoExportFrameWidth     , "videoexport/framewidth"                , 1280)
SETTING(videoExportSizeChoice     , VideoExportSizeChoice     , "videoexport/sizeChoice"                , 0) // TODO: Enum
SETTING(writeLogFile              , WriteLogFile              , "settings/logfile"                      , false)

#include "libclient/settings_table_macros.h"
