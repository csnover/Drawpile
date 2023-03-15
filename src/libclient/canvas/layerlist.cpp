// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/canvas/layerlist.h"
#include "libclient/utils/changeflags.h"
#include "rustpile/rustpile.h"
#include "libshared/util/qtcompat.h"

#include <QBrush>
#include <QDebug>
#include <QFont>
#include <QIcon>
#include <QImage>
#include <QPalette>
#include <QStringList>
#include <QRegularExpression>

namespace canvas {

LayerListModel::LayerListModel(AclState &aclState, QObject *parent)
	: QAbstractItemModel(parent)
	, m_aclstate(aclState)
	, m_rootLayerCount(0)
	, m_defaultLayer(0)
	, m_autoselectAny(true)
	, m_frameMode(false)
{
}

QVariant LayerListModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	const LayerListItem &item = m_items.at(index.internalId());

	switch(role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
		return item.title;
	case Qt::DecorationRole:
		if (item.hidden)
			return QIcon::fromTheme("layer-visible-off");
		else if (item.attributes.censored)
			return QIcon(":/icons/censored.svg");
		else if (item.group)
			return QIcon::fromTheme("folder");
		else
			return QIcon::fromTheme("layer-visible-on");
	case Qt::FontRole:
		if (item.id == m_defaultLayer) {
			auto font = QFont();
			font.setUnderline(true);
			return font;
		}
		break;
	case IdRole:
		return item.id;
	case IsDefaultRole:
		return item.id == m_defaultLayer;
	case IsLockedRole:
		return (m_frameMode && !m_frameLayers.contains(item.frameId))
			|| (m_aclstate.isLayerLocked(item.id));
	case IsGroupRole:
		return item.group;
	case OpacityRole:
		return item.attributes.opacity;
	case ItemRole:
		return QVariant::fromValue(item);
	default: {}
	}

	return QVariant();
}

void LayerListModel::addGroup(int target)
{
	addLayerWithFlags(target, tr("Group"), rustpile::LayerCreateMessage_FLAGS_GROUP);
}

void LayerListModel::addLayer(int target)
{
	addLayerWithFlags(target, tr("Layer"), 0);
}

void LayerListModel::duplicateLayer(const QModelIndex &index)
{
	const int id = getAvailableLayerId();
	if(!index.isValid() || id==0)
		return;

	const auto layer = index.data(IdRole).toInt();
	const auto title = index.data(Qt::DisplayRole).toString();

	const QString name = getAvailableLayerName(title);

	rustpile::write_undopoint(m_eb, m_aclstate.localUserId());
	rustpile::write_newlayer(
		m_eb,
		m_aclstate.localUserId(),
		id,
		layer, // source
		layer, // target
		0, // fill
		0, // flags
		reinterpret_cast<const uint16_t*>(name.constData()),
		name.length()
	);
}

void LayerListModel::removeLayer(const QModelIndex &index)
{
	rustpile::write_undopoint(m_eb, m_aclstate.localUserId());
	rustpile::write_deletelayer(m_eb, m_aclstate.localUserId(), index.data(IdRole).toUInt(), false);
}

void LayerListModel::mergeLayers(const QModelIndex &index, const QModelIndex &below)
{
	rustpile::write_undopoint(m_eb, m_aclstate.localUserId());
	rustpile::write_deletelayer(
		m_eb,
		m_aclstate.localUserId(),
		index.data(IdRole).value<uint16_t>(),
		below.data(IdRole).value<uint16_t>()
	);
}

void LayerListModel::toggleLayerFlags(const QModelIndex &index, uint8_t flags, bool on)
{
	if (!index.isValid())
		return;

	auto &item = m_items[index.internalId()];
	const auto newFlags = ChangeFlags<uint8_t>().set(flags, on).update(item.attributeFlags());
	item.attributes.setFlags(newFlags);
	rustpile::write_layerattr(
		m_eb,
		m_aclstate.localUserId(),
		item.id,
		0,
		newFlags,
		item.attributes.intOpacity(),
		item.attributes.blend
	);
	emit dataChanged(index, index, { AttributesRole, ItemRole });
}

