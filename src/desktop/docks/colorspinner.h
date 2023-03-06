// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef COLORSPINNERDOCK_H
#define COLORSPINNERDOCK_H

#include <QDockWidget>
#include <memory>

class QEvent;

namespace color_widgets {
	class ColorPalette;
}

namespace docks {

class ColorSpinnerDock : public QDockWidget {
	Q_OBJECT
public:
	ColorSpinnerDock(QWidget *parent);
	~ColorSpinnerDock();

public slots:
	void setColor(const QColor& color);
	void setLastUsedColors(const color_widgets::ColorPalette &pal);

signals:
	void colorSelected(const QColor& color);

private slots:
	void updateSettings();

private:
	struct Private;
	std::unique_ptr<Private> d;
};

}

#endif
