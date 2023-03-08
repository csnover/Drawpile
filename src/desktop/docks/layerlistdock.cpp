// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/widgets/groupedtoolbutton.h"
#include "libclient/canvas/acl.h"
#include "libclient/canvas/layerlist.h"
#include "libclient/canvas/canvasmodel.h"
#include "libclient/canvas/userlist.h"
#include "libclient/canvas/paintengine.h"
#include "desktop/docks/layerlistdock.h"
#include "desktop/docks/layerlistdelegate.h"
#include "desktop/docks/layeraclmenu.h"
#include "desktop/docks/titlewidget.h"
#include "desktop/dialogs/layerproperties.h"
#include "desktop/utils/dynamicui.h"
#include "libclient/utils/icon.h"
#include "libclient/net/envelopebuilder.h"

#include "rustpile/rustpile.h"

#include <QDebug>
#include <QItemSelection>
#include <QMessageBox>
#include <QPushButton>
#include <QActionGroup>
#include <QTimer>
#include <QSettings>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QTreeView>

namespace docks {

LayerList::LayerList(QWidget *parent)
	: QDockWidget(parent)
	, m_canvas(nullptr)
	, m_selectedId(0)
	, m_nearestToDeletedId(0)
	, m_noupdate(false)
	, m_addLayerAction(nullptr)
	, m_duplicateLayerAction(nullptr)
	, m_mergeLayerAction(nullptr)
	, m_deleteLayerAction(nullptr)
{
	AUTO_TR(this, setWindowTitle, tr("Layers"));

	auto *titlebar = new TitleWidget(this);
	setTitleBarWidget(titlebar);

	m_view = new QTreeView;
	m_view->setHeaderHidden(true);
	setWidget(m_view);

	m_view->setDragEnabled(true);
	m_view->viewport()->setAcceptDrops(true);
	m_view->setEnabled(false);
	m_view->setSelectionMode(QAbstractItemView::SingleSelection);
	m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_view->setContextMenuPolicy(Qt::CustomContextMenu);

	m_contextMenu = new QMenu(this);
	connect(m_view, &QTreeView::customContextMenuRequested, this, &LayerList::showContextMenu);

	// Custom layer list item delegate
	LayerListDelegate *del = new LayerListDelegate(this);
	connect(del, &LayerListDelegate::toggleVisibility, this, &LayerList::setLayerVisibility);
	connect(del, &LayerListDelegate::editProperties, this, &LayerList::showPropertiesOfIndex);
	m_view->setItemDelegate(del);
}

void LayerList::setCanvas(canvas::CanvasModel *canvas)
{
	m_canvas = canvas;
	m_view->setModel(canvas->layerlist());

	m_aclmenu->setUserList(canvas->userlist()->onlineUsers());

	connect(canvas->layerlist(), &canvas::LayerListModel::modelAboutToBeReset, this, &LayerList::beforeLayerReset);
	connect(canvas->layerlist(), &canvas::LayerListModel::modelReset, this, &LayerList::afterLayerReset);

	connect(canvas->aclState(), &canvas::AclState::featureAccessChanged, this, &LayerList::onFeatureAccessChange);
	connect(canvas->aclState(), &canvas::AclState::layerAclChanged, this, &LayerList::lockStatusChanged);
	connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged(QItemSelection)));

	// Init
	m_view->setEnabled(true);
	updateLockedControls();
}

