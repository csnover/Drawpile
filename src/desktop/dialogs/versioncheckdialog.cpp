// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "desktop/dialogs/versioncheckdialog.h"
#include "libshared/util/networkaccess.h"
#include "libshared/util/paths.h"

#include "ui_versioncheck.h"

#include <QMessageBox>
#include <QSettings>
#include <QStyle>
#include <QPushButton>
#include <QDir>
#include <QDesktopServices>

namespace dialogs {

DP_DYNAMIC_DEFAULT_IMPL(VersionCheckDialog)

VersionCheckDialog::VersionCheckDialog(QWidget *parent)
	: DynamicUiWidget(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);

	m_downloadButton = m_ui->buttonBox->addButton(QString(), QDialogButtonBox::ActionRole);
	m_downloadButton->hide();
	m_downloadButtonText = makeTranslator(m_downloadButton, [=](QString version, int downloadSize) {
		m_downloadButton->setText(tr("Download %1 (%2)")
			.arg(version)
			.arg(tr("%L1MiB").arg(downloadSize / (1024.0 * 1024.0), 0, 'f', 2))
		);
	}, QString(), 0);

	connect(m_downloadButton, &QPushButton::clicked, this, &VersionCheckDialog::downloadNewVersion);

	m_ui->checkForUpdates->setChecked(QSettings().value("versioncheck/enabled", true).toBool());

	connect(this, &VersionCheckDialog::finished, this, &VersionCheckDialog::rememberSettings);

	m_downloadLabelText = makeTranslator(m_ui->downloadLabel, [=](DownloadState state, QString arg) {
		switch (state) {
		case DownloadState::Downloading:
			m_ui->downloadLabel->setText(tr("Downloading %1...").arg(arg));
			break;
		case DownloadState::Downloaded:
			m_ui->downloadLabel->setText(tr("Downloaded %1!").arg(arg));
			break;
		case DownloadState::Error:
			m_ui->downloadLabel->setText(arg);
			break;
		}
	}, DownloadState::Downloading, QString());

	m_textBrowserHtml = makeTranslator(m_ui->textBrowser, [=](const QVector<NewVersionCheck::Version> *versions) {
		if(!versions || versions->isEmpty()) {
			m_ui->textBrowser->setHtml(QStringLiteral("<h1>%1</h1><p>%2</p>")
				.arg(tr("You're up to date!"), tr("No new versions found."))
			);
		} else {
			QString content = QStringLiteral("<h1>%1</h1>")
				.arg(tr("A new version of Drawpile is available!"));

			for(const auto &version : *versions) {
				content += QStringLiteral("<h2><a href=\"%1\">%2</a></h2>")
					.arg(version.announcementUrl, tr("Version %1").arg(version.version));
				content += version.description;
			}

			m_ui->textBrowser->setHtml(content);
		}
	}, static_cast<const QVector<NewVersionCheck::Version> *>(nullptr));
}

VersionCheckDialog::~VersionCheckDialog()
{}

void VersionCheckDialog::rememberSettings()
{
	QSettings().setValue("versioncheck/enabled", m_ui->checkForUpdates->isChecked());
}

void VersionCheckDialog::doVersionCheckIfNeeded()
{
	if(!QSettings().contains("versioncheck/enabled")) {
		QMessageBox mb {
			QMessageBox::NoIcon,
			tr("Enable auto-updates?"),
			tr("Should Drawpile automatically check for updates?"),
			QMessageBox::Yes | QMessageBox::No
		};
		const auto icon = QIcon::fromTheme("update-none");
		const auto iconSize = mb.style()->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, &mb);
		auto pixmap = icon.pixmap(iconSize);
		mb.setIconPixmap(pixmap);
		mb.setInformativeText(tr("You can always check for updates manually from the menu."));
		const auto result = mb.exec();
		QSettings().setValue("versioncheck/enabled", result == QMessageBox::Yes);
	}

	if(NewVersionCheck::needCheck()) {
		// The dialog will autodelete if there is nothing to show
		VersionCheckDialog *dlg = new VersionCheckDialog;
		dlg->queryNewVersions();
	}
}

