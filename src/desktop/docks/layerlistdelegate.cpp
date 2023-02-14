// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/docks/layerlistdelegate.h"
#include "libclient/canvas/layerlist.h"
#include "libclient/utils/icon.h"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QLineEdit>

namespace docks {

LayerListDelegate::LayerListDelegate(QObject *parent)
	: QItemDelegate(parent),
	  m_visibleIcon(icon::fromTheme("layer-visible-on")),
	  m_groupIcon(icon::fromTheme("folder")),
	  m_censoredIcon(QIcon(":/icons/censored.svg")),
	  m_hiddenIcon(icon::fromTheme("layer-visible-off"))
{
}

static inline auto &getStyle(const QStyleOptionViewItem &option)
{
	const auto *style = option.widget ? option.widget->style() : QApplication::style();
	Q_ASSERT(style);
	return *style;
}

static QRect calcIconRect(const QStyleOptionViewItem &option)
{
	const auto &style = getStyle(option);
	const auto iconSize = style.pixelMetric(QStyle::PM_ButtonIconSize);
	QRect rect(option.rect);
	rect.setSize({ iconSize, iconSize });
	rect.translate({ 0, (option.rect.height() - iconSize) / 2 });
	return rect;
}

void LayerListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const canvas::LayerListItem &layer = index.data().value<canvas::LayerListItem>();

	QStyleOptionViewItem opt = setOptions(index, option);
	if(index.data(canvas::LayerListModel::IsDefaultRole).toBool()) {
		opt.font.setUnderline(true);
	}
	if(index.data(canvas::LayerListModel::IsLockedRole).toBool()) {
		opt.state &= ~QStyle::State_Enabled;
	}

	const auto iconRect = calcIconRect(opt);
	auto textRect = option.rect;
	textRect.setLeft(calcIconRect(option).right() + getStyle(option).pixelMetric(QStyle::PM_CheckBoxLabelSpacing));

	painter->save();

	drawBackground(painter, option, index);
	drawOpacityGlyph(iconRect, painter, layer.opacity, layer.hidden, layer.censored, layer.group);
	drawDisplay(painter, opt, textRect,layer.title);

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
		if(calcIconRect(option).contains(me->pos())) {
			const auto &layer = index.data().value<canvas::LayerListItem>();
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
	const auto &style = getStyle(option);
	const auto iconHeight = style.pixelMetric(QStyle::PM_ButtonIconSize)
		+ style.pixelMetric(QStyle::PM_ButtonMargin) * 2;
	const auto minHeight = qMax(QFontMetrics(option.font).height(), iconHeight);
	QSize size = QItemDelegate::sizeHint(option, index);
	if(size.height() < minHeight)
		size.setHeight(minHeight);
	return size;
}

void LayerListDelegate::drawOpacityGlyph(const QRect &r, QPainter *painter, float value, bool hidden, bool censored, bool group) const
{
	if(hidden) {
		m_hiddenIcon.paint(painter, r);
	} else {
		painter->save();
		painter->setOpacity(value);
		if(censored)
			m_censoredIcon.paint(painter, r);
		else if(group)
			m_groupIcon.paint(painter, r);
		else
			m_visibleIcon.paint(painter, r);
		painter->restore();
	}
}

}