void LayerList::setLayerEditActions(QAction *addLayer, QAction *addGroup, QAction *duplicate, QAction *merge, QAction *properties, QAction *del)
{
	Q_ASSERT(addLayer);
	Q_ASSERT(addGroup);
	Q_ASSERT(duplicate);
	Q_ASSERT(merge);
	Q_ASSERT(del);
	m_addLayerAction = addLayer;
	m_addGroupAction = addGroup;
	m_duplicateLayerAction = duplicate;
	m_mergeLayerAction = merge;
	m_propertiesAction = properties;
	m_deleteLayerAction = del;

	// Add the actions to the header bar
	TitleWidget *titlebar = qobject_cast<TitleWidget*>(titleBarWidget());
	Q_ASSERT(titlebar);

	m_lockButton = new widgets::GroupedToolButton(widgets::GroupedToolButton::NotGrouped, titlebar);
	m_lockButton->setIcon(icon::fromTheme("object-locked"));
	m_lockButton->setCheckable(true);
	m_lockButton->setPopupMode(QToolButton::InstantPopup);
	titlebar->addSpace(m_lockButton->sizeHint().width());

	// Layer ACL menu
	m_aclmenu = new LayerAclMenu(this);
	connect(m_aclmenu, &LayerAclMenu::layerAclChange, this, &LayerList::changeLayerAcl);
	connect(m_aclmenu, &LayerAclMenu::layerCensoredChange, this, &LayerList::censorSelected);
	m_lockButton->setMenu(m_aclmenu);

	titlebar->addStretch();

	auto *addLayerButton = new widgets::GroupedToolButton(widgets::GroupedToolButton::GroupLeft, titlebar);
	addLayerButton->setDefaultAction(addLayer);
	titlebar->addCustomWidget(addLayerButton);

	auto *addGroupButton = new widgets::GroupedToolButton(widgets::GroupedToolButton::GroupCenter, titlebar);
	addGroupButton->setDefaultAction(addGroup);
	titlebar->addCustomWidget(addGroupButton);

	auto *duplicateLayerButton = new widgets::GroupedToolButton(widgets::GroupedToolButton::GroupCenter, titlebar);
	duplicateLayerButton->setDefaultAction(m_duplicateLayerAction);
	titlebar->addCustomWidget(duplicateLayerButton);

	auto *mergeLayerButton = new widgets::GroupedToolButton(widgets::GroupedToolButton::GroupCenter, titlebar);
	mergeLayerButton->setDefaultAction(m_mergeLayerAction);
	titlebar->addCustomWidget(mergeLayerButton);

	auto *propertiesButton = new widgets::GroupedToolButton(widgets::GroupedToolButton::GroupCenter, titlebar);
	propertiesButton->setDefaultAction(m_propertiesAction);
	titlebar->addCustomWidget(propertiesButton);

	auto *deleteLayerButton = new widgets::GroupedToolButton(widgets::GroupedToolButton::GroupRight, titlebar);
	deleteLayerButton->setDefaultAction(m_deleteLayerAction);
	titlebar->addCustomWidget(deleteLayerButton);

	titlebar->addStretch();

	// Add the actions to the context menu
	m_contextMenu->addAction(m_propertiesAction);
	m_contextMenu->addSeparator();
	m_contextMenu->addAction(m_addLayerAction);
	m_contextMenu->addAction(m_addGroupAction);
	m_contextMenu->addAction(m_duplicateLayerAction);
	m_contextMenu->addAction(m_mergeLayerAction);
	m_contextMenu->addAction(m_deleteLayerAction);

	// Action functionality
	connect(m_addLayerAction, &QAction::triggered, this, &LayerList::addLayer);
	connect(m_addGroupAction, &QAction::triggered, this, &LayerList::addGroup);
	connect(m_duplicateLayerAction, &QAction::triggered, this, &LayerList::duplicateLayer);
	connect(m_mergeLayerAction, &QAction::triggered, this, &LayerList::mergeSelected);
	connect(m_propertiesAction, &QAction::triggered, this, &LayerList::showPropertiesOfSelected);
	connect(m_deleteLayerAction, &QAction::triggered, this, &LayerList::deleteSelected);

	titlebar->addCustomWidget(m_lockButton);

	updateLockedControls();
}

void LayerList::onFeatureAccessChange(canvas::Feature feature, bool canUse)
{
	Q_UNUSED(canUse);
	switch(feature) {
		case canvas::Feature::EditLayers:
		case canvas::Feature::OwnLayers:
			updateLockedControls();
		default: break;
	}
}

