// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H

class QString;
class QWidget;

namespace server {
namespace gui {

class Server;

class PageFactory
{
public:
	PageFactory() = default;
	virtual ~PageFactory() = default;

	/**
	 * @brief Get the title of the page (shown in the sidebar)
	 */
	virtual QString title() const = 0;

	/**
	 * @brief Get the internal page identifier
	 */
	virtual QString pageId() const = 0;

	/**
	 * @brief Construct a new page
	 */
	virtual QWidget *makePage(Server *server) const = 0;
};

}
}

#endif
