/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2006-2021 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QDebug>
#include <QTimer>
#include <QApplication>

#include "desktop/scene/canvasscene.h"
#include "desktop/scene/canvasitem.h"
#include "desktop/scene/selectionitem.h"
#include "desktop/scene/annotationitem.h"
#include "desktop/scene/usermarkeritem.h"
#include "desktop/scene/lasertrailitem.h"

#include "libclient/canvas/canvasmodel.h"
#include "libclient/canvas/layerlist.h"
#include "libclient/canvas/paintengine.h"
#include "libclient/canvas/userlist.h"
#include "libclient/drawdance/annotationlist.h"

namespace drawingboard {

CanvasScene::CanvasScene(QObject *parent)
	: QGraphicsScene(parent), m_model(nullptr),
	  m_selection(nullptr),
	  m_showAnnotationBorders(false), m_showAnnotations(true),
	  m_showUserMarkers(true), m_showUserNames(true), m_showUserLayers(true), m_showUserAvatars(true),
	  m_showLaserTrails(true)
{
	setItemIndexMethod(NoIndex);

	m_canvasItem = new CanvasItem;
	addItem(m_canvasItem);

	// Timer for on-canvas animations (user pointer fadeout, laser trail flickering and such)
	// Our animation needs are very simple, so we use this instead of QGraphicsScene own
	// animation system.
	auto animationTimer = new QTimer(this);
	connect(animationTimer, &QTimer::timeout, this, &CanvasScene::advanceAnimations);
	animationTimer->setInterval(20);
	animationTimer->start(20);
}

/**
 * This prepares the canvas for new drawing commands.
 * @param myid the context id of the local user
 */
void CanvasScene::initCanvas(canvas::CanvasModel *model)
{
	m_model = model;
	m_canvasItem->setPaintEngine(m_model->paintEngine());

	onSelectionChanged(nullptr);

	connect(m_model->paintEngine(), &canvas::PaintEngine::resized, this, &CanvasScene::handleCanvasResize);
	connect(m_model->paintEngine(), &canvas::PaintEngine::annotationsChanged, this, &CanvasScene::annotationsChanged);
	connect(m_model->paintEngine(), &canvas::PaintEngine::cursorMoved, this, &CanvasScene::userCursorMoved);

	connect(m_model, &canvas::CanvasModel::previewAnnotationRequested, this, &CanvasScene::previewAnnotation);
	connect(m_model, &canvas::CanvasModel::laserTrail, this, &CanvasScene::laserTrail);
	connect(m_model, &canvas::CanvasModel::selectionChanged, this, &CanvasScene::onSelectionChanged);

	const auto items = this->items();
	for(QGraphicsItem *item : items) {
		if(item->type() == AnnotationItem::Type || item->type() == UserMarkerItem::Type || item->type() == LaserTrailItem::Type) {
			delete item;
		}
	}
	m_usermarkers.clear();
	m_activeLaserTrail.clear();

	QList<QRectF> regions;
	regions.append(sceneRect());
	emit changed(regions);
}

void CanvasScene::showCanvas()
{
	if(m_canvasItem)
		m_canvasItem->setVisible(true);
}

void CanvasScene::hideCanvas()
{
	if(m_canvasItem)
		m_canvasItem->setVisible(false);
}

void CanvasScene::canvasViewportChanged(const QPolygonF &viewport)
{
	if(m_canvasItem) {
		m_canvasItem->setViewportBounds(viewport.boundingRect());
	}
}

void CanvasScene::onSelectionChanged(canvas::Selection *selection)
{
	if(m_selection) {
		removeItem(m_selection);
		delete m_selection;
		m_selection = nullptr;
	}
	if(selection) {
		m_selection = new SelectionItem(selection);
		addItem(m_selection);
	}
}

void CanvasScene::showAnnotations(bool show)
{
	if(m_showAnnotations != show) {
		m_showAnnotations = show;
		for(QGraphicsItem *item : items()) {
			if(item->type() == AnnotationItem::Type)
				item->setVisible(show);
		}
	}
}

void CanvasScene::showAnnotationBorders(bool showBorders)
{
	if(m_showAnnotationBorders != showBorders) {
		m_showAnnotationBorders = showBorders;
		for(QGraphicsItem *item : items()) {
			auto *ai = qgraphicsitem_cast<AnnotationItem*>(item);
			if(ai)
				ai->setShowBorder(showBorders);
		}
	}
}

void CanvasScene::handleCanvasResize(int xoffset, int yoffset, const QSize &oldsize)
{
	if(!m_canvasItem)
		return;
	QRectF bounds = m_canvasItem->boundingRect();

	setSceneRect(bounds.adjusted(-MARGIN, -MARGIN, MARGIN, MARGIN));
	emit canvasResized(xoffset, yoffset, oldsize);
}

AnnotationItem *CanvasScene::getAnnotationItem(int id)
{
	for(QGraphicsItem *i : items()) {
		AnnotationItem *item = qgraphicsitem_cast<AnnotationItem*>(i);
		if(item && item->id() == id)
			return item;
	}
	return nullptr;
}

void CanvasScene::setActiveAnnotation(int id)
{
	for(QGraphicsItem *i : items()) {
		AnnotationItem *item = qgraphicsitem_cast<AnnotationItem*>(i);
		if(item)
			item->setHighlight(item->id() == id);
	}
}

void CanvasScene::annotationsChanged(const drawdance::AnnotationList &al) {
	QHash<int, AnnotationItem*> annotationItems;
	for(QGraphicsItem *item : items()) {
		 AnnotationItem *ai = qgraphicsitem_cast<AnnotationItem*>(item);
		 if(ai) {
			 annotationItems.insert(ai->id(), ai);
		 }
	}

	int count = al.count();
	for (int i = 0; i < count; ++i) {
		drawdance::Annotation a = al.at(i);
		int id = a.id();
		QHash<int, AnnotationItem *>::iterator it = annotationItems.find(id);
		AnnotationItem *ai;
		if(it == annotationItems.end()) {
			ai = new AnnotationItem(id);
			ai->setShowBorder(showAnnotationBorders());
			ai->setVisible(m_showAnnotations);
			addItem(ai);
		} else {
			ai = it.value();
			annotationItems.erase(it);
		}
		ai->setText(a.text());
		ai->setGeometry(a.bounds());
		ai->setColor(a.backgroundColor());
		ai->setProtect(a.protect());
		ai->setValign(a.valign());
	}

	for(AnnotationItem *ai : annotationItems) {
		emit annotationDeleted(ai->id());
		delete ai;
	}
}

void CanvasScene::previewAnnotation(int id, const QRect &shape)
{
	auto ai = getAnnotationItem(id);
	if(!ai) {
		ai = new AnnotationItem(id);
		ai->setShowBorder(showAnnotationBorders());
		ai->setVisible(m_showAnnotations);
		addItem(ai);
	}

	ai->setGeometry(shape);
}

/**
 * @brief Advance canvas animations
 *
 * Note. We don't use the scene's built-in animation features since we care about
 * just a few specific animations.
 */
void CanvasScene::advanceAnimations()
{
	const double STEP = 0.02; // time delta in seconds

	const auto items = this->items();
	for(QGraphicsItem *item : items) {
		if(LaserTrailItem *lt = qgraphicsitem_cast<LaserTrailItem*>(item)) {
			if(lt->animationStep(STEP) == false) {
				if(m_activeLaserTrail.value(lt->owner()) == lt)
					m_activeLaserTrail.remove(lt->owner());
				delete item;
			}

		} else if(UserMarkerItem *um = qgraphicsitem_cast<UserMarkerItem*>(item)) {
			um->animationStep(STEP);
		}
	}

	if(m_selection)
		m_selection->marchingAnts(STEP);
}

void CanvasScene::laserTrail(uint8_t userId, int persistence, const QColor &color)
{
	if(persistence == 0) {
		m_activeLaserTrail.remove(userId);
	} else if(m_showLaserTrails) {
		LaserTrailItem *item = new LaserTrailItem(userId, persistence, color);
		m_activeLaserTrail[userId] = item;
		addItem(item);
	}
}

void CanvasScene::userCursorMoved(uint8_t userId, uint16_t layerId, int x, int y)
{
	// User cursor motion is used to update the laser trail, if one exists
	bool isLaserPointer = m_activeLaserTrail.contains(userId);
	if(isLaserPointer) {
		LaserTrailItem *laser = m_activeLaserTrail[userId];
		laser->addPoint(QPointF(x, y));
	}

	if(!m_showUserMarkers)
		return;

	// TODO in some cases (playback, laser pointer) we want to show our cursor as well.
	if(userId == m_model->localUserId())
		return;

	UserMarkerItem *item = m_usermarkers[userId];
	if(!item) {
		const auto user = m_model->userlist()->getUserById(userId);
		item = new UserMarkerItem(userId);
		item->setText(user.name.isEmpty() ? QStringLiteral("#%1").arg(int(userId)) : user.name);
		item->setShowText(m_showUserNames);
		item->setShowSubtext(m_showUserLayers);
		item->setAvatar(user.avatar);
		item->setShowAvatar(m_showUserAvatars);
		item->setTargetPos(x, y, true);
		addItem(item);
		m_usermarkers[userId] = item;
	}

	if(m_showUserLayers)
		item->setSubtext(m_model->layerlist()->layerIndex(layerId).data(canvas::LayerListModel::TitleRole).toString());

	// Smooth the movement by default so that the curser doesn't jump around
	// like crazy when using MyPaint brushes with spread out dabs. If this is
	// a laser being pointed, set the position directly, without interpolation.
	item->setTargetPos(x, y, isLaserPointer);
	item->fadein();
}

void CanvasScene::showUserMarkers(bool show)
{
	if(m_showUserMarkers != show) {
		m_showUserMarkers = show;
		if(!show) {
			for(UserMarkerItem *item : qAsConst(m_usermarkers)) {
				item->hide();
			}
		}
	}
}

void CanvasScene::showUserNames(bool show)
{
	if(m_showUserNames != show) {
		m_showUserNames = show;
		for(UserMarkerItem *item : qAsConst(m_usermarkers))
			item->setShowText(show);
	}
}

void CanvasScene::showUserLayers(bool show)
{
	if(m_showUserLayers != show) {
		m_showUserLayers = show;
		for(UserMarkerItem *item : qAsConst(m_usermarkers))
			item->setShowSubtext(show);
	}
}

void CanvasScene::showUserAvatars(bool show)
{
	if(m_showUserAvatars != show) {
		m_showUserAvatars = show;
		for(UserMarkerItem *item : qAsConst(m_usermarkers))
			item->setShowAvatar(show);
	}
}

void CanvasScene::showLaserTrails(bool show)
{
	m_showLaserTrails = show;
	if(!show) {
		m_activeLaserTrail.clear();
		const auto items = this->items();
		for(auto i : items) {
			if(i->type() == LaserTrailItem::Type)
				delete i;
		}
	}
}

}