void LayerListModel::changeLayerAcl(const QModelIndex &index, bool lock, rustpile::Tier tier, QVector<uint8_t> exclusive)
{
	if (!index.isValid())
		return;

	const auto layerId = m_items.at(index.internalId()).id;
	const auto acl = m_aclstate.layerAcl(layerId);

	if (acl.locked != lock || acl.tier != tier || acl.exclusive != exclusive) {
		rustpile::write_layeracl(
			m_eb,
			m_aclstate.localUserId(),
			layerId,
			(lock ? 0x80 : 0) | uint8_t(tier),
			exclusive.constData(),
			exclusive.length()
		);
		emit dataChanged(index, index, { IsLockedRole });
	}
}

void LayerListModel::addLayerWithFlags(int target, QString basename, uint8_t flags)
{
	const int id = getAvailableLayerId();
	if(id==0) {
		qWarning("Couldn't find a free ID for a new layer!");
		return;
	}

	const QString name = getAvailableLayerName(basename);

	rustpile::write_undopoint(m_eb, m_aclstate.localUserId());
	rustpile::write_newlayer(
		m_eb,
		m_aclstate.localUserId(),
		id,
		0, // source
		target, // target
		0, // fill
		flags, // flags
		reinterpret_cast<const uint16_t*>(name.constData()),
		name.length()
	);
}

bool LayerListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!index.isValid())
		return false;

	auto &item = m_items[index.internalId()];

	switch(role) {
	case Qt::DisplayRole:
	case Qt::EditRole: {
		const auto title = value.toString();
		if(title != item.title) {
			item.title = title;
			rustpile::write_retitlelayer(
				m_eb,
				m_aclstate.localUserId(),
				item.id,
				reinterpret_cast<const uint16_t*>(title.constData()),
				title.length()
			);
			emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole, ItemRole });
		}
		return true;
	}
	case AttributesRole: {
		const auto attrs = value.value<canvas::LayerListItem::Attributes>();
		if (item.attributes != attrs) {
			item.attributes = attrs;
			rustpile::write_layerattr(
				m_eb,
				m_aclstate.localUserId(),
				item.id,
				0,
				attrs.flags(),
				attrs.intOpacity(),
				attrs.blend
			);
			emit dataChanged(index, index, { role, ItemRole });
		}
		return true;
	}
	case IsDefaultRole: {
		if(value.toBool() && m_defaultLayer != item.id) {
			rustpile::write_defaultlayer(m_eb, m_aclstate.localUserId(), item.id);
			setDefaultLayer(item.id);
		}
		return true;
	}
	default: {}
	}

	return false;
}

void LayerListModel::revert()
{
	m_eb = net::EnvelopeBuilder{};
}

bool LayerListModel::submit()
{
	const auto envelope = m_eb.toEnvelope();
	if (!envelope.isEmpty()) {
		emit layerCommand(envelope);
		m_eb = net::EnvelopeBuilder{};
	}
	return true;
}

Qt::ItemFlags LayerListModel::flags(const QModelIndex& index) const
{
	auto flags = Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;

	if(index.isValid()) {
		const auto isGroup = m_items.at(index.internalId()).group;
		flags |= Qt::ItemIsEditable | (isGroup ? Qt::ItemIsDropEnabled : Qt::ItemNeverHasChildren);
	} else {
		flags |= Qt::ItemIsDropEnabled;
	}

	return flags;
}

Qt::DropActions LayerListModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

QStringList LayerListModel::mimeTypes() const {
	return QStringList() << "application/x-qt-image";
}

QMimeData *LayerListModel::mimeData(const QModelIndexList& indexes) const
{
	return new LayerMimeData(this, indexes[0].data(canvas::LayerListModel::ItemRole).value<LayerListItem>().id);
}

