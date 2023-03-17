// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef COLORSLIDERDOCK_H
#define COLORSLIDERDOCK_H

#include <QDockWidget>
#include <memory>

namespace color_widgets {
	class ColorPalette;
}

namespace docks {

class ColorSliderDock final : public QDockWidget {
	Q_OBJECT
public:
	ColorSliderDock(QWidget *parent);
	~ColorSliderDock() override;

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
	const std::unique_ptr<Private> d;
};

}

#endif
