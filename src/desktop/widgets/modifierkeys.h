// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef MODIFIERKEYS_H
#define MODIFIERKEYS_H

#include <QWidget>

#ifdef DESIGNER_PLUGIN
#include <QtUiPlugin/QDesignerExportWidget>
#else
#define QDESIGNER_WIDGET_EXPORT
#endif

class QAbstractButton;

namespace widgets {

class QDESIGNER_WIDGET_EXPORT ModifierKeys final : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(Qt::KeyboardModifiers modifiers READ modifiers WRITE setModifiers)
public:
	explicit ModifierKeys(QWidget *parent = nullptr);

	Qt::KeyboardModifiers modifiers() const;

signals:

public slots:
	void setModifiers(Qt::KeyboardModifiers modifiers);

private:
	QAbstractButton *m_buttons[4];
};

}

#endif // MODIFIERKEYS_H
