// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef TOOLSETTINGS_ANNOTATION_H
#define TOOLSETTINGS_ANNOTATION_H

#include "desktop/toolwidgets/toolsettings.h"

#include <memory>

class Ui_TextSettings;
class QTimer;
class QAction;
class QActionGroup;
class QLabel;

namespace drawingboard {
	class CanvasScene;
}

namespace tools {

/**
 * @brief Settings for the annotation tool
 *
 * The annotation tool is special because it is used to manipulate
 * annotation objects rather than pixel data.
 */
class AnnotationSettings : public ToolSettings {
	Q_OBJECT
public:
	AnnotationSettings(ToolController *ctrl, QObject *parent=nullptr);
	~AnnotationSettings();

	QString toolType() const override { return QStringLiteral("annotation"); }

	/**
	 * @brief Get the ID of the currently selected annotation
	 * @return ID or 0 if none selected
	 */
	uint16_t selected() const { return m_selectionId; }

	/**
	 * @brief Focus content editing box and set cursor position
	 * @param cursorPos cursor position
	 */
	void setFocusAt(int cursorPos);

	void setForeground(const QColor &) override {}
	int getSize() const override { return 0; }
	bool getSubpixelMode() const override { return false; }

	void setScene(drawingboard::CanvasScene *scene) { m_scene = scene; }
	QWidget *getHeaderWidget() override;

public slots:
	//! Set the currently selected annotation item
	void setSelectionId(uint16_t id);

	//! Focus the content editing box
	void setFocus();

private slots:
	void changeAlignment(const QAction *action);
	void toggleBold(bool bold);
	void toggleStrikethrough(bool strike);
	void updateStyleButtons();
	void setEditorBackgroundColor(const QColor &color);

	void applyChanges();
	void saveChanges();
	void removeAnnotation();
	void bake();

	void updateFontIfUniform();

protected:
	QWidget *createUiWidget(QWidget *parent) override;

private:
	void resetContentFont(bool resetFamily, bool resetSize, bool resetColor);
	void setUiEnabled(bool enabled);

	std::unique_ptr<Ui_TextSettings> m_ui;
	QWidget *m_headerWidget;
	QActionGroup *m_editActions;
	QAction *m_protectedAction;
	QLabel *m_creatorLabel;

	uint16_t m_selectionId;

	bool m_noupdate;
	QTimer *m_updatetimer;
	drawingboard::CanvasScene *m_scene;
};

}

#endif
