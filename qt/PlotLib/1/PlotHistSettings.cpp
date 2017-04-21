#include "PlotHistSettings.h"
#include "ui_PlotHistSettings.h"

PlotHistSettings::PlotHistSettings(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::PlotHistSettings)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint));

    ui->cbAutoWidth->setVisible(false);
}

PlotHistSettings::~PlotHistSettings()
{
    delete ui;
}

bool PlotHistSettings::MDI() const {
    return ui->cbMDI->isChecked();
}
void PlotHistSettings::SetMDI(bool aMDI) {
    ui->cbMDI->setChecked(aMDI);
}

bool PlotHistSettings::AutoWidth() const {
    return ui->cbAutoWidth->isChecked();
}
void PlotHistSettings::SetAutoWidth(bool aAutoWidth) {
    ui->cbAutoWidth->setVisible(true);
    ui->cbAutoWidth->setChecked(aAutoWidth);
}
