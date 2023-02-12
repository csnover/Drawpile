// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen, askmeaboutloom

#ifndef DP_BRUSHPRESETMODEL_H
#define DP_BRUSHPRESETMODEL_H

#include <QAbstractItemModel>

class QFile;
class QFileInfo;

namespace brushes {

struct ActiveBrush;
struct ClassicBrush;
struct MyPaintBrush;

struct Tag {
	int id;
	QString name;
	bool editable;
};

struct TagAssignment {
	int id;
	QString name;
	bool assigned;
};

struct PresetMetadata {
	int id;
	QString name;
	QString description;
	QByteArray thumbnail;
};

class BrushPresetModel;

class BrushPresetTagModel : public QAbstractItemModel {
	Q_OBJECT
	friend class BrushPresetModel;
public:
	explicit BrushPresetTagModel(QObject *parent = nullptr);
	virtual ~BrushPresetTagModel();

	BrushPresetModel *presetModel() { return m_presetModel; }

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QModelIndex parent(const QModelIndex &index) const override;
	QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	Tag getTagAt(int row) const;
	int getTagRowById(int tagId) const;

	int newTag(const QString& name);
	int editTag(int tagId, const QString &name);
	void deleteTag(int tagId);

	void setState(const QString &key, const QVariant &value);
	QVariant getState(const QString &key) const;

private:
	class Private;
	Private *d;
	BrushPresetModel *m_presetModel;

	static bool isBuiltInTag(int row);

	void convertOrCreateClassicPresets();
	bool convertOldPresets();
	void createDefaultClassicPresets();
	void newClassicPreset(int tagId, const QString &name,
		const QString &description, const ActiveBrush &brush);
};

class BrushPresetModel : public QAbstractItemModel {
	Q_OBJECT
public:
	enum Roles {
		FilterRole = Qt::UserRole + 1,
		BrushRole,
	};

	explicit BrushPresetModel(BrushPresetTagModel *tagModel);
	virtual ~BrushPresetModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QModelIndex parent(const QModelIndex &index) const override;
	QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	int getIdFromIndex(const QModelIndex &index) { return index.isValid() ? index.internalId() : 0; };

	void setTagIdToFilter(int tagId);

	QList<TagAssignment> getTagAssignments(int presetId);
	bool changeTagAssignment(int presetId, int tagId, bool assigned);

	PresetMetadata getPresetMetadata(int presetId);

	int newPreset(const QString &type, const QString &name, const QString description,
		const QPixmap &thumbnail, const QByteArray &data);
	int duplicatePreset(int presetId);
	bool updatePresetData(int presetId, const QString &type, const QByteArray &data);
	bool updatePresetMetadata(int presetId, const QString &name, const QString &description,
		const QPixmap &thumbnail);
	bool deletePreset(int presetId);

	QSize iconSize() const;
	int iconDimension() const;
	void setIconDimension(int dimension);

	int importMyPaintBrush(const QString &file);

public slots:
	void tagsAboutToBeReset();
	void tagsReset();

private:
	BrushPresetTagModel::Private *d;
	int m_tagIdToFilter;

	static QPixmap loadBrushPreview(const QFileInfo &fileInfo);
	static QByteArray toPng(const QPixmap &pixmap);
};

}

#endif
