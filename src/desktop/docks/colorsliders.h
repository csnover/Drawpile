// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef COLORSLIDERDOCK_H
#define COLORSLIDERDOCK_H

#include <QDockWidget>

namespace color_widgets {
	class ColorPalette;
}

namespace docks {

class ColorSliderDock : public QDockWidget {
	Q_OBJECT
public:
	ColorSliderDock(const QString& title, QWidget *parent);
	~ColorSliderDock();

public slots:
	void setColor(const QColor& color);
	void setLastUsedColors(const color_widgets::ColorPalette &pal);

private slots:
	void updateFromRgbSliders();
	void updateFromRgbSpinbox();
	void updateFromHsvSliders();
	void updateFromHsvSpinbox();

signals:
	void colorSelected(const QColor& color);

private:
	struct Private;
	Private *d;
};

}

#endif
