// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_SETTINGSMODEL_H
#define DESKTOP_SETTINGSMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QModelIndex>
#include <QSettings>
#include <algorithm>
#include <optional>

/**
 * An interface to application-wide settings.
 *
 * Unlike QSettings, this interface can expose non-persistent and static
 * platform-specific settings and provides all the normal benefits of
 * QAbstractItemModel for event-driven UI updates, commit/rollback, multiple
 * views into the same data, etc. It also prevents key typos!
 */
class Settings final : public QAbstractItemModel {
	Q_OBJECT
public:
	enum {
		ValueRole                  // QVariant
	};

	enum Row {
		Invalid,                   // null
		First,
		Theme = First,             // enum
		ThemeStyle,                // string (virtual)
		ThemePalette,              // enum (virtual)
		Language,                  // string
		LogFile,                   // bool
		AutoSaveInterval,          // int
		ConfirmLayerDelete,        // bool
		CanvasScrollBars,          // bool TODO: enum
		TabletEvents,              // bool
		TabletEraser,              // bool
		TabletDriver,              // enum (virtual)
		TabletWindowsInk,          // bool
		TabletRelativePenMode,     // bool
		Smoothing,                 // int
		GlobalPressureCurve,       // QString
		CurvesPresets,             // QVector<QMap<QString, QString>>
		CurvesPresetsConverted,    // bool
		OneFingerDraw,             // bool
		OneFingerScroll,           // bool
		OneFingerTouch,            // enum (virtual)
		TwoFingerZoom,             // bool
		TwoFingerRotate,           // bool
		ToolToggle,                // bool
		ShareBrushSlotColor,       // bool
		BrushCursor,               // enum
		BrushOutlineWidth,         // double
		ColorWheelShape,           // enum
		ColorWheelAngle,           // enum
		ColorWheelSpace,           // enum
		Shortcuts,                 // QMap<QString, QVariant>
		InsecurePasswordStorage,   // bool
		RecordPause,               // bool
		RecordMinimumPause,        // double
		RecordTimestamp,           // bool
		RecordTimestampInterval,   // int
		ServerPort,                // int
		ServerAutoReset,           // bool
		ServerTimeout,             // int
		ServerPrivateUserList,     // bool
		ServerHideIp,              // bool
		ListServers,               // QVector<QMap<QString, QVariant>>
		NewCanvasSize,             // QSize
		NewCanvasBackColor,        // QColor
		CompactChat,               // bool
		LastPalette,               // int
		LastWindowSize,            // QSize
		LastWindowPosition,        // QPoint
		LastWindowMaximized,       // bool
		LastWindowState,           // QByteArray
		LastWindowViewState,       // QByteArray
		LastWindowDocks,           // QMap<QString, QVariant>
		LastWindowActions,         // QMap<QString, bool>
		LastFileOpenPath,          // QString
		LastSessionTitle,          // QString
		LastIdAlias,               // QString
		LastAnnounce,              // bool
		LastListingServer,         // int
		LastHostRemote,            // bool
		LastJoinDialogSize,        // QSize
		LastUsername,              // QString
		LastAvatar,                // QString
		FilterLocked,              // bool
		FilterNsfm,                // bool
		FilterClosed,              // bool
		JoinListSortColumn,        // int
		MaxRecentFiles,            // int
		RecentFiles,               // QStringList
		RecentHosts,               // QStringList
		RecentRemoteHosts,         // QStringList
		SoundVolume,               // int
		NotificationChat,          // bool
		NotificationMarker,        // bool
		NotificationLogin,         // bool
		NotificationLogout,        // bool
		NotificationLock,          // bool
		NotificationUnlock,        // bool
		FlipbookFrameRate,         // int
		FlipbookWindow,            // QRect
		FlipbookCrop,              // QRect
		Layouts,                   // QVector<QMap<QString, QVariant>>
		FeatureControlLocked,      // bool
		FeatureControlLevel,       // int
		FeatureControlTags,        // QString
		FeatureControlAutoTag,     // bool
		FeatureControlForceCensor, // bool
		FeatureControlHideLocked,  // bool
		EngineFrameRate,           // int
		EngineSnapshotCount,       // int
		EngineSnapshotInterval,    // int
		EngineUndoDepth,           // int
		CanvasShortcuts,           // QMap<QString, QVariant>
		VersionCheckEnabled,       // bool
		VersionCheckLastCheck,     // QString
		VersionCheckLastSuccess,   // QString
		VersionCheckLatest,        // QString
		VideoExportFormat,         // int
		VideoExportFrameRate,      // int
		VideoExportFrameWidth,     // int
		VideoExportFrameHeight,    // int
		VideoExportSizeChoice,     // int
		VideoExportCustomFfmpeg,   // QString
		NavigatorShowCursors,      // bool
		NavigatorRealtime,         // bool
		OnionSkinsFrameCount,      // int
		OnionSkinsTintBelow,       // QColor
		OnionSkinsTintAbove,       // QColor
		OnionSkinsFrames,          // QVector<int>
		LastToolColor,             // QColor
		LastTool,                  // int
		Toolset,                   // QMap<QString, QMap<QString, QVariant>>
		ShowTrayIcon,              // bool
		GuiServerPort,             // int
		ServerUseSsl,              // bool
		ServerSslCert,             // QString
		ServerSslKey,              // QString
		ServerLocalAddress,        // QString
		ServerExtAuth,             // QString
		ServerSessionStorage,      // QString
		HideDockTitleBars,         // bool (non-persistent)
		RowCount,
		Last = RowCount
	};

