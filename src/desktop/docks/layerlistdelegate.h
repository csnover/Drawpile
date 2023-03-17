// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef LAYERLISTMODEL_H
#define LAYERLISTMODEL_H

#include <QAbstractListModel>
#include <QIcon>
#include <QItemDelegate>
#include <QModelIndex>
#include <QRect>
#include <memory>

class QAbstractItemModel;

namespace canvas {
	class LayerListModel;
}

namespace widgets {
	class PopupMessage;
}

namespace docks {

/**
 * \brief A custom item delegate for displaying layer names and editing layer settings.
 */
class LayerListDelegate final : public QItemDelegate {
	Q_OBJECT

	enum Region {
		None,
		Decoration,
		Text,
		Opacity,
	};
public:
	LayerListDelegate(QObject *parent = nullptr);

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
			const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option,
			const QModelIndex &index) const override;

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
	bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;
	bool eventFilter(QObject *object, QEvent *event) override;

signals:
	void toggleVisibility(int layerId, bool visible);
	void editProperties(const QModelIndex &index);
	void openEditor(const QModelIndex &index);

protected:
	void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const override;
	void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const override;
	void drawOpacity(QPainter *painter, const QStyleOptionViewItem &option, qreal opacity) const;
	void drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const;

private:
	QStyleOptionViewItem setOptions(const QModelIndex &index, const QStyleOptionViewItem &option) const;
	QRect regionRect(const QStyleOptionViewItem &option, Region region, bool forLayout = true) const;
	Region region(const QStyleOptionViewItem &option, const QPoint &pos) const;

	struct {
		Region region = None;
		QPoint controlPoint;
		QPoint popupAt;
		float value = 0;
		QAbstractItemModel *model = nullptr;
		QModelIndex index;
		std::unique_ptr<widgets::PopupMessage> popup;
	} m_editor;
};

}

#endif
