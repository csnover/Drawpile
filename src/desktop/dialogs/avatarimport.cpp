// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/avatarimport.h"
#include "desktop/utils/dynamicui.h"
#include "libclient/utils/avatarlistmodel.h"

#include "ui_avatarimport.h"

#include <QImageReader>
#include <QFileDialog>
#include <QMessageBox>

namespace dialogs {

DP_DYNAMIC_DEFAULT_IMPL(AvatarImport)

AvatarImport::AvatarImport(const QImage &source, QWidget *parent)
	: DynamicUiWidget(parent)
	, m_originalImage(source)
{
	const int maxSize = qMin(source.width(), source.height());

	m_ui->resizer->setImage(source);
	m_ui->resizer->setOriginalSize(source.size());
	m_ui->resizer->setTargetSize(QSize { maxSize, maxSize });

	m_ui->sizeSlider->setMaximum(maxSize);
	m_ui->sizeSlider->setValue(maxSize);

	connect(m_ui->sizeSlider, &QSlider::valueChanged, this, [this](int value) {
		m_ui->resizer->setTargetSize(QSize { value, value });
	});
}

AvatarImport::~AvatarImport()
{}

QImage AvatarImport::croppedAvatar() const
{
	const QPoint offset = m_ui->resizer->offset();
	const QSize size = m_ui->resizer->targetSize();

	return m_originalImage.copy(
		-offset.x(),
		-offset.y(),
		size.width(),
		size.height()
	);
}

void AvatarImport::importAvatar(AvatarListModel *avatarList, QPointer<QWidget> parentWindow)
{
	QString formats;
	for(const auto &format : QImageReader::supportedImageFormats()) {
		formats += "*." + format + " ";
	}

	QString path = QFileDialog::getOpenFileName(parentWindow, tr("Import Avatar"), QString(),
		QCoreApplication::translate("FileFormatOptions", "Images (%1)").arg(formats) + ";;" +
		QFileDialog::tr("All Files (*)")
	);

	if(path.isEmpty())
		return;

	const QImage picture(path);
	if(picture.isNull()) {
		QMessageBox::warning(parentWindow, tr("Import Avatar"), tr("Couldn't read image"));
		return;
	}

	if(picture.width() < Size || picture.height() < Size) {
		QMessageBox::warning(parentWindow, tr("Import Avatar"), tr("Picture is too small"));
		return;
	}

	if(picture.width() != picture.height()) {
		// Not square format: needs cropping
		auto *dlg = new dialogs::AvatarImport(picture, parentWindow);
		dlg->setModal(true);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		connect(dlg, &QDialog::accepted, avatarList, [parentWindow, dlg, avatarList]() {
			avatarList->addAvatar(QPixmap::fromImage(dlg->croppedAvatar().scaled(Size, Size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
		});

		dlg->show();

	} else {
		avatarList->addAvatar(QPixmap::fromImage(picture.scaled(Size, Size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
	}
}

}