	enum class ThemeEnum : int {
		System,
		FusionLight,
		FusionDark,
		KritaBright,
		KritaDark,
		KritaDarker,
		SystemLight,
		SystemDark,
		HotdogStand,
		Count,
#ifdef Q_OS_MACOS
		Default = System,
#else
		Default = KritaDark,
#endif
	};

	Q_ENUM(ThemeEnum)

	enum class BrushCursorEnum : int {
		Dot,
		Cross,
		Arrow,
		TriangleRight,
		TriangleLeft,
		Count,
	};

	Q_ENUM(BrushCursorEnum)

	enum class MetaVersion : int {
		V0 = 0,
		V1,
		Max = V1
	};

	Settings(QObject *parent = nullptr);

	int columnCount(const QModelIndex & = QModelIndex()) const override { return 1; }
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &) const override { return QModelIndex(); }
	int rowCount(const QModelIndex & = QModelIndex()) const override { return RowCount; }
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	/// @brief Binds a setting unidirectionally to a callback method. The
	/// callback will be invoked immediately with the current value of the
	/// setting.
	/// @param row The setting to bind.
	/// @param receiver The method that will receive new setting values.
	/// @return A connection object that can be used to destroy the binding.
	template <typename T>
	auto bind(Row row, void (*receiver)(T)) const
	{
		std::invoke(receiver, get<T>(row));
		return listen(row, receiver);
	}

	/// @brief Binds a setting unidirectionally to an object using the given
	/// receiver method. The receiver will be invoked immediately with the
	/// current value of the setting.
	/// @param row The setting to bind.
	/// @param object The object to bind.
	/// @param receiver The method that will receive new setting values.
	/// @return A connection object that can be used to destroy the binding.
	template <typename O, typename T>
	auto bind(Row row, O *object, void (O::*receiver)(T)) const
	{
		std::invoke(receiver, object, get<T>(row));
		return listen(row, object, receiver);
	}

	/// @brief Binds a setting bidirectionally to an object using the given
	/// receiver method and signal. The receiver will be invoked immediately
	/// with the current value of the setting.
	/// @param row The setting to bind.
	/// @param object The object to bind.
	/// @param receiver The method that will receive new setting values.
	/// @param signal The signal that emits new setting values.
	/// @return A connection object that can be used to destroy the binding.
	template <typename O, typename T>
	auto bind(Row row, O *object, void (O::*receiver)(T), void (O::*signal)(T))
	{
		std::invoke(receiver, object, get<T>(row));
		return listen(row, object, receiver, signal);
	}

	/// @brief Binds a setting unidirectionally to a callback method. The
	/// callback method is only invoked when a change to the setting occurs.
	/// @param row The setting to bind.
	/// @param receiver The method that will receive new setting values.
	/// @return A connection object that can be used to destroy the binding.
	template <typename T>
	auto listen(Row row, void (*receiver)(T)) const
	{
		return connect(this, &Settings::dataChanged, [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &) {
			if (topLeft.row() <= row && bottomRight.row() >= row) {
				receiver(get<T>(row));
			}
		});
	}

	/// @brief Binds a setting unidirectionally to an object using the given
	/// receiver method. The receiver is only invoked when a change to the
	/// setting occurs.
	/// @param row The setting to bind.
	/// @param receiver The method that will receive new setting values.
	/// @return A connection object that can be used to destroy the binding.
	template <typename O, typename T>
	auto listen(Row row, O *object, void (O::*receiver)(T)) const
	{
		return connect(this, &Settings::dataChanged, object, [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &) {
			if (topLeft.row() <= row && bottomRight.row() >= row) {
				std::invoke(receiver, object, get<T>(row));
			}
		});
	}

	/// @brief Binds a setting bidirectionally to an object using the given
	/// receiver method and signal. The receiver is only invoked when a change
	/// to the setting occurs.
	/// @param row The setting to bind.
	/// @param object The object to bind.
	/// @param receiver The method that will receive new setting values.
	/// @param signal The signal that emits new setting values.
	/// @return A connection object that can be used to destroy the binding.
	template <typename O, typename T>
	auto listen(Row row, O *object, void (O::*receiver)(T), void (O::*signal)(T))
	{
		// TODO: Cannot make one connection out of two connections.
		listen(row, object, receiver);
		return connect(object, signal, this, [=](T value) {
			set(row, value);
		});
	}

	template <typename T>
	T get(Row row) const
	{
		return data(createIndex(row, 0), ValueRole).value<T>();
	}

	template <typename T>
	bool set(Row row, T value)
	{
		return setData(createIndex(row, 0), value, ValueRole);
	}

public slots:
	void revert() override;
	bool submit() override;

private:
	inline bool isValidIndex(int row, int column, const QModelIndex &parent) const
	{
		return column == 0 && row >= 0 && row < RowCount && !parent.isValid();
	}

	inline bool isValidIndex(const QModelIndex &index) const
	{
		return isValidIndex(index.row(), index.column(), index.parent());
	}

	QVariant dataFromStorage(Row row) const;
	QVariant downgradeData(Row row, MetaVersion version, const QVariant &value) const;
	QVariant upgradeData(Row row, MetaVersion version, QVariant &&value) const;

	/// @brief Current settings data.
	QSettings m_settings;

	/// @brief Uncommitted settings changes.
	QHash<Row, QVariant> m_pending;
};

#endif
