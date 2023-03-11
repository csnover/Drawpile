// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/widgets/tablettest.h"
#include "desktop/utils/qtguicompat.h"

#include <QPaintEvent>
#include <QPainter>

namespace widgets {

TabletTester::TabletTester(QWidget *parent)
	: QWidget(parent), m_mouseDown(false), m_tabletDown(false)
{
}

void TabletTester::clear()
{
	m_mousePath.clear();
	m_tabletPath.clear();
	update();
}

void TabletTester::paintEvent(QPaintEvent *e)
{
	Q_UNUSED(e);
	const int w = width();
	const int h = height();
	QPainter p(this);
	p.fillRect(0, 0, w, h, QColor(200, 200, 200));
	p.setPen(QColor(128, 128, 128));

	// Draw grid
	for(int i=w/8;i<w;i+=w/8)
		p.drawLine(i, 0, i, h);
	for(int i=h/8;i<h;i+=h/8)
		p.drawLine(0, i, w, i);

	// Draw paths
	if(!m_mousePath.isEmpty()) {
		p.setPen(QPen(Qt::red, 3));
		p.drawPolyline(m_mousePath);
	}
	if(!m_tabletPath.isEmpty()) {
		p.setPen(QPen(Qt::blue, 2));
		p.drawPolyline(m_tabletPath);
	}
}

void TabletTester::mousePressEvent(QMouseEvent *e)
{
	const auto mousePos = compat::mousePos(*e);
	emit eventReport(tr("Mouse press X=%1 Y=%2 B=%3").arg(mousePos.x()).arg(mousePos.y()).arg(e->button()));
	m_mouseDown = true;
	m_mousePath.clear();
	update();
}

void TabletTester::mouseMoveEvent(QMouseEvent *e)
{
	const auto mousePos = compat::mousePos(*e);
	emit eventReport(tr("Mouse move X=%1 Y=%2 B=%3").arg(mousePos.x()).arg(mousePos.y()).arg(e->buttons()));
	m_mousePath << e->pos();
	update();
}

void TabletTester::mouseReleaseEvent(QMouseEvent *e)
{
	Q_UNUSED(e);
	emit eventReport(tr("Mouse release"));
	m_mouseDown = false;
}

void TabletTester::tabletEvent(QTabletEvent *e)
{
	e->accept();


	auto device = compat::tabDevice(*e);

	QString msg;
	switch(device) {
		case compat::DeviceType::Stylus: msg = tr("Stylus"); break;
		default: {
			msg = tr("Device(%1)").arg(int(device));
			break;
		}
	}

	switch(e->type()) {
		case QEvent::TabletMove:
			msg = tr("%1 move").arg(msg);
			break;
		case QEvent::TabletPress:
			msg = tr("%1 press").arg(msg);
			m_tabletPath.clear();
			m_tabletDown = true;
			break;
		case QEvent::TabletRelease:
			msg = tr("%1 release").arg(msg);
			m_tabletDown = false;
			break;
		default:
			msg = tr("%1 event-%2").arg(msg).arg(e->type());
			break;
	}

	const auto posF = compat::tabPosF(*e);
	msg = tr("%1 X=%2 Y=%3 B=%4 P=%5%")
		.arg(msg)
		.arg(posF.x(), 0, 'f', 2)
		.arg(posF.y(), 0, 'f', 2)
		.arg(e->buttons())
		.arg(e->pressure()*100, 0, 'f', 1)
		;

	if(e->type() == QEvent::TabletMove) {
		if(m_tabletDown) {
			msg = tr("%1 (DRAW)").arg(msg);
			m_tabletPath << compat::tabPosF(*e).toPoint();
			update();
		} else {
			msg = tr("%1 (HOVER)").arg(msg);
		}
	}

	emit eventReport(msg);
}

}
