// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include <QWidget>

class QLabel;
class QPaintEvent;
class QPushButton;

namespace widgets {

class NotificationBar : public QWidget
{
	Q_OBJECT
public:
	enum class RoleColor {
		Warning,
		Fatal
	};

	NotificationBar(QWidget *parent);

	void show(const QString &text, const QString &actionButtonLabel, RoleColor color);

signals:
	void actionButtonClicked();

protected:
	void paintEvent(QPaintEvent *) override;

private:
	QColor m_color;
	QLabel *m_icon;
	QLabel *m_label;
	QPushButton *m_actionButton;
	QPushButton *m_closeButton;
};

}