void LayerList::updateLockedControls()
{
	// The basic permissions
	const bool canEdit = m_canvas && m_canvas->aclState()->canUseFeature(canvas::Feature::EditLayers);
	const bool ownLayers = m_canvas && m_canvas->aclState()->canUseFeature(canvas::Feature::OwnLayers);

	// Layer creation actions work as long as we have an editing permission
	const bool canAdd = canEdit | ownLayers;
	const bool hasEditActions = m_addLayerAction != nullptr;
	if(hasEditActions) {
		m_addLayerAction->setEnabled(canAdd);
	}

	// Rest of the controls need a selection to work.
	const bool enabled = m_selectedId && (canEdit || (ownLayers && (m_selectedId>>8) == m_canvas->localUserId()));

	m_lockButton->setEnabled(enabled);

	if(hasEditActions) {
		m_duplicateLayerAction->setEnabled(enabled);
		m_propertiesAction->setEnabled(enabled);
		m_deleteLayerAction->setEnabled(enabled);
		m_mergeLayerAction->setEnabled(enabled && canMergeCurrent());
	}
}

void LayerList::selectLayer(int id)
{
	selectLayerIndex(m_canvas->layerlist()->layerIndex(id), true);
}

void LayerList::selectLayerIndex(QModelIndex index, bool scrollTo)
{
	if(index.isValid()) {
		m_view->selectionModel()->select(
			index, QItemSelectionModel::SelectCurrent|QItemSelectionModel::Clear);
		if(scrollTo) {
			m_view->setExpanded(index, true);
			m_view->scrollTo(index);
		}
	}
}

void LayerList::censorSelected(bool censor)
{
	QModelIndex index = currentSelection();
	if(!index.isValid())
		return;

	auto *layers = m_canvas->layerlist();
	Q_ASSERT(layers);
	layers->toggleLayerFlags(index, rustpile::LayerAttributesMessage_FLAGS_CENSOR, censor);
	layers->submit();
}

void LayerList::setLayerVisibility(int layerId, bool visible)
{
	rustpile::paintengine_set_layer_visibility(
		m_canvas->paintEngine()->engine(),
		layerId,
		visible
	);
}

void LayerList::changeLayerAcl(bool lock, rustpile::Tier tier, QVector<uint8_t> exclusive)
{
	const QModelIndex index = currentSelection();
	if(!index.isValid())
		return;

	auto *layers = m_canvas->layerlist();
	Q_ASSERT(layers);
	layers->changeLayerAcl(index, lock, tier, exclusive);
	layers->submit();
}

/**
 * @brief Layer add button pressed
 */
void LayerList::addLayer()
{
	auto *layers = m_canvas->layerlist();
	Q_ASSERT(layers);
	layers->addLayer(m_selectedId);
	layers->submit();
}

void LayerList::addGroup()
{
	auto *layers = m_canvas->layerlist();
	Q_ASSERT(layers);
	layers->addGroup(m_selectedId);
	layers->submit();
}

void LayerList::duplicateLayer()
{
	auto *layers = m_canvas->layerlist();
	Q_ASSERT(layers);
	layers->duplicateLayer(currentSelection());
	layers->submit();
}

bool LayerList::canMergeCurrent() const
{
	const QModelIndex index = currentSelection();
	const QModelIndex below = index.sibling(index.row()+1, 0);

	return index.isValid() && below.isValid() &&
			!below.data(canvas::LayerListModel::IsGroupRole).toBool() &&
			!m_canvas->aclState()->isLayerLocked(below.data(canvas::LayerListModel::IdRole).toInt())
			;
}

void LayerList::deleteSelected()
{
	auto *layers = m_canvas->layerlist();
	Q_ASSERT(layers);
	layers->removeLayer(currentSelection());
	layers->submit();
}

void LayerList::mergeSelected()
{
	auto *layers = m_canvas->layerlist();
	Q_ASSERT(layers);

	QModelIndex index = currentSelection();
	if(!index.isValid())
		return;

	QModelIndex below = index.sibling(index.row()+1, 0);
	if(!below.isValid())
		return;

	layers->mergeLayers(index, below);
	layers->submit();
}

