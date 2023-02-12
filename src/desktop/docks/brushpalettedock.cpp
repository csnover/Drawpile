/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2017-2022 Calle Laakkonen, askmeaboutloom

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "desktop/docks/brushpalettedock.h"
#include "desktop/dialogs/brushpresetproperties.h"
#include "desktop/widgets/groupedtoolbutton.h"
#include "desktop/toolwidgets/brushsettings.h"
#include "libclient/brushes/brushpresetmodel.h"
#include "libclient/brushes/brush.h"
#include "libclient/utils/icon.h"
#include "desktop/docks/titlewidget.h"

#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>

static constexpr char SELECTED_TAG_ID_KEY[] = "brushpalette:selected_tag_id";

namespace docks {

struct BrushPalette::Private {
	brushes::BrushPresetTagModel *tagModel;
	brushes::BrushPresetModel *presetModel;
	QSortFilterProxyModel *presetProxyModel;
	tools::BrushSettings *brushSettings;
	brushes::Tag currentTag;
	brushes::ActiveBrush newBrush;

	QComboBox *tagComboBox;
	QLineEdit *searchLineEdit;
	widgets::GroupedToolButton *menuButton;
	QMenu *assignmentMenu;
	QAction *newBrushAction;
	QAction *duplicateBrushAction;
	QAction *overwriteBrushAction;
	QAction *editBrushAction;
	QAction *deleteBrushAction;
	QMenu *iconSizeMenu;
	QMenu *menu;
	QAction *newTagAction;
	QAction *editTagAction;
	QAction *deleteTagAction;
	QAction *importMyPaintBrushesAction;
	QListView *presetListView;
};

BrushPalette::BrushPalette(QWidget *parent)
	: QDockWidget(parent)
	, d(new Private)
{
	d->tagModel = new brushes::BrushPresetTagModel(this);
	d->presetModel = d->tagModel->presetModel();

	d->presetProxyModel = new QSortFilterProxyModel(this);
	d->presetProxyModel->setSourceModel(d->presetModel);
	d->presetProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	d->presetProxyModel->setFilterRole(brushes::BrushPresetModel::FilterRole);

	setWindowTitle(tr("Brushes"));

	TitleWidget *titleWidget = new TitleWidget(this);
	setTitleBarWidget(titleWidget);

	d->tagComboBox = new QComboBox(this);
	d->tagComboBox->setInsertPolicy(QComboBox::NoInsert);
	titleWidget->addCustomWidget(d->tagComboBox, true);
	titleWidget->addSpace(4);

	d->searchLineEdit = new QLineEdit(this);
	d->searchLineEdit->setPlaceholderText("Search");
	titleWidget->addCustomWidget(d->searchLineEdit, true);
	titleWidget->addSpace(4);

	d->menuButton = new widgets::GroupedToolButton(this);
	d->menuButton->setIcon(icon::fromTheme("application-menu"));
	d->menuButton->setPopupMode(QToolButton::InstantPopup);
	d->menuButton->setMaximumHeight(d->tagComboBox->height());
	titleWidget->addCustomWidget(d->menuButton);
	titleWidget->addSpace(4);

	d->menu = new QMenu(this);
	d->newTagAction = d->menu->addAction(tr("New Tag"));
	d->editTagAction = d->menu->addAction(tr("Edit Tag"));
	d->deleteTagAction = d->menu->addAction(tr("Delete Tag"));
	d->menu->addSeparator();
	d->newBrushAction = d->menu->addAction(icon::fromTheme("list-add"), tr("New Brush"));
	d->duplicateBrushAction = d->menu->addAction(icon::fromTheme("edit-copy"), tr("Duplicate Brush"));
	d->overwriteBrushAction = d->menu->addAction(icon::fromTheme("document-save"), tr("Overwrite Brush"));
	d->editBrushAction = d->menu->addAction(icon::fromTheme("configure"), tr("Edit Brush"));
	d->deleteBrushAction = d->menu->addAction(icon::fromTheme("list-remove"), tr("Delete Brush"));
	d->assignmentMenu = d->menu->addMenu(tr("Brush Tags"));
	d->iconSizeMenu = d->menu->addMenu(tr("Icon Size"));
	d->menu->addSeparator();
	d->importMyPaintBrushesAction = d->menu->addAction(tr("Import MyPaint Brushes..."));
	d->menuButton->setMenu(d->menu);

	for(int dimension = 16; dimension <= 128; dimension += 16) {
		QAction *sizeAction = d->iconSizeMenu->addAction(tr("%1x%1").arg(dimension));
		sizeAction->setCheckable(true);
		sizeAction->setData(dimension);
		connect(sizeAction, &QAction::triggered, [=](){
			d->presetModel->setIconDimension(dimension);
		});
	}

	d->presetListView = new QListView(this);
	d->presetListView->setUniformItemSizes(true);
	d->presetListView->setFlow(QListView::LeftToRight);
	d->presetListView->setWrapping(true);
	d->presetListView->setResizeMode(QListView::Adjust);
	d->presetListView->setContextMenuPolicy(Qt::CustomContextMenu);
	setWidget(d->presetListView);

	d->tagComboBox->setModel(d->tagModel);
	d->presetListView->setModel(d->presetProxyModel);

	connect(d->presetModel, &QAbstractItemModel::modelReset, this, &BrushPalette::presetsReset);
	connect(d->tagComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &BrushPalette::tagIndexChanged);
	connect(d->searchLineEdit, &QLineEdit::textChanged,
		d->presetProxyModel, &QSortFilterProxyModel::setFilterFixedString);
	connect(d->presetListView->selectionModel(), &QItemSelectionModel::currentChanged,
		this, &BrushPalette::presetSelectionChanged);
	connect(d->newTagAction, &QAction::triggered, this, &BrushPalette::newTag);
	connect(d->editTagAction, &QAction::triggered, this, &BrushPalette::editCurrentTag);
	connect(d->deleteTagAction, &QAction::triggered, this, &BrushPalette::deleteCurrentTag);
	connect(d->newBrushAction, &QAction::triggered, this, &BrushPalette::newPreset);
	connect(d->duplicateBrushAction, &QAction::triggered, this, &BrushPalette::duplicateCurrentPreset);
	connect(d->overwriteBrushAction, &QAction::triggered, this, &BrushPalette::overwriteCurrentPreset);
	connect(d->editBrushAction, &QAction::triggered, this, &BrushPalette::editCurrentPreset);
	connect(d->deleteBrushAction, &QAction::triggered, this, &BrushPalette::deleteCurrentPreset);
	connect(d->importMyPaintBrushesAction, &QAction::triggered, this, &BrushPalette::importMyPaintBrushes);
	connect(d->presetListView, &QAbstractItemView::clicked, this, &BrushPalette::applyToBrushSettings);
	connect(d->presetListView, &QAbstractItemView::doubleClicked, this, &BrushPalette::editCurrentPreset);
	connect(d->presetListView, &QWidget::customContextMenuRequested, this, &BrushPalette::showPresetContextMenu);

	bool selectedTagIdOk;
	int selectedTagId = d->tagModel->getState(SELECTED_TAG_ID_KEY).toInt(&selectedTagIdOk);
	int selectedTagRow = selectedTagIdOk ? d->tagModel->getTagRowById(selectedTagId) : -1;
	int initialTagRow = selectedTagRow > 0 ? selectedTagRow : 0;
	d->tagComboBox->setCurrentIndex(initialTagRow);
	tagIndexChanged(d->tagComboBox->currentIndex());
	presetsReset();
}

BrushPalette::~BrushPalette()
{
	delete d;
}

void BrushPalette::connectBrushSettings(tools::ToolSettings *toolSettings)
{
	d->brushSettings = qobject_cast<tools::BrushSettings*>(toolSettings);
}

void BrushPalette::tagIndexChanged(int row)
{
	d->currentTag = d->tagModel->getTagAt(row);
	d->editTagAction->setEnabled(d->currentTag.editable);
	d->deleteTagAction->setEnabled(d->currentTag.editable);
	d->presetModel->setTagIdToFilter(d->currentTag.id);
	d->tagModel->setState(SELECTED_TAG_ID_KEY, d->currentTag.id);
}

void BrushPalette::presetsReset()
{
	presetSelectionChanged(d->presetListView->currentIndex(), QModelIndex());
	int iconDimension = d->presetModel->iconDimension();
	for(QAction *action : d->iconSizeMenu->actions()) {
		action->setChecked(action->data().toInt() == iconDimension);
	}
}

void BrushPalette::presetSelectionChanged(const QModelIndex &current, const QModelIndex &)
{
	int presetId = presetProxyIndexToId(current);
	bool selected = presetId > 0;
	if(selected) {
		QList<brushes::TagAssignment> tagAssignments = d->presetModel->getTagAssignments(presetId);
		if(!tagAssignments.isEmpty()) {
			std::sort(tagAssignments.begin(), tagAssignments.end(),
				[](const brushes::TagAssignment &a, const brushes::TagAssignment &b) {
					return a.name < b.name;
				});
			d->assignmentMenu->clear();
			for (const brushes::TagAssignment &ta : tagAssignments) {
				QAction *action = d->assignmentMenu->addAction(ta.name);
				action->setCheckable(true);
				action->setChecked(ta.assigned);
				int tagId = ta.id;
				connect(action, &QAction::triggered, [=](bool checked) {
					changeTagAssignment(tagId, checked);
				});
			}
		}
	}
	d->assignmentMenu->setEnabled(selected && !d->assignmentMenu->actions().isEmpty());
	d->duplicateBrushAction->setEnabled(selected);
	d->overwriteBrushAction->setEnabled(selected);
	d->editBrushAction->setEnabled(selected);
	d->deleteBrushAction->setEnabled(selected);
}

void BrushPalette::newTag()
{
	bool ok;
	QString name = QInputDialog::getText(
		this, tr("New Tag"), tr("Tag name:"), QLineEdit::Normal, QString(), &ok);
	if(ok && !(name = name.trimmed()).isEmpty()) {
		int tagId = d->tagModel->newTag(name);
		if(tagId > 0) {
			d->tagComboBox->setCurrentIndex(tagIdToProxyRow(tagId));
		}
	}
}

void BrushPalette::editCurrentTag()
{
	if(d->currentTag.editable) {
		bool ok;
		QString name = QInputDialog::getText(
			this, tr("Edit Tag"), tr("Tag name:"), QLineEdit::Normal,
			d->currentTag.name, &ok);
		if(ok && !(name = name.trimmed()).isEmpty()) {
			int sourceRow = d->tagModel->editTag(d->currentTag.id, name);
			if(sourceRow >= 0) {
				d->tagComboBox->setCurrentIndex(sourceRow);
			}
		}
	}
}

void BrushPalette::deleteCurrentTag()
{
	if(d->currentTag.editable) {
		d->tagModel->deleteTag(d->currentTag.id);
	}
}

void BrushPalette::newPreset()
{
	if(!d->brushSettings) {
		qWarning("Cannot overwrite preset: BrushSettings not connected to BrushPalette");
		return;
	}
	d->newBrush = d->brushSettings->currentBrush();

	dialogs::BrushPresetProperties *dialog = new dialogs::BrushPresetProperties(
		0, tr("New Brush"), QString(), d->newBrush.presetThumbnail(), this);
	connect(dialog, &dialogs::BrushPresetProperties::presetPropertiesApplied,
		this, &BrushPalette::applyPresetProperties);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->setWindowTitle(tr("New Brush Preset"));
	dialog->show();
}

void BrushPalette::duplicateCurrentPreset()
{
	int presetId = currentPresetId();
	if(presetId > 0) {
		d->presetModel->duplicatePreset(presetId);
	}
}

void BrushPalette::overwriteCurrentPreset()
{
	if(!d->brushSettings) {
		qWarning("Cannot overwrite preset: BrushSettings not connected to BrushPalette");
		return;
	}
	int presetId = currentPresetId();
	if(presetId > 0) {
		brushes::ActiveBrush brush = d->brushSettings->currentBrush();
		d->presetModel->updatePresetData(presetId, brush.presetType(), brush.presetData());
	}
}

void BrushPalette::editCurrentPreset()
{
	int presetId = currentPresetId();
	if(presetId > 0) {
		brushes::PresetMetadata pm = d->presetModel->getPresetMetadata(presetId);
		if(pm.id > 0) {
			QPixmap thumbnail;
			if(!thumbnail.loadFromData(pm.thumbnail)) {
				qWarning("Loading thumbnail for preset %d failed", pm.id);
			}
			dialogs::BrushPresetProperties *dialog = new dialogs::BrushPresetProperties(
				pm.id, pm.name, pm.description, thumbnail, this);
			connect(dialog, &dialogs::BrushPresetProperties::presetPropertiesApplied,
				this, &BrushPalette::applyPresetProperties);
			dialog->setAttribute(Qt::WA_DeleteOnClose);
			dialog->setWindowTitle(tr("Edit Brush Preset"));
			dialog->show();
		}
	}
}

void BrushPalette::deleteCurrentPreset()
{
	int presetId = currentPresetId();
	if(presetId > 0) {
		d->presetModel->deletePreset(presetId);
	}
}

void BrushPalette::importMyPaintBrushes()
{
	QStringList files = QFileDialog::getOpenFileNames(this,
		tr("Select one or more MyPaint brushes to import"), QString(),
		"MyPaint Brush (*.myb)");

	if(!files.isEmpty()) {
		int tagId = d->currentTag.id;
		QStringList successes, failures;
		for(const QString &file : files) {
			int presetId = d->presetModel->importMyPaintBrush(file);
			(presetId > 0 ? successes : failures).append(QFileInfo(file).completeBaseName());
			if(tagId > 0) {
				d->presetModel->changeTagAssignment(presetId, tagId, true);
			}
		}

		QStringList messages;
		messages.append(tr("Import of %1 MyPaint brush(es).").arg(files.count()));
		if(!successes.isEmpty()) {
			messages.append(tr("%1 successfully imported: %2")
				.arg(successes.count()).arg(successes.join(", ")));
		}
		if(!failures.isEmpty()) {
			messages.append(tr("%1 failed to import: %2")
				.arg(failures.count()).arg(failures.join(", ")));
		}

		QMessageBox::information(this, tr("MyPaint Brush Import"), messages.join("\n\n"));
	}
}

void BrushPalette::applyPresetProperties(int id, const QString &name,
	const QString &description, const QPixmap &thumbnail)
{
	if(id == 0) {
		int presetId = d->presetModel->newPreset(d->newBrush.presetType(), name,
			description, thumbnail, d->newBrush.presetData());
		int tagId = d->currentTag.id;
		if(presetId > 0 && tagId > 0) {
			d->presetModel->changeTagAssignment(presetId, tagId, true);
		}
	} else {
		d->presetModel->updatePresetMetadata(id, name, description, thumbnail);
	}
}

void BrushPalette::applyToBrushSettings(const QModelIndex &proxyIndex)
{
	if(!d->brushSettings) {
		qWarning("BrushSettings not connected to BrushPalette");
		return;
	}

	QModelIndex sourceIndex = presetIndexToSource(proxyIndex);
	QVariant v = sourceIndex.data(brushes::BrushPresetModel::BrushRole);
	if(v.isNull()) {
		qWarning("Brush was null");
		return;
	}

	d->brushSettings->setCurrentBrush(v.value<brushes::ActiveBrush>());
}

void BrushPalette::showPresetContextMenu(const QPoint &pos)
{
	d->menu->popup(d->presetListView->mapToGlobal(pos));
}

void BrushPalette::changeTagAssignment(int tagId, bool assigned)
{
	int presetId = currentPresetId();
	if(presetId > 0) {
		d->presetModel->changeTagAssignment(presetId, tagId, assigned);
	}
}

int BrushPalette::tagIdToProxyRow(int tagId)
{
	return d->tagModel->getTagRowById(tagId);
}

int BrushPalette::presetRowToSource(int proxyRow)
{
	return presetIndexToSource(d->presetProxyModel->index(proxyRow, 0)).row();
}

int BrushPalette::presetRowToProxy(int sourceRow)
{
	return presetIndexToProxy(d->presetModel->index(sourceRow)).row();
}

QModelIndex BrushPalette::presetIndexToSource(const QModelIndex &proxyIndex)
{
	return d->presetProxyModel->mapToSource(proxyIndex);
}

QModelIndex BrushPalette::presetIndexToProxy(const QModelIndex &sourceIndex)
{
	return d->presetProxyModel->mapFromSource(sourceIndex);
}

int BrushPalette::presetProxyIndexToId(const QModelIndex &proxyIndex)
{
	return d->presetModel->getIdFromIndex(presetIndexToSource(proxyIndex));
}

int BrushPalette::currentPresetId()
{
	return presetProxyIndexToId(d->presetListView->currentIndex());
}

}
