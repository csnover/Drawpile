/*
 * Drawpile - a collaborative drawing program.
 *
 * Copyright (C) 2023 askmeaboutloom
 *
 * Drawpile is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drawpile is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "desktop/dialogs/canvasshortcutsdialog.h"
#include "ui_canvasshortcutsdialog.h"
#include "libclient/utils/canvasshortcutsmodel.h"
#include <QPushButton>

namespace dialogs {

struct CanvasShortcutsDialog::Private {
	Ui::CanvasShortcutDialog ui;
	const CanvasShortcuts::Shortcut *s;
	const CanvasShortcutsModel &canvasShortcuts;
};

CanvasShortcutsDialog::CanvasShortcutsDialog(
	const CanvasShortcuts::Shortcut *s,
	const CanvasShortcutsModel &canvasShortcuts, QWidget *parent)
	: QDialog{parent}
	, d{new Private{{}, s, canvasShortcuts}}
{
	d->ui.setupUi(this);

	d->ui.typeCombo->addItem(
		tr("Key Combination"), CanvasShortcuts::KEY_COMBINATION);
	d->ui.typeCombo->addItem(tr("Mouse Button"), CanvasShortcuts::MOUSE_BUTTON);
	d->ui.typeCombo->addItem(tr("Mouse Wheel"), CanvasShortcuts::MOUSE_WHEEL);
	d->ui.typeCombo->addItem(
		tr("Constraint Key"), CanvasShortcuts::CONSTRAINT_KEY_COMBINATION);
	d->ui.typeCombo->setCurrentIndex(0);

	d->ui.actionCombo->addItem(tr("Pan Canvas"), CanvasShortcuts::CANVAS_PAN);
	d->ui.actionCombo->addItem(
		tr("Rotate Canvas"), CanvasShortcuts::CANVAS_ROTATE);
	d->ui.actionCombo->addItem(tr("Zoom Canvas"), CanvasShortcuts::CANVAS_ZOOM);
	d->ui.actionCombo->addItem(tr("Pick Color"), CanvasShortcuts::COLOR_PICK);
	d->ui.actionCombo->addItem(tr("Pick Layer"), CanvasShortcuts::LAYER_PICK);
	d->ui.actionCombo->addItem(
		tr("Change Brush Size"), CanvasShortcuts::TOOL_ADJUST);
	d->ui.actionCombo->setCurrentIndex(0);

	d->ui.constraintsCombo->addItem(
		tr("Constrain Tool"), CanvasShortcuts::TOOL_CONSTRAINT1);
	d->ui.constraintsCombo->addItem(
		tr("Center Tool"), CanvasShortcuts::TOOL_CONSTRAINT2);
	d->ui.constraintsCombo->addItem(
		tr("Constrain and Center Tool"),
		CanvasShortcuts::TOOL_CONSTRAINT1 | CanvasShortcuts::TOOL_CONSTRAINT2);
	d->ui.constraintsCombo->setCurrentIndex(0);

	if(s) {
		for(int i = 0; i < d->ui.typeCombo->count(); ++i) {
			if(d->ui.typeCombo->itemData(i).toUInt() == s->type) {
				d->ui.typeCombo->setCurrentIndex(i);
				break;
			}
		}

		d->ui.shortcutEdit->setMods(s->mods);
		d->ui.shortcutEdit->setKeys(s->keys);
		d->ui.shortcutEdit->setButton(s->button);

		for(int i = 0; i < d->ui.actionCombo->count(); ++i) {
			if(d->ui.actionCombo->itemData(i).toUInt() == s->action) {
				d->ui.actionCombo->setCurrentIndex(i);
				break;
			}
		}

		d->ui.invertedCheckBox->setChecked(
			s->flags & CanvasShortcuts::INVERTED);
		d->ui.swapAxesCheckBox->setChecked(
			s->flags & CanvasShortcuts::SWAP_AXES);

		unsigned int constraints = s->flags & CanvasShortcuts::CONSTRAINT_MASK;
		for(int i = 0; i < d->ui.constraintsCombo->count(); ++i) {
			if(d->ui.constraintsCombo->itemData(i).toUInt() == constraints) {
				d->ui.constraintsCombo->setCurrentIndex(i);
				break;
			}
		}
	}

	connect(
		d->ui.typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[this](int) {
			updateType();
			updateResult();
		});
	connect(
		d->ui.shortcutEdit, &widgets::CanvasShortcutEdit::shortcutChanged, this,
		&CanvasShortcutsDialog::updateResult);
	connect(
		d->ui.actionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[this](int) {
			updateAction();
			updateResult();
		});

	updateType();
	updateAction();
	updateResult();
}

CanvasShortcutsDialog::~CanvasShortcutsDialog()
{
	delete d;
}

CanvasShortcuts::Shortcut CanvasShortcutsDialog::shortcut() const
{
	unsigned int type = d->ui.typeCombo->currentData().toUInt();
	unsigned int action;
	unsigned int flags;
	if(type == CanvasShortcuts::CONSTRAINT_KEY_COMBINATION) {
		action = CanvasShortcuts::CONSTRAINT;
		flags = d->ui.constraintsCombo->currentData().toUInt();
	} else {
		action = d->ui.actionCombo->currentData().toUInt();
		flags = CanvasShortcuts::NORMAL;
		if(d->ui.invertedCheckBox->isChecked()) {
			flags |= CanvasShortcuts::INVERTED;
		}
		if(d->ui.swapAxesCheckBox->isChecked()) {
			flags |= CanvasShortcuts::SWAP_AXES;
		}
	}
	return {
		CanvasShortcuts::Type(type),	 d->ui.shortcutEdit->mods(),
		d->ui.shortcutEdit->keys(),		 d->ui.shortcutEdit->button(),
		CanvasShortcuts::Action(action), flags,
	};
}

void CanvasShortcutsDialog::updateType()
{
	unsigned int type = d->ui.typeCombo->currentData().toUInt();
	d->ui.shortcutEdit->setType(type);

	QString typeDescription;
	bool showAction;
	bool showConstraints;
	switch(type) {
	case CanvasShortcuts::Type::KEY_COMBINATION:
		typeDescription = tr("A regular key combination on the canvas without "
							 "further mouse or pen inputs. Example: holding "
							 "Space to pan, without having to click as well.");
		showAction = true;
		showConstraints = false;
		break;
	case CanvasShortcuts::Type::MOUSE_BUTTON:
		typeDescription =
			tr("Pressing a mouse or pen button, optionally while also holding "
			   "down keys. Putting the pen down is like a left click. Example: "
			   "holding space and pressing left click to pan.");
		showAction = true;
		showConstraints = false;
		break;
	case CanvasShortcuts::Type::MOUSE_WHEEL:
		typeDescription =
			tr("Turning the mouse wheel or some input device that acts like "
			   "one, optionally while also holding down keys. Example: "
			   "scrolling to zoom the canvas.");
		showAction = true;
		showConstraints = false;
		break;
	case CanvasShortcuts::Type::CONSTRAINT_KEY_COMBINATION:
		typeDescription =
			tr("Keys to hold down to make rectangle, line or selection tools "
			   "behave differently. Constrain means to e.g. keep the aspect "
			   "ratio, center means to e.g. center shapes around the origin.");
		showAction = false;
		showConstraints = true;
		break;
	default:
		typeDescription = tr("Unknown type %1.").arg(type);
		showAction = false;
		showConstraints = false;
		break;
	}
	d->ui.typeDescription->setText(typeDescription);
	d->ui.shortcutGroup->setVisible(showAction || showConstraints);
	d->ui.actionGroup->setVisible(showAction);
	d->ui.constraintsGroup->setVisible(showConstraints);
}

void CanvasShortcutsDialog::updateAction()
{
	unsigned int action = d->ui.actionCombo->currentData().toUInt();
	bool showModifiers;
	switch(action) {
	case CanvasShortcuts::CANVAS_PAN:
	case CanvasShortcuts::CANVAS_ROTATE:
	case CanvasShortcuts::CANVAS_ZOOM:
	case CanvasShortcuts::TOOL_ADJUST:
		showModifiers = true;
		break;
	default:
		showModifiers = false;
		break;
	}
	d->ui.modifiersWrapper->setVisible(showModifiers);
}

void CanvasShortcutsDialog::updateResult()
{
	const CanvasShortcuts::Shortcut s = shortcut();
	const CanvasShortcuts::Shortcut *conflict =
		d->canvasShortcuts.searchConflict(s, d->s);
	bool valid = s.isValid();
	bool unmodifiedLeftClick = s.isUnmodifiedClick(Qt::LeftButton);

	QString conflictDescription;
	if(conflict) {
		conflictDescription =
			tr("<b>Conflict:</b> the existing shortcut for '%1' will be "
			   "overwritten if you proceed.")
				.arg(CanvasShortcutsModel::shortcutTitle(conflict, true));
	} else if(unmodifiedLeftClick) {
		conflictDescription =
			tr("You can't assign a shortcut to a Left Click without any "
			   "keys since that would interfere with drawing.");
	} else if(!valid) {
		conflictDescription = tr("Assign a shortcut to proceed.");
	} else {
		conflictDescription = QString{};
	}
	d->ui.conflictDescription->setText(conflictDescription);

	QPushButton *okButton = d->ui.buttonBox->button(QDialogButtonBox::Ok);
	if(okButton) {
		okButton->setEnabled(valid && !unmodifiedLeftClick);
	}
}

}
