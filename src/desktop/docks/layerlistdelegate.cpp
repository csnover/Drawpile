// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/docks/layerlistdelegate.h"
#include "desktop/widgets/popupmessage.h"
#include "libclient/canvas/layerlist.h"
#include "desktop/utils/qtguicompat.h"

#include <QApplication>
#include <QIcon>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPainter>
#include <QTreeView>
#if QT_CONFIG(tooltip)
#include <QToolTip>
#endif

namespace {
	using Model = canvas::LayerListModel;
}

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

static inline auto getRect(QStyle::SubElement se, const QStyleOptionViewItem &option)
{
	return getStyle(option).subElementRect(se, &option, option.widget);
}

void LayerListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	auto opt = setOptions(index, option);

	painter->save();
	painter->setRenderHint(QPainter::SmoothPixmapTransform);

	// It is not possible to just set the flags of the model item to be disabled
	// because then it will be unselectable, but it is desired to draw it in the
	// special state
	if (index.data(Model::IsLockedRole).toBool()) {
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
	drawOpacity(painter, opt, index.data(Model::OpacityRole).toFloat());

	painter->restore();
}

QWidget *LayerListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
	if (m_editor.region == Text) {
		return new QLineEdit(parent);
	}

	return nullptr;
}

void LayerListDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	editor->setGeometry(regionRect(setOptions(index, option), m_editor.region, true));
}

bool LayerListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	const auto type = event->type();
	switch (type) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonDblClick: {
		auto *me = static_cast<QMouseEvent *>(event);
		if (me->button() != Qt::LeftButton) {
			break;
		}
		const auto pos = me->pos();
		switch (region(setOptions(index, option), pos)) {
		case Decoration: {
			const auto &layer = index.data(Model::ItemRole).value<canvas::LayerListItem>();
			emit toggleVisibility(layer.id, layer.hidden);
			event->accept();
			return true;
		}
		case Opacity:
			// Must install an event filter to handle the mouse events because
			// the QTreeView does not understand that we are grabbing the mouse so
			// does not know to keep sending events to the item delegate
			qApp->installEventFilter(this);
			m_editor.region = Opacity;
			m_editor.controlPoint = compat::globalPos(*me);
			m_editor.popupAt = option.widget->mapToGlobal(regionRect(option, Opacity, true).topLeft());
			m_editor.value = index.data(Model::OpacityRole).toFloat();
			m_editor.model = model;
			m_editor.index = index;
			m_editor.popup.reset(new widgets::PopupMessage(1, option.widget));
			event->accept();
			return true;
		case Text:
			if (type == QEvent::MouseButtonDblClick) {
				m_editor.region = Text;
				emit openEditor(index);
				event->accept();
				return true;
			}
			break;
		default:
			if (type == QEvent::MouseButtonDblClick) {
				emit editProperties(index);
				event->accept();
				return true;
			}
		}
		break;
	}
	default: {}
	}

	return QItemDelegate::editorEvent(event, model, option, index);
}

bool LayerListDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	if (!index.isValid())
		return false;

	switch (event->type()) {
#if QT_CONFIG(tooltip)
	case QEvent::ToolTip: {
		QString tooltip;
		switch(region(setOptions(index, option), event->pos())) {
		case Opacity:
			tooltip = tr("%1% opacity").arg(
				index.data(Model::OpacityRole).toFloat() * 100.f,
				0, 'f', 0
			);
			break;
		case Decoration:
			tooltip = index.data(Model::DecorationToolTipRole).toString();
			break;
		default:
			tooltip = index.data(Qt::ToolTipRole).toString();
		}

		if (!tooltip.isEmpty()) {
			QToolTip::showText(event->globalPos(), tooltip, view->viewport(), option.rect);
			event->setAccepted(true);
		}
		break;
	}
#endif
	default: {}
	}

	return event->isAccepted();
}

bool LayerListDelegate::eventFilter(QObject *object, QEvent *event)
{
	if (m_editor.region != Opacity) {
		return QItemDelegate::eventFilter(object, event);
	}

	if (!m_editor.index.isValid() || (event->type() == QEvent::MouseButtonRelease && static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton)) {
		qApp->removeEventFilter(this);
		m_editor.region = None;
		m_editor.popup.reset(nullptr);
		event->accept();
		return true;
	} else if (event->type() == QEvent::MouseMove) {
		auto *me = static_cast<QMouseEvent *>(event);
		const auto delta = compat::globalPos(*me) - m_editor.controlPoint;
		// TODO: Do y-amplification
		const auto dx = delta.x() * 0.01;

		const auto opacity = qBound(0., m_editor.value + dx, 1.);
		m_editor.popup->showMessage(
			m_editor.popupAt,
			tr("%1%").arg(opacity * 100, 0, 'f', 0)
		);
		m_editor.model->setData(m_editor.index, opacity, Model::OpacityRole);
		m_editor.model->submit();
		event->accept();
		return true;
	}

	return false;
}

