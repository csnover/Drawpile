// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmake-config/config.h"
#include "desktop/dialogs/settingsdialog/general.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QString>
#include <QStyleFactory>
#include <QWidget>

namespace dialogs {
namespace settingsdialog {

constexpr auto KEY = "setting";

General::General(QWidget *parent)
	: QWidget(parent)
{
	auto *form = new QFormLayout(this);

	auto *style = new QComboBox(this);
	style->setProperty(KEY, "theme/style");
	style->addItems(QStyleFactory::keys());
	form->addRow(tr("Style:"), style);

	auto *theme = new QComboBox(this);
	theme->setProperty(KEY, "theme/palette");
	theme->addItems({
		tr("System"),
		tr("Light"),
		tr("Dark"),
		tr("Krita Bright"),
		tr("Krita Dark"),
		tr("Krita Darker")
	});
	form->addRow(tr("Theme:"), theme);

	auto *language = new QComboBox(this);
	language->setProperty(KEY, "language");
	language->addItem(tr("System"), QStringLiteral());

	const QLocale english(QLocale::English);
	language->addItem(tr("%1 (%2)")
		.arg(english.nativeLanguageName())
		.arg(QLocale::languageToString(english.language())),
		QStringLiteral("en")
	);

	language->addItem(QStringLiteral("English"), QStringLiteral("en"));
	const QLocale localeC = QLocale::c();
	for (const auto *localeName : cmake_config::locales()) {
		QLocale locale(localeName);
		if (locale != localeC) {
			language->addItem(
				tr("%1 (%2)")
					.arg(locale.nativeLanguageName())
					.arg(QLocale::languageToString(locale.language())),
				localeName
			);
		}
	}
	form->addRow(tr("Language:"), language);

	auto *separator = new QFrame(this);
	separator->setFrameShape(QFrame::HLine);
	form->addWidget(separator);

	auto *enableLogging = new QCheckBox(this);
	enableLogging->setProperty(KEY, "logfile");
	enableLogging->setText(tr("Write log to file"));
	form->addRow(tr("Logging:"), enableLogging);
}

} // namespace settingsdialog
} // namespace dialogs
