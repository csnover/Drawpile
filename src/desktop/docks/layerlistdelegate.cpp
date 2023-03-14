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

static QRect calcIconRect(const QStyleOptionViewItem &option)
{
	const auto iconSize = getMetric(QStyle::PM_ListViewIconSize, option);
	QRect rect(option.rect);
	rect.setSize({ iconSize, iconSize });
	rect.translate({ 0, (option.rect.height() - iconSize) / 2 });
	return rect;
}

void LayerListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// Cannot really delegate to QItemDelegate at all because it is not possible
	// to override the functions it uses

	auto opt = setOptions(index, option);

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
		if(calcIconRect(option).contains(me->pos())) {
			const auto &layer = index.data(canvas::LayerListModel::ItemRole).value<canvas::LayerListItem>();
			emit toggleVisibility(layer.id, layer.hidden);
			return true;
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
	auto cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

	if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
		cg = QPalette::Inactive;

	const auto fill = option.palette.color(cg, (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text);

	// The hateful Qt CoreGraphics implementation does not work to composite
	// the pixmap directly and will draw #ececec in all of the areas that are
	// supposed to be transparent if the pixmap does not start filled with
	// Qt::transparent for some reason. QTBUG-11142 kinda.
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

void LayerListDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const
{
	auto textRect = rect.adjusted(0, 0, -0, 0);
	QItemDelegate::drawDisplay(painter, option, textRect, text);
}

}