bool LayerListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_UNUSED(action);
	Q_UNUSED(column);
	Q_UNUSED(parent);

	const LayerMimeData *ldata = qobject_cast<const LayerMimeData*>(data);
	if(ldata && ldata->source() == this) {
		if(m_items.size() < 2)
			return false;

		int targetId;
		bool intoGroup = false;
		bool below = false;
		if(row < 0) {
			if(parent.isValid()) {
				// row<0, valid parent: move into the group
				targetId = m_items.at(parent.internalId()).id;
				intoGroup = true;
			} else {
				// row<0, no parent: the empty area below the layer list (move to root/bottom)
				targetId = m_items.at(index(rowCount()-1, 0).internalId()).id;
				below = true;
			}
		} else {
			const int children = rowCount(parent);
			if(row >= children) {
				// row >= number of children in group (or root): move below the last item in the group
				targetId = m_items.at(index(children-1, 0, parent).internalId()).id;
				below = true;
			} else {
				// the standard case: move above this layer
				targetId = m_items.at(index(row, 0, parent).internalId()).id;
			}
		}

#if 0
		qInfo("Drop row=%d (parent row=%d, id=%d, rowCount=%d)", row, parent.row(), parent.internalId(), rowCount(parent));
		for(int i=0;i<m_items.size();++i) {
			qInfo("[%d] id=%d, children=%d", i, m_items.at(i).id, m_items.at(i).children);
		}
		qInfo("Requesting move of %d to %s %d, into=%d", ldata->layerId(), below ? "below" : "above", targetId, intoGroup);
#endif

		emit moveRequested(ldata->layerId(), targetId, intoGroup, below);

	} else {
		// TODO support new layer drops
		qWarning("External layer drag&drop not supported");
	}
	return false;
}

QModelIndex LayerListModel::layerIndex(uint16_t id) const
{
	for(int i=0;i<m_items.size();++i) {
		if(m_items.at(i).id == id) {
			return createIndex(m_items.at(i).relIndex, 0, i);
		}
	}

	return QModelIndex();
}

int LayerListModel::findNearestLayer(int layerId) const
{
	const auto i = layerIndex(layerId);
	const int row = i.row();

	auto nearest = i.sibling(row+1, 0);
	if(nearest.isValid())
		return m_items.at(nearest.internalId()).id;

	nearest = i.sibling(row-1, 0);
	if(nearest.isValid())
		return m_items.at(nearest.internalId()).id;

	nearest = i.parent();
	if(nearest.isValid())
		return m_items.at(nearest.internalId()).id;

	return 0;
}

int LayerListModel::rowCount(const QModelIndex &parent) const
{
	if(parent.isValid()) {
		return m_items.at(parent.internalId()).children;
	}

	return m_rootLayerCount;
}

int LayerListModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 1;
}

QModelIndex LayerListModel::parent(const QModelIndex &index) const
{
	if(!index.isValid())
		return QModelIndex();

	int seek = index.internalId();

	const int right = m_items.at(seek).right;
	while(--seek >= 0) {
		if(m_items.at(seek).right > right) {
			return createIndex(m_items.at(seek).relIndex, 0, seek);
		}
	}

	return QModelIndex();
}

QModelIndex LayerListModel::index(int row, int column, const QModelIndex &parent) const
{
	if(m_items.isEmpty() || row < 0 || column != 0)
		return QModelIndex();

	int cursor;

	if(parent.isValid()) {
		Q_ASSERT(m_items.at(parent.internalId()).group);
		cursor = parent.internalId();
		if(row >= m_items.at(cursor).children)
			return QModelIndex();

		cursor += 1; // point to the first child element

	} else {
		if(row >= m_rootLayerCount)
			return QModelIndex();

		cursor = 0;
	}

	int next = m_items.at(cursor).right + 1;

	int i = 0;
	while(i < row) {
		while(cursor < m_items.size() && m_items.at(cursor).left < next)
			++cursor;

		if(cursor == m_items.size() || m_items.at(cursor).left > next)
			return QModelIndex();

		next = m_items.at(cursor).right + 1;
		++i;
	}

#if 0
	qInfo("index(row=%d), parent row=%d (id=%d, children=%d, group=%d), relIndex=%d, cursor=%d, left=%d, right=%d",
		  row,
		  parent.row(), int(parent.internalId()), m_items.at(parent.internalId()).children, m_items.at(parent.internalId()).group,
		  m_items.at(cursor).relIndex, cursor,
		  m_items.at(cursor).left, m_items.at(cursor).right
		  );
#endif

	Q_ASSERT(m_items.at(cursor).relIndex == row);

	return createIndex(row, column, cursor);
}

