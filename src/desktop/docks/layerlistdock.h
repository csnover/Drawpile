// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef LAYERLISTDOCK_H
#define LAYERLISTDOCK_H

#include "rustpile/rustpile.h"

#include <QDockWidget>

class QActionGroup;
class QModelIndex;
class QItemSelection;
class QMenu;
class QSlider;
class QTimer;
class QTreeView;

class Ui_LayerBox;

namespace net {
	class Envelope;
}

namespace canvas {
	class CanvasModel;
	enum class Feature;
}

namespace widgets {
	class GroupedToolButton;
}
namespace docks {

class LayerAclMenu;

class LayerList final : public QDockWidget
{
Q_OBJECT
public:
	LayerList(QWidget *parent=nullptr);

	void setCanvas(canvas::CanvasModel *canvas);

	//! These actions are shown in a menu outside this dock
	void setLayerEditActions(QAction *addLayer, QAction *addGroup, QAction *duplicate, QAction *merge, QAction *properties, QAction *del, LayerAclMenu *aclMenu, QMenu *blendMenu);

	/**
	 * Is the currently selected layer locked for editing?
	 *
	 * This may be because it is actually locked or because it is hidden or a
	 * non-editable (group) layer.
	 */
	bool isCurrentLayerLocked() const;

public slots:
	void selectLayer(int id);

signals:
	//! A layer was selected by the user
	void layerSelected(int id);
	void activeLayerVisibilityChanged();

private slots:
	void beforeLayerReset();
	void afterLayerReset();

	void onFeatureAccessChange(canvas::Feature feature, bool canuse);

	void addLayer();
	void addGroup();
	void duplicateLayer();
	void deleteSelected();
	void mergeSelected();

	void showPropertiesOfSelected();
	void showPropertiesOfIndex(QModelIndex index);
	void showContextMenu(const QPoint &pos);
	void censorSelected(bool censor);
	void setLayerVisibility(int layerId, bool visible);
	void changeOpacity(int value);
	void changeLayerAcl(bool lock, rustpile::Tier tier, QVector<uint8_t> exclusive);
	void changeLayerBlendMode(QAction *action);
	void changeDefaultLayer(bool on);

	void lockStatusChanged(int layerId);
	void selectionChanged(const QItemSelection &selected);

private:
	void updateLockedControls();
	bool canMergeCurrent() const;

	void updateUiFromSelection();

	QModelIndex currentSelection() const;
	void selectLayerIndex(QModelIndex index, bool scrollTo=false);

	canvas::CanvasModel *m_canvas;

	// cache selection and remember it across model resets
	int m_selectedId;
	int m_nearestToDeletedId;

	// try to retain view status across model resets
	QVector<int> m_expandedGroups;
	int m_lastScrollPosition;

	bool m_noupdate;

	QMenu *m_contextMenu;
	LayerAclMenu *m_aclMenu;
	QMenu *m_blendMenu;
	QSlider *m_opacity;

	widgets::GroupedToolButton *m_menuButton;
	QTreeView *m_view;

	QAction *m_addLayerAction;
	QAction *m_addGroupAction;
	QAction *m_duplicateLayerAction;
	QAction *m_mergeLayerAction;
	QAction *m_propertiesAction;
	QAction *m_deleteLayerAction;
};

}

#endif