void LayerList::showPropertiesOfSelected()
{
	showPropertiesOfIndex(currentSelection());
}

void LayerList::showPropertiesOfIndex(QModelIndex index)
{
	if(!index.isValid())
		return;

	Q_ASSERT(m_canvas);
	auto *dlg = new dialogs::LayerProperties(*m_canvas, index, this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setModal(false);

	connect(dlg, &dialogs::LayerProperties::visibilityChanged, this, &LayerList::setLayerVisibility);
	dlg->show();
}

void LayerList::showContextMenu(const QPoint &pos)
{
	QModelIndex index = m_view->indexAt(pos);
	if(index.isValid()) {
		m_contextMenu->popup(m_view->mapToGlobal(pos));
	}
}

void LayerList::beforeLayerReset()
{
	m_nearestToDeletedId = m_canvas->layerlist()->findNearestLayer(m_selectedId);

	m_expandedGroups.clear();
	for(const auto &item : m_canvas->layerlist()->layerItems()) {
		if(m_view->isExpanded(m_canvas->layerlist()->layerIndex(item.id)))
			m_expandedGroups << item.id;
	}
	m_lastScrollPosition = m_view->verticalScrollBar()->value();
}

void LayerList::afterLayerReset()
{
	const bool wasAnimated = m_view->isAnimated();
	m_view->setAnimated(false);
	if(m_selectedId) {
		const auto selectedIndex = m_canvas->layerlist()->layerIndex(m_selectedId);
		if(selectedIndex.isValid()) {
			selectLayerIndex(selectedIndex);
		} else {
			selectLayer(m_nearestToDeletedId);
		}
	}

	for(const int id : qAsConst(m_expandedGroups))
		m_view->setExpanded(m_canvas->layerlist()->layerIndex(id), true);

	m_view->verticalScrollBar()->setValue(m_lastScrollPosition);
	m_view->setAnimated(wasAnimated);
}

QModelIndex LayerList::currentSelection() const
{
	QModelIndexList sel = m_view->selectionModel()->selectedIndexes();
	if(sel.isEmpty())
		return QModelIndex();
	return sel.first();
}

bool LayerList::isCurrentLayerLocked() const
{
	if(!m_canvas)
		return false;

	QModelIndex idx = currentSelection();
	if(idx.isValid()) {
		const canvas::LayerListItem &item = idx.data(canvas::LayerListModel::ItemRole).value<canvas::LayerListItem>();
		return item.hidden
			|| item.group // group layers have no pixel content to edit
			|| m_canvas->aclState()->isLayerLocked(item.id)
			|| (item.attributes.censored && m_canvas->paintEngine()->isCensored())
			;
	}
	return false;
}

void LayerList::selectionChanged(const QItemSelection &selected)
{
	bool on = selected.count() > 0;

	if(on) {
		updateUiFromSelection();
	} else {
		m_selectedId = 0;
	}

	updateLockedControls();

	emit layerSelected(m_selectedId);
}

void LayerList::updateUiFromSelection()
{
	const canvas::LayerListItem &layer = currentSelection().data(canvas::LayerListModel::ItemRole).value<canvas::LayerListItem>();
	m_noupdate = true;
	m_selectedId = layer.id;

	m_aclmenu->setCensored(layer.attributes.censored);

	lockStatusChanged(layer.id);
	updateLockedControls();

	// TODO use change flags to detect if this really changed
	emit activeLayerVisibilityChanged();
	m_noupdate = false;
}

void LayerList::lockStatusChanged(int layerId)
{
	if(m_selectedId == layerId) {
		const auto acl = m_canvas->aclState()->layerAcl(layerId);
		m_lockButton->setChecked(acl.locked || acl.tier != rustpile::Tier::Guest || !acl.exclusive.isEmpty());
		m_aclmenu->setAcl(acl.locked, acl.tier, acl.exclusive);

		emit activeLayerVisibilityChanged();
	}
}

}