void LayerListModel::setLayers(const QVector<LayerListItem> &items)
{
	// See if there are any new layers we should autoselect
	int autoselect = -1;

	const uint8_t localUser = m_aclstate.localUserId();

	if(m_items.size() < items.size()) {
		for(const LayerListItem &newItem : items) {
			// O(n²) loop but the number of layers is typically small enough that
			// it doesn't matter
			bool isNew = true;
			for(const LayerListItem &oldItem : qAsConst(m_items)) {
				if(oldItem.id == newItem.id) {
					isNew = false;
					break;
				}
			}
			if(!isNew)
				continue;

			// Autoselection rules:
			// 1. If we haven't participated yet, and there is a default layer,
			//    only select the default layer
			// 2. If we haven't participated in the session yet, select any new layer
			// 3. Otherwise, select any new layer that was created by us
			// TODO implement the other rules
			if(
					newItem.creatorId() == localUser ||
					(m_autoselectAny && (
						 (m_defaultLayer>0 && newItem.id == m_defaultLayer)
						 || m_defaultLayer==0
						 )
					 )
				) {
				autoselect = newItem.id;
				break;
			}
		}
	}

	// Count root layers
	int rootLayers = 0;
	if(!items.isEmpty()) {
		++rootLayers;
		int next = items[0].right + 1;
		for(int i=1;i<items.length();++i) {
			if(items[i].left == next) {
				++rootLayers;
				next = items[i].right + 1;
			}
		}
	}

	beginResetModel();
	m_rootLayerCount = rootLayers;
	m_items = items;
	endResetModel();

	if(autoselect>=0)
		emit autoSelectRequest(autoselect);
}

void LayerListModel::setLayersVisibleInFrame(const QVector<int> &layers, bool frameMode)
{
	beginResetModel(); //FIXME don't reset the whole model
	m_frameLayers = layers;
	m_frameMode = frameMode;
	endResetModel();
}

void LayerListModel::setDefaultLayer(uint16_t id)
{
	if(id == m_defaultLayer)
		return;

	const QVector<int> role { IsDefaultRole };

	const auto oldIdx = layerIndex(m_defaultLayer);
	if(oldIdx.isValid())
		emit dataChanged(oldIdx, oldIdx, role);

	m_defaultLayer = id;

	const auto newIdx = layerIndex(m_defaultLayer);
	if(newIdx.isValid())
		emit dataChanged(newIdx, newIdx, role);
}

int LayerListModel::getAvailableLayerId() const
{
	const int prefix = int(m_aclstate.localUserId()) << 8;
	QList<int> takenIds;
	for(const LayerListItem &item : m_items) {
		if((item.id & 0xff00) == prefix)
			takenIds.append(item.id);
	}

	for(int i=0;i<256;++i) {
		int id = prefix | i;
		if(!takenIds.contains(id))
			return id;
	}

	return 0;
}

QString LayerListModel::getAvailableLayerName(QString basename) const
{
	// Return a layer name of format "basename n" where n is one bigger than the
	// biggest suffix number of layers named "basename n".

	// First, strip suffix number from the basename (if it exists)

	QRegularExpression suffixNumRe("(\\d+)$");
	{
		auto m = suffixNumRe.match(basename);
		if(m.hasMatch()) {
			basename = basename.mid(0, m.capturedStart()).trimmed();
		}
	}

	// Find the biggest suffix in the layer stack
	int suffix = 0;
	for(const LayerListItem &l : m_items) {
		auto m = suffixNumRe.match(l.title);
		if(m.hasMatch()) {
			if(l.title.startsWith(basename)) {
				suffix = qMax(suffix, m.captured(1).toInt());
			}
		}
	}

	// Make unique name
	return QString("%2 %1").arg(suffix+1).arg(basename);
}

QStringList LayerMimeData::formats() const
{
	return QStringList() << "application/x-qt-image";
}

QVariant LayerMimeData::retrieveData(const QString &mimeType, compat::RetrieveDataMetaType type) const
{
	if(compat::isImageMime(mimeType, type) && m_source->m_getlayerfn)
		return m_source->m_getlayerfn(m_id);

	return QVariant();
}

uint8_t LayerListItem::Attributes::flags() const {
	return (censored ? rustpile::LayerAttributesMessage_FLAGS_CENSOR : 0)
		| (isolated ? rustpile::LayerAttributesMessage_FLAGS_ISOLATED : 0);
}

void LayerListItem::Attributes::setFlags(uint8_t flags) {
	censored = (flags & rustpile::LayerAttributesMessage_FLAGS_CENSOR) != 0;
	isolated = (flags & rustpile::LayerAttributesMessage_FLAGS_ISOLATED) != 0;
}

}
