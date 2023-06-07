// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/dialogs/curvepresetdialog.h"
#include "desktop/main.h"
#include "desktop/widgets/kis_curve_widget.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QtMath>

namespace dialogs {

CurvePresetDialog::CurvePresetDialog(
	const KisCubicCurve &current, bool linear, QWidget *parent)
	: QDialog(parent)
{
	setModal(true);

	m_curveWidget = new KisCurveWidget{this};
	m_curveWidget->setReadOnly(true);
	m_curveWidget->setLinear(linear);
	m_curveWidget->setFixedSize(300, 300);

	m_presetList = new QListWidget{this};

	QHBoxLayout *buttons = new QHBoxLayout;

	m_useButton = new QPushButton(tr("Use"), this);
	buttons->addWidget(m_useButton);

	QPushButton *cancelButton = new QPushButton(tr("Cancel"), this);
	buttons->addWidget(cancelButton);

	m_saveRenameButton = new QPushButton{tr("Rename"), this};
	buttons->addWidget(m_saveRenameButton);

	m_deleteButton = new QPushButton{tr("Delete"), this};
	buttons->addWidget(m_deleteButton);

	QGridLayout *grid = new QGridLayout{this};
	grid->addWidget(m_curveWidget, 0, 0, 2, 1);
	grid->addWidget(m_presetList, 0, 1);
	grid->addLayout(buttons, 1, 1);

	connect(
		m_presetList, &QListWidget::currentItemChanged, this,
		&CurvePresetDialog::curveSelected);
	connect(
		m_presetList, &QListWidget::itemDoubleClicked, this,
		&CurvePresetDialog::curveDoubleClicked);
	connect(
		m_useButton, &QPushButton::clicked, this, &CurvePresetDialog::accept);
	connect(
		cancelButton, &QPushButton::clicked, this, &CurvePresetDialog::reject);
	connect(
		m_saveRenameButton, &QPushButton::clicked, this,
		&CurvePresetDialog::saveRenameCurve);
	connect(
		m_deleteButton, &QPushButton::clicked, this,
		&CurvePresetDialog::deleteCurve);

	loadPresets(current, linear);
}

CurvePresetDialog::~CurvePresetDialog() {}

KisCubicCurve CurvePresetDialog::curve() const
{
	return m_curveWidget->curve();
}

void CurvePresetDialog::curveSelected(
	QListWidgetItem *current, QListWidgetItem *previous)
{
	Q_UNUSED(previous);
	bool isUnsaved = false;
	if(current) {
		KisCubicCurve curve;
		curve.fromString(current->data(CurveRole).toString());
		m_curveWidget->setCurve(curve);
		switch(current->data(TypeRole).toInt()) {
		case Unsaved:
			m_useButton->setEnabled(false);
			m_saveRenameButton->setEnabled(true);
			m_deleteButton->setEnabled(false);
			isUnsaved = true;
			break;
		case Saved:
			m_useButton->setEnabled(true);
			m_saveRenameButton->setEnabled(true);
			m_deleteButton->setEnabled(true);
			break;
		case Builtin:
			m_useButton->setEnabled(true);
			m_saveRenameButton->setEnabled(false);
			m_deleteButton->setEnabled(false);
			break;
		}
	} else {
		m_useButton->setEnabled(false);
		m_saveRenameButton->setEnabled(false);
		m_deleteButton->setEnabled(false);
	}

	m_saveRenameButton->setText(isUnsaved ? tr("Save") : tr("Rename"));
}

void CurvePresetDialog::curveDoubleClicked(QListWidgetItem *item)
{
	if(item) {
		curveSelected(item, nullptr);
		accept();
	}
}

void CurvePresetDialog::saveRenameCurve()
{
	QListWidgetItem *item = m_presetList->currentItem();
	if(item) {
		int type = item->data(TypeRole).toInt();
		if(type == Unsaved) {
			bool ok;
			QString name = QInputDialog::getText(
				this, tr("Save Curve"), tr("Name"), QLineEdit::Normal,
				QString{}, &ok);
			if(ok && !name.trimmed().isEmpty()) {
				item->setIcon(getPresetIcon(Saved));
				item->setText(name.trimmed());
				item->setData(TypeRole, Saved);
				QFont font = item->font();
				font.setItalic(false);
				item->setFont(font);
				savePresets();
				curveSelected(item, nullptr);
			}
		} else if(type == Saved) {
			bool ok;
			QString name = QInputDialog::getText(
				this, tr("Rename Curve"), tr("Name"), QLineEdit::Normal,
				item->text(), &ok);
			if(ok && !name.trimmed().isEmpty()) {
				item->setText(name.trimmed());
				savePresets();
			}
		}
	}
}

void CurvePresetDialog::deleteCurve()
{
	QListWidgetItem *item = m_presetList->currentItem();
	if(item && item->data(TypeRole).toInt() == Saved) {
		QMessageBox::StandardButton result = QMessageBox::question(
			this, tr("Delete Curve"),
			tr("Really delete curve '%1'?").arg(item->text()));
		if(result == QMessageBox::Yes) {
			delete item; // Deleting the item will remove it from the list.
			savePresets();
		}
	}
}

void CurvePresetDialog::loadPresets(const KisCubicCurve &current, bool linear)
{
	loadSavedPresets();
	loadFunctionPreset(linear, tr("Smooth Out"), quadraticOut);
	loadFunctionPreset(linear, tr("Smooth In"), quadraticIn);
	loadFunctionPreset(linear, tr("Smooth"), quadraticInOut);
	addPreset(tr("Linear"), Builtin, KisCubicCurve{{{0.0, 0.0}, {1.0, 1.0}}});
	addPreset(tr("Current (unsaved)"), Unsaved, current);
	m_presetList->setCurrentRow(0);
}

void CurvePresetDialog::loadSavedPresets()
{
	auto &settings = dpApp().settings();
	// TODO: This migration should be in Settings
	if (settings.curvesPresets().isEmpty() && !settings.curvesPresetsConverted()) {
		convertInputPresetsToCurvePresets(settings);
		settings.setCurvesPresetsConverted(true);
	}

	const auto presets = dpApp().settings().curvesPresets();
	for (const auto &preset : presets) {
		KisCubicCurve curve;
		curve.fromString(preset.value("curve").toString());
		addPreset(preset.value("name").toString(), Saved, curve);
	}
}

void CurvePresetDialog::convertInputPresetsToCurvePresets(desktop::settings::Settings &cfg)
{
	// TODO: This migration should be in Settings
	desktop::settings::Settings::CurvesPresetsType curves;
	const auto inputs = cfg.inputPresets();
	for (const auto &input : inputs) {
		const auto curveString = input.value("curve").toString();
		// A straight line is not an interesting preset.
		if(!isLinearCurve(curveString)) {
			curves.append({
				{"name", input.value("name")},
				{"curve", curveString}
			});
		}
	}
}

bool CurvePresetDialog::isLinearCurve(const QString &curveString)
{
	KisCubicCurve curve;
	curve.fromString(curveString);
	QList<QPointF> points = curve.points();
	return points == QList<QPointF>{{0.0, 0.0}, {1.0, 1.0}};
}

void CurvePresetDialog::loadFunctionPreset(
	bool linear, const QString &name, double (*f)(double))
{
	QList<QPointF> points;
	int npoints = linear ? 10 : 4;
	points.reserve(npoints);
	for(int i = 0; i <= npoints; ++i) {
		double x = 1.0 / double(npoints) * double(i);
		points.append({x, f(x)});
	}
	addPreset(name, Builtin, KisCubicCurve{points});
}

void CurvePresetDialog::addPreset(
	const QString &name, int type, const KisCubicCurve &curve)
{
	QListWidgetItem *item = new QListWidgetItem{getPresetIcon(type), name};
	item->setData(CurveRole, curve.toString());
	item->setData(TypeRole, type);
	if(type == Unsaved) {
		QFont font = item->font();
		font.setItalic(true);
		item->setFont(font);
	}
	m_presetList->insertItem(0, item);
}

QIcon CurvePresetDialog::getPresetIcon(int type)
{
	switch(type) {
	case Saved:
		return QIcon::fromTheme("pathshape");
	case Builtin:
		return QIcon::fromTheme("insert-math-expression");
	default:
		return QIcon{};
	}
}

void CurvePresetDialog::savePresets()
{
	desktop::settings::Settings::CurvesPresetsType presets;
	const auto count = m_presetList->count();
	for(auto i = 0; i < count; ++i) {
		QListWidgetItem *item = m_presetList->item(i);
		if(item->data(TypeRole).toInt() == Saved) {
			presets.append({
				{"name", item->text()},
				{"curve", item->data(CurveRole).toString()}
			});
		}
	}
	dpApp().settings().setCurvesPresets(presets);
}

double CurvePresetDialog::quadraticIn(double x)
{
	return x * x;
}

double CurvePresetDialog::quadraticOut(double x)
{
	return 1 - (1 - x) * (1 - x);
}

double CurvePresetDialog::quadraticInOut(double x)
{
	return x < 0.5 ? 2.0 * x * x : 1.0 - qPow(-2.0 * x + 2.0, 2.0) / 2.0;
}

}
