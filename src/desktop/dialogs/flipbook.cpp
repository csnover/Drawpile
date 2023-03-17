// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/flipbook.h"
#include "libclient/canvas/paintengine.h"
#include "libclient/utils/icon.h"
#include "desktop/utils/dynamicui.h"
#include "desktop/utils/qtguicompat.h"
#include "rustpile/rustpile.h"

#include "ui_flipbook.h"

#include <QSettings>
#include <QRect>
#include <QTimer>
#include <QScreen>
#include <QApplication>

namespace dialogs {

DP_DYNAMIC_DEFAULT_IMPL(Flipbook)

Flipbook::Flipbook(QWidget *parent)
	: DynamicUiWidget(parent)
	, m_paintengine(nullptr)
{
	connect(m_ui->rewindButton, &QToolButton::clicked, this, &Flipbook::rewind);
	connect(m_ui->playButton, &QToolButton::clicked, this, &Flipbook::playPause);
	connect(m_ui->layerIndex, QOverload<int>::of(&QSpinBox::valueChanged), this, &Flipbook::loadFrame);
	connect(m_ui->loopStart, QOverload<int>::of(&QSpinBox::valueChanged), this, &Flipbook::updateRange);
	connect(m_ui->loopEnd, QOverload<int>::of(&QSpinBox::valueChanged), this, &Flipbook::updateRange);
	connect(m_ui->fps, QOverload<int>::of(&QSpinBox::valueChanged), this, &Flipbook::updateFps);
	connect(&m_timer, &QTimer::timeout, m_ui->layerIndex, &QSpinBox::stepUp);
	connect(m_ui->view, &FlipbookView::cropped, this, &Flipbook::setCrop);
	connect(m_ui->zoomButton, &QToolButton::clicked, this, &Flipbook::resetCrop);

	updateRange();

	m_ui->playButton->setFocus();

	// Load default settings
	QSettings cfg;
	cfg.beginGroup("flipbook");

	m_ui->fps->setValue(cfg.value("fps", 15).toInt());

	QRect geom = cfg.value("window", QRect()).toRect();
	if(geom.isValid()) {
		setGeometry(geom);
	}

	// Autoplay
	m_ui->playButton->click();

	m_timelineModeLabelText = makeTranslator(m_ui->timelineModeLabel, [=](bool useTimeline) {
		m_ui->timelineModeLabel->setText(useTimeline
			? tr("Timeline: manual")
			: tr("Timeline: automatic")
		);
	}, false);
}

Flipbook::~Flipbook()
{
	// Save settings
	QSettings cfg;
	cfg.beginGroup("flipbook");

	cfg.setValue("fps", m_ui->fps->value());
	cfg.setValue("window", geometry());
	cfg.setValue("crop", m_crop);
}

void Flipbook::updateRange()
{
	m_ui->layerIndex->setMinimum(m_ui->loopStart->value());
	m_ui->layerIndex->setMaximum(m_ui->loopEnd->value());
}

void Flipbook::rewind()
{
	m_ui->layerIndex->setValue(m_ui->layerIndex->minimum());
}

void Flipbook::playPause()
{
	if(m_timer.isActive()) {
		m_timer.stop();
		m_ui->playButton->setIcon(icon::fromTheme("media-playback-start"));

	} else {
		m_timer.start(1000 / m_ui->fps->value());
		m_ui->playButton->setIcon(icon::fromTheme("media-playback-pause"));
	}
}

void Flipbook::updateFps(int newFps)
{
	if(m_timer.isActive()) {
		m_timer.setInterval(1000 / newFps);
	}
	QPalette pal = palette();
	if(newFps != m_realFps)
		pal.setColor(QPalette::Text, Qt::red);
	m_ui->fps->setPalette(pal);
}

void Flipbook::setPaintEngine(canvas::PaintEngine *pe)
{
	Q_ASSERT(pe);

	m_paintengine = pe;
	const int max = m_paintengine->frameCount();
	m_ui->loopStart->setMaximum(max);
	m_ui->loopEnd->setMaximum(max);
	m_ui->layerIndex->setMaximum(max);
	m_ui->layerIndex->setSuffix(QStringLiteral("/%1").arg(max));
	m_ui->loopEnd->setValue(max);

	m_crop = QRect(QPoint(), pe->size());

	const QRect crop = QSettings().value("flipbook/crop").toRect();
	if(m_crop.contains(crop, true)) {
		m_crop = crop;
		m_ui->zoomButton->setEnabled(true);
	} else {
		m_ui->zoomButton->setEnabled(false);
	}

	m_realFps = rustpile::paintengine_get_metadata_int(pe->engine(), rustpile::MetadataInt::Framerate);

	m_timelineModeLabelText.args(rustpile::paintengine_get_metadata_int(
		m_paintengine->engine(),
		rustpile::MetadataInt::UseTimeline
	));

	updateFps(m_ui->fps->value());
	resetFrameCache();
	loadFrame();
}

void Flipbook::setCrop(const QRectF &rect)
{
	const int w = m_crop.width();
	const int h = m_crop.height();

	if(rect.width()*w<=5 || rect.height()*h<=5) {
		m_crop = QRect(QPoint(), m_paintengine->size());
		m_ui->zoomButton->setEnabled(false);
	} else {
		m_crop = QRect(
			m_crop.x() + rect.x()*w,
			m_crop.y() + rect.y()*h,
			rect.width()*w,
			rect.height()*h
		);
		m_ui->zoomButton->setEnabled(true);
	}

	resetFrameCache();
	loadFrame();
}

void Flipbook::resetCrop()
{
	setCrop(QRectF());
}

void Flipbook::resetFrameCache()
{
	m_frames.clear();
	if(m_paintengine) {
		const int frames = m_paintengine->frameCount();
		for(int i=0;i<frames;++i)
			m_frames.append(QPixmap());
	}
}

void Flipbook::loadFrame()
{
	const int f = m_ui->layerIndex->value() - 1;
	if(m_paintengine && f>=0 && f < m_frames.size()) {
		if(m_frames.at(f).isNull()) {
			QImage img = m_paintengine->getFrameImage(f, m_crop);

			// Scale down the image if it is too big
			const QSize maxSize = compat::widgetScreen(*this)->availableSize() * 0.7;

			if(img.width() > maxSize.width() || img.height() > maxSize.height()) {
				const QSize newSize = QSize(img.width(), img.height()).boundedTo(maxSize);
				img = img.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}

			m_frames[f] = QPixmap::fromImage(img);
		}

		m_ui->view->setPixmap(m_frames.at(f));
	} else
		m_ui->view->setPixmap(QPixmap());
}

}
