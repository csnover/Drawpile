// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/docks/layerlistdelegate.h"
#include "libclient/canvas/layerlist.h"

#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPainter>
#include <QTreeView>

namespace docks {

LayerListDelegate::LayerListDelegate(QObject *parent)
	: QItemDelegate(parent)
{
}

static inline auto &getStyle(const QStyleOptionViewItem &option)
{
	const auto *style = option.widget ? option.widget->style() : QApplication::style();
	Q_ASSERT(style);
	return *style;
}

static inline auto getMetric(QStyle::PixelMetric pm, const QStyleOptionViewItem &option)
{
	return getStyle(option).pixelMetric(pm, &option, option.widget);
}

void LayerListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	auto opt = setOptions(index, option);

	painter->save();
	painter->setRenderHint(QPainter::SmoothPixmapTransform);

	// It is not possible to just set the flags of the model item to be disabled
	// because then it will be unselectable, but it is desired to draw it in the
	// special state
	if (index.data(canvas::LayerListModel::IsLockedRole).toBool()) {
		opt.state &= ~QStyle::State_Enabled;

		// The parts to the left of the item in a tree have to be redrawn
		// explicitly, otherwise the colour of the background and the branch
		// control will be wrong
		if (auto *widget = qobject_cast<const QTreeView *>(option.widget)) {
			auto rect = opt.rect;
			rect.setLeft(0);
			painter->fillRect(rect, opt.palette.color(QPalette::Disabled,
				opt.state & QStyle::State_Selected
				? QPalette::Highlight
				: QPalette::Window
			));

			auto treeOpt = opt;
			const auto expanded = widget->isExpanded(index);
			const auto children = index.model()->hasChildren(index);
			const auto moreSiblings = widget->indexBelow(index).isValid();

			treeOpt.state
				= QStyle::State_Item
				| (moreSiblings ? QStyle::State_Sibling : QStyle::State_None)
				| (children ? QStyle::State_Children : QStyle::State_None)
				| (expanded ? QStyle::State_Open : QStyle::State_None);

			const auto width = getMetric(QStyle::PM_TreeViewIndentation, treeOpt);
			treeOpt.rect.translate(-width, 0);
			treeOpt.rect.setWidth(width);
			getStyle(treeOpt).drawPrimitive(QStyle::PE_IndicatorBranch, &treeOpt, painter, widget);
		}
	}

	QItemDelegate::paint(painter, opt, index);

	const auto margin = getMetric(QStyle::PM_ButtonMargin, opt);
	auto rect = m_decorationRect;
	rect.moveTo(opt.rect.x() + opt.rect.width() - rect.width() - margin, rect.y());
	drawOpacity(painter, opt, rect, index.data(canvas::LayerListModel::OpacityRole).toFloat());

	qDebug() << m_decorationRect << m_displayRect << m_opacityRect;

	painter->restore();
}

bool LayerListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	QEvent::Type type = event->type();
	switch(type) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonDblClick: {
		const auto *me = static_cast<QMouseEvent *>(event);
		if(me->button() != Qt::LeftButton) {
			break;
		}
		const auto rgn = region(me->pos());
		if(rgn == Region::Decoration) {
			const auto &layer = index.data(canvas::LayerListModel::ItemRole).value<canvas::LayerListItem>();
			emit toggleVisibility(layer.id, layer.hidden);
			return true;
		} else if(rgn == Region::Opacity) {
			qDebug() << "in opacity";
		} else if(type == QEvent::MouseButtonDblClick) {
			emit editProperties(index);
			return true;
		}
		break;
	}
	default: {}
	}

	return QItemDelegate::editorEvent(event, model, option, index);
}

QSize LayerListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const auto iconHeight = getMetric(QStyle::PM_ListViewIconSize, option) + 2 * getMetric(QStyle::PM_ButtonMargin, option);
	return QItemDelegate::sizeHint(option, index).expandedTo(QSize(0, iconHeight));
}

void LayerListDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
	drawIcon(painter, option, rect, pixmap);
	m_decorationRect = rect;
}

void LayerListDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
	const auto iconWidth = getMetric(QStyle::PM_ListViewIconSize, option) + 2 * getMetric(QStyle::PM_ButtonMargin, option);
	const auto textRect = rect.adjusted(0, 0, -iconWidth, 0);
	QItemDelegate::drawDisplay(painter, option, textRect, text);

	m_displayRect = textRect;
}

void LayerListDelegate::drawOpacity(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, qreal opacity) const
{
	const auto icon = QIcon::fromTheme("view-visible");
	const auto pixmap = icon.pixmap(icon.actualSize(option.decorationSize));

	painter->save();
	painter->setOpacity(opacity);
	drawIcon(painter, option, rect, pixmap);
	painter->restore();
	m_opacityRect = rect;
}

void LayerListDelegate::drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
	auto cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

	if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
		cg = QPalette::Inactive;

	const auto fill = option.palette.color(cg, (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text);

	// An unknown issue in Qt on macOS causes the OS default window background
	// colour to be drawn in the composited area if not rendered to a QPixmap
	// that is initialised with a Qt::transparent fill for some reason.
	// QTBUG-111936
	auto icon = QPixmap(pixmap);
	icon.fill(Qt::transparent);
	{
		QPainter copier(&icon);
		copier.fillRect(icon.rect(), fill);
		copier.setCompositionMode(QPainter::CompositionMode_DestinationIn);
		copier.drawPixmap(0, 0, pixmap);
	}

	painter->drawPixmap(rect, icon);
}

LayerListDelegate::Region LayerListDelegate::region(const QPoint &pos) const
{
	if (m_decorationRect.contains(pos)) {
		return Region::Decoration;
	} else if (m_displayRect.contains(pos)) {
		return Region::Text;
	} else if (m_opacityRect.contains(pos)) {
		return Region::Opacity;
	} else {
		return Region::None;
	}
}

}
