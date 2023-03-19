// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Drawpile contributors

#ifndef DESKTOP_WIDGETS_SPANAWARETREEVIEW_H
#define DESKTOP_WIDGETS_SPANAWARETREEVIEW_H

#include <QList>
#include <QTreeView>

#ifdef DESIGNER_PLUGIN
#include <QtUiPlugin/QDesignerExportWidget>
#else
#define QDESIGNER_WIDGET_EXPORT
#endif

class QModelIndex;

namespace widgets {

class QDESIGNER_WIDGET_EXPORT SpanAwareTreeView final : public QTreeView {
public:
	using QTreeView::QTreeView;
	void reset() override;

protected:
	void rowsInserted(const QModelIndex &parent, int start, int end) override;

private:
	void setAllSpans(const QModelIndex &index);
};

} // namespace widgets
#endif