QSize LayerListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const auto iconHeight = getMetric(QStyle::PM_ListViewIconSize, option) + 2 * getMetric(QStyle::PM_ButtonMargin, option);
	return QItemDelegate::sizeHint(option, index).expandedTo(QSize(0, iconHeight));
}

void LayerListDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
	drawIcon(painter, option, rect, pixmap);
}

void LayerListDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &, const QString &text) const
{
	const auto rect = regionRect(option, Text, true);
	QItemDelegate::drawDisplay(painter, option, rect, text);
}

void LayerListDelegate::drawOpacity(QPainter *painter, const QStyleOptionViewItem &option, qreal opacity) const
{
	const auto icon = QIcon::fromTheme("view-visible");
	const auto pixmap = icon.pixmap(icon.actualSize(option.decorationSize));

	painter->save();
	painter->setOpacity(0.25 + opacity * 0.75);
	drawIcon(painter, option, regionRect(option, Opacity, true), pixmap);
	painter->restore();
}

void LayerListDelegate::drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const
{
	auto cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

	if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
		cg = QPalette::Inactive;

	const auto fill = option.palette.color(cg, (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text);

	// Paints go directly to the window surface so tinting compositing needs
	// to be done separately. QTBUG-111936
	auto icon = QPixmap(pixmap);
	icon.fill(Qt::transparent);
	{
		QPainter tinter(&icon);
		tinter.fillRect(icon.rect(), fill);
		tinter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
		tinter.drawPixmap(0, 0, pixmap);
	}

	painter->drawPixmap(rect, icon);
}

QStyleOptionViewItem LayerListDelegate::setOptions(const QModelIndex &index, const QStyleOptionViewItem &option) const
{
	auto opt = QItemDelegate::setOptions(index, option);

	// These flags are only set by QStyledItemDelegate for some reason,
	// but are required for QStyle::subElementRect to give a correct result
	const auto text = index.data(Qt::DisplayRole);
	if (text.isValid()) {
		opt.features |= QStyleOptionViewItem::HasDisplay;
		// This is required to calculate the bounding box of the text without
		// also having to retain and pass the index around, and again is only
		// set by QStyledItemDelegate for some reason
		opt.text = text.toString();
	}
	const auto icon = index.data(Qt::DecorationRole);
	if (icon.isValid()) {
		opt.features |= QStyleOptionViewItem::HasDecoration;
		// This is again is only set by QStyledItemDelegate for some reason;
		// it is not strictly needed but let’s just avoid any subtle bugs
		// later caused by it not existing
		opt.icon = icon.value<QIcon>();
	}

	return opt;
}

QRect LayerListDelegate::regionRect(const QStyleOptionViewItem &option, Region region, bool forLayout) const
{
	const auto decorationRect = getRect(QStyle::SE_ItemViewItemDecoration, option);

	switch (region) {
	case Text: {
		const auto rtl = (option.direction == Qt::RightToLeft);
		const auto textMargin = getMetric(QStyle::PM_FocusFrameHMargin, option);
		const auto endMargin = getMetric(QStyle::PM_ToolBarItemSpacing, option);
		const auto dx = endMargin + decorationRect.width();
		const auto dx1 = rtl ? dx : 0;
		const auto dx2 = rtl ? 0 : dx;

		auto textRect = getRect(QStyle::SE_ItemViewItemText, option)
			.adjusted(textMargin + dx1, 0, -textMargin - dx2, 0);

		if (forLayout) {
			return textRect;
		} else {
			// QItemDelegate::drawDisplay in at least Qt 6.4.2 “removes”
			// the margin with an +1, which is probably an off-by-one error;
			// reproduce that to get the correct bounding box
			textRect.adjust(textMargin + 1, 0, -textMargin - 1, 0);

			const auto layoutSize = QSize(textRect.width(), option.fontMetrics.height());
			const auto layoutRect = QStyle::alignedRect(option.direction, option.displayAlignment, layoutSize, textRect);
			const auto text = option.fontMetrics.elidedText(option.text, option.textElideMode, layoutRect.width());
			return option.fontMetrics.boundingRect(text)
				.translated(
					layoutRect.topLeft()
					+ QPoint(0, option.fontMetrics.ascent())
				);
		}
	}
	case Decoration:
		return decorationRect;
	case Opacity: {
		const auto textRect = regionRect(option, Text, true);
		const auto textMargin = getMetric(QStyle::PM_FocusFrameHMargin, option) * 2;
		const auto dir = option.direction == Qt::RightToLeft ? -1 : 1;
		return decorationRect.translated(
			(decorationRect.width() + textRect.width() + textMargin) * dir,
			0
		);
	}
	default: Q_UNREACHABLE();
	}
}

LayerListDelegate::Region LayerListDelegate::region(const QStyleOptionViewItem &option, const QPoint &pos) const
{
	if (regionRect(option, Decoration, false).contains(pos)) {
		return Decoration;
	} else if (regionRect(option, Text, false).contains(pos)) {
		return Text;
	} else if (regionRect(option, Opacity, false).contains(pos)) {
		return Opacity;
	} else {
		return None;
	}
}

}
