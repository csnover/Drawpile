// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef COLORPALETTEDOCK_H
#define COLORPALETTEDOCK_H

#include <QDockWidget>

namespace color_widgets {
	class ColorPalette;
}

namespace docks {

class ColorPaletteDock : public QDockWidget {
	Q_OBJECT
public:
	ColorPaletteDock(QWidget *parent);
	~ColorPaletteDock();

public slots:
	void setColor(const QColor& color);

signals:
	void colorSelected(const QColor& color);

private slots:
	void paletteChanged(int index);
	void addPalette();
	void copyPalette();
	void deletePalette();
	void renamePalette();
	void paletteRenamed();
	void setPaletteReadonly(bool readonly);
	void exportPalette();
	void importPalette();

	void paletteClicked(int index);
	void paletteDoubleClicked(int index);
	void paletteRightClicked(int index);

private:
	struct Private;
	std::unique_ptr<Private> d;
};

int findPaletteColor(const color_widgets::ColorPalette &pal, const QColor &color);

}

#endif