void VersionCheckDialog::queryNewVersions()
{
	m_newversion = new NewVersionCheck(this);
	m_newversion->setShowBetas(QSettings().value("versioncheck/beta", false).toBool());
	connect(m_newversion, &NewVersionCheck::versionChecked, this, &VersionCheckDialog::versionChecked);
	m_newversion->queryVersions();
}

void VersionCheckDialog::versionChecked(bool isNew, const QString &errorMessage)
{
	if(!isNew && !isVisible()) {
		// If dialog is not yet visible, this was a background version check.
		// Don't bother the user if there is nothing to tell.
		deleteLater();
		return;
	}

	if(!errorMessage.isEmpty()) {
		m_ui->textBrowser->insertHtml(QStringLiteral("<p style=\"color: red\">%1</p>").arg(errorMessage));
		m_ui->views->setCurrentIndex(1);

	} else {
		setNewVersions(m_newversion->getNewer());
	}

	show();
}

void VersionCheckDialog::setNewVersions(const QVector<NewVersionCheck::Version> &versions)
{
	m_textBrowserHtml.args(&versions);
	if (!versions.isEmpty()) {
		const auto latest = versions.first();
		if(!latest.downloadUrl.isEmpty()) {
			m_downloadUrl = latest.downloadUrl;
			if(m_downloadUrl.isValid() && !m_downloadUrl.fileName().isEmpty()) {
				if(latest.downloadChecksumType == "sha256")
					m_downloadSha256 = QByteArray::fromHex(latest.downloadChecksum.toLatin1());
				m_downloadSize = latest.downloadSize;
				m_downloadButtonText.args(latest.version, latest.downloadSize);
				m_downloadButton->show();
			}
		}
	}

	m_ui->views->setCurrentIndex(1);
}

void VersionCheckDialog::downloadNewVersion()
{
	Q_ASSERT(m_downloadUrl.isValid());

	const QDir downloadDir = utils::paths::writablePath(QStandardPaths::DownloadLocation, ".", ".");
	const auto downloadPath = downloadDir.absoluteFilePath(m_downloadUrl.fileName());

	{
		QFile oldFile(downloadPath);
		if(oldFile.exists()) {
			if(oldFile.open(QFile::ReadOnly)) {
				bool hashOk = true;

				if(!m_downloadSha256.isEmpty()) {
					QCryptographicHash hash(QCryptographicHash::Sha256);
					hash.addData(&oldFile);
					hashOk = hash.result() == m_downloadSha256;
				}

				if(hashOk) {
					// Old download is still valid
					QDesktopServices::openUrl(QUrl::fromLocalFile(downloadDir.path()));
					return;
				}
			}

			// File is corrupt, try to remove it
			oldFile.remove();
		}
	}

	auto *fd = new networkaccess::FileDownload(this);
	fd->setTarget(downloadPath);
	fd->setMaxSize(m_downloadSize);

	if(!m_downloadSha256.isEmpty())
		fd->setExpectedHash(m_downloadSha256, QCryptographicHash::Sha256);

	connect(fd, &networkaccess::FileDownload::progress, this, [this](qint64 progress, qint64 total) {
		m_ui->progressBar->setMaximum(int(total));
		m_ui->progressBar->setValue(int(progress));
	});

	connect(fd, &networkaccess::FileDownload::finished, this, [this, downloadDir](const QString &errorMessage) {
		if(errorMessage.isEmpty()) {
			m_downloadLabelText.args(DownloadState::Downloaded, m_downloadUrl.fileName());
			QDesktopServices::openUrl(QUrl::fromLocalFile(downloadDir.path()));
			close();
		} else {
			m_downloadLabelText.args(DownloadState::Error, errorMessage);
		}
	});

	m_downloadLabelText.args(DownloadState::Downloading, m_downloadUrl.fileName());
	m_ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
	m_downloadButton->hide();
	m_ui->views->setCurrentIndex(2);

	fd->start(m_downloadUrl);
}

}
