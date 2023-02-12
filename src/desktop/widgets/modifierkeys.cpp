// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/widgets/modifierkeys.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QtGlobal>

namespace widgets {

static inline constexpr auto MAC_OR_PC(const char * const mac, const char * const pc) {
#ifdef Q_OS_MACOS
	Q_UNUSED(pc);
	return mac;
#else
	Q_UNUSED(mac);
	return pc;
#endif
}

ModifierKeys::ModifierKeys(QWidget *parent)
	: QWidget(parent)
{
	auto *layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	m_buttons[0] = new QCheckBox(MAC_OR_PC("⇧", "Shift"), this);
	m_buttons[1] = new QCheckBox(MAC_OR_PC("⌘", "Ctrl"), this);
	m_buttons[2] = new QCheckBox(MAC_OR_PC("⎇", "Alt"), this);
	m_buttons[3] = new QCheckBox(MAC_OR_PC("Ctrl", "Meta"), this);

	for(int i=0;i<4;++i)
		layout->addWidget(m_buttons[i]);
}

Qt::KeyboardModifiers ModifierKeys::modifiers() const
{
	return
		(m_buttons[0]->isChecked() ? Qt::ShiftModifier : Qt::NoModifier)
		| (m_buttons[1]->isChecked() ? Qt::ControlModifier : Qt::NoModifier)
		| (m_buttons[2]->isChecked() ? Qt::AltModifier : Qt::NoModifier)
		| (m_buttons[3]->isChecked() ? Qt::MetaModifier : Qt::NoModifier);
}

void ModifierKeys::setModifiers(Qt::KeyboardModifiers mods)
{
	m_buttons[0]->setChecked(mods.testFlag(Qt::ShiftModifier));
	m_buttons[1]->setChecked(mods.testFlag(Qt::ControlModifier));
	m_buttons[2]->setChecked(mods.testFlag(Qt::AltModifier));
	m_buttons[3]->setChecked(mods.testFlag(Qt::MetaModifier));
}

}
