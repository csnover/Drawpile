// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef LAYERLISTMODEL_H
#define LAYERLISTMODEL_H

#include <QAbstractListModel>
#include <QIcon>
#include <QItemDelegate>
#include <QRect>

namespace docks {

/**
 * \brief A custom item delegate for displaying layer names and editing layer settings.
 */
class LayerListDelegate : public QItemDelegate {
	Q_OBJECT

	enum Region {
		None,
		Decoration,
		Text,
		Opacity,
	};
public:
	LayerListDelegate(QObject *parent=nullptr);

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
			const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option,
			const QModelIndex &index) const override;

	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

signals:
	void toggleVisibility(int layerId, bool visible);
	void editProperties(QModelIndex index);

protected:
	void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const override;
	void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const override;
	void drawOpacity(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, qreal opacity) const;
	void drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const;

	Region region(const QPoint &pos) const;

private:
	mutable QRect m_decorationRect;
	mutable QRect m_displayRect;
	mutable QRect m_opacityRect;
	bool m_draggingOpacity = false;
};

}

#endif
