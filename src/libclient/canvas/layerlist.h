// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef DP_NET_LAYERLIST_H
#define DP_NET_LAYERLIST_H

#include "libclient/canvas/acl.h"
#include "libclient/canvas/canvasmodel.h"
#include "libclient/net/envelope.h"
#include "libclient/net/envelopebuilder.h"
#include "libshared/util/qtcompat.h"

#include <QAbstractItemModel>
#include <QMimeData>
#include <QVector>

#include <functional>

namespace rustpile {
	enum class Blendmode : uint8_t;
}

namespace canvas {
	class AclState;
}

namespace canvas {

struct LayerListItem {
	//! Layer ID
	// Note: normally, layer ID range is from 0 to 0xffff, but internal
	// layers use values outside that range. However, internal layers are not
	// shown in the layer list.
	uint16_t id;

	//! ID of the frame layer this layer belongs to
	uint16_t frameId;

	//! Layer title
	QString title;

	//! Layer attributes
	// These are combined as a single message so must be handled as a single
	// thing to avoid broken instructions
	struct Attributes {
		//! Layer opacity in the range 0–1
		float opacity;

		//! Layer is flagged for censoring
		bool censored;

		//! Isolated (not pass-through) group?
		bool isolated;

		//! Blending mode
		rustpile::Blendmode blend;

		bool operator==(const Attributes &other) const {
			return intOpacity() == other.intOpacity()
				&& censored == other.censored
				&& isolated == other.isolated
				&& blend == other.blend;
		}

		bool operator!=(const Attributes &other) const {
			return !operator==(other);
		}

		uint8_t intOpacity() const {
			return qRound(opacity * 255);
		}

		uint8_t flags() const;

		void setFlags(uint8_t flags);
	} attributes;

	//! Layer hidden flag (local only)
	bool hidden;

	//! Is this a layer group?
	bool group;

	//! Number of child layers
	uint16_t children;

	//! Index in parent group
	uint16_t relIndex;

	//! Left index (MPTT)
	int left;

	//! Right index (MPTT)
	int right;

	//! Get the LayerAttributes flags as a bitfield
	uint8_t attributeFlags() const {
		return attributes.flags();
	}

	//! Get the ID of the user who created this layer
	uint8_t creatorId() const { return uint8_t((id & 0xff00) >> 8); }
};

}

Q_DECLARE_TYPEINFO(canvas::LayerListItem, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(canvas::LayerListItem::Attributes, Q_MOVABLE_TYPE);

namespace canvas {

typedef std::function<QImage(int id)> GetLayerFunction;

class LayerListModel : public QAbstractItemModel {
	Q_OBJECT
	friend class LayerMimeData;
public:
	enum LayerListRoles {
		IdRole = Qt::UserRole + 1, // uint16_t
		IsDefaultRole,             // bool
		IsLockedRole,              // bool
		IsGroupRole,               // bool
		AttributesRole,            // canvas::LayerListItem::Attributes
		ItemRole,                  // canvas::LayerListItem
	};

	LayerListModel(AclState &aclState, QObject *parent=nullptr);

	int rowCount(const QModelIndex &parent=QModelIndex()) const override;
	int columnCount(const QModelIndex &parent=QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	Qt::DropActions supportedDropActions() const override;
	QStringList mimeTypes() const override;
	QMimeData *mimeData(const QModelIndexList& indexes) const override;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
	QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;

	QModelIndex layerIndex(uint16_t id) const;
	const QVector<LayerListItem> &layerItems() const { return m_items; }

	void addGroup(int target);
	void addLayer(int target);
	void duplicateLayer(const QModelIndex &index);
	void removeLayer(const QModelIndex &index);
	void mergeLayers(const QModelIndex &index, const QModelIndex &below);
	void toggleLayerFlags(const QModelIndex &index, uint8_t flags, bool on);
	void changeLayerAcl(const QModelIndex &index, bool lock, rustpile::Tier tier, QVector<uint8_t> exclusive);

	void setLayerGetter(GetLayerFunction fn) { m_getlayerfn = fn; }

	/**
	 * Enable/disable any (not just own) layer autoselect requests
	 *
	 * When the local user hasn't yet drawn anything, any newly created layer
	 * should be selected.
	 */
	void setAutoselectAny(bool autoselect) { m_autoselectAny = autoselect; }

	/**
	 * @brief Get the default layer to select when logging in
	 * Zero means no default.
	 */
	uint16_t defaultLayer() const { return m_defaultLayer; }
	void setDefaultLayer(uint16_t id);

	/**
	 * @brief Find a free layer ID
	 * @return layer ID or 0 if all are taken
	 */
	int getAvailableLayerId() const;

	/**
	 * @brief Find a unique name for a layer
	 * @param basename
	 * @return unique name
	 */
	QString getAvailableLayerName(QString basename) const;

	/**
	 * Find the nearest layer to the one given.
	 *
	 * This is used to find a layer to auto-select when the
	 * current selection is deleted.
	 *
	 * @returns layer ID or 0 if there are no layers
	 */
	int findNearestLayer(int layerId) const;

public slots:
	void revert() override;
	bool submit() override;
	void setLayers(const QVector<LayerListItem> &items);
	void setLayersVisibleInFrame(const QVector<int> &layers, bool frameMode);

signals:
	void layerCommand(net::Envelope envelope);

	//! A new layer was created that should be automatically selected
	void autoSelectRequest(int);

	//! Emitted when layers are manually reordered
	void moveRequested(int sourceId, int targetId, bool intoGroup, bool below);

private:
	void addLayerWithFlags(int target, QString basename, uint8_t flags);

	net::EnvelopeBuilder m_eb;
	QVector<LayerListItem> m_items;
	QVector<int> m_frameLayers;
	GetLayerFunction m_getlayerfn;
	AclState &m_aclstate;
	int m_rootLayerCount;
	uint16_t m_defaultLayer;
	bool m_autoselectAny;
	bool m_frameMode;
};

/**
 * A specialization of QMimeData for passing layers around inside
 * the application.
 */
class LayerMimeData : public QMimeData
{
Q_OBJECT
public:
	LayerMimeData(const LayerListModel *source, uint16_t id)
		: QMimeData(), m_source(source), m_id(id) {}

	const LayerListModel *source() const { return m_source; }

	uint16_t layerId() const { return m_id; }

	QStringList formats() const override;

protected:
	QVariant retrieveData(const QString& mimeType, compat::RetrieveDataMetaType type) const override;

private:
	const LayerListModel *m_source;
	uint16_t m_id;
};

}

Q_DECLARE_METATYPE(canvas::LayerListItem)
Q_DECLARE_METATYPE(canvas::LayerListItem::Attributes)

#endif
