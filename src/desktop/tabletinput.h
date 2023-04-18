// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TABLETINPUT_H
#define TABLETINPUT_H

#include <QString>

class DrawpileApp;

namespace tabletinput {

void init(DrawpileApp &app);

QString current();

}

#endif
