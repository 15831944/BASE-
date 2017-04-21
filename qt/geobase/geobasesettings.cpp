#include "geobasesettings.h"
#include "ui_geobasesettings.h"

#include "../VProject/GlobalSettings.h"

GeobaseSettings::GeobaseSettings(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::GeobaseSettings)
{
    ui->setupUi(this);
}

GeobaseSettings::~GeobaseSettings()
{
    delete ui;
}

void GeobaseSettings::SetOnDblClick(int aValue)
{
    ui->cbOnDblClick->setCurrentIndex(aValue);
}

void GeobaseSettings::SetSelectBeh(int aValue)
{
    ui->cbSelectBeh->setCurrentIndex(aValue);
}

void GeobaseSettings::SetSelectMode(int aValue)
{
    ui->cbSelectMode->setCurrentIndex(aValue);
}

void GeobaseSettings::SetDrawingShowMode(int aValue)
{
    ui->cbDrawingShow->setCurrentIndex(aValue);
}

int GeobaseSettings::OnDblClick() const
{
    return ui->cbOnDblClick->currentIndex();
}

int GeobaseSettings::GetSelectBeh() const
{
    return ui->cbSelectBeh->currentIndex();
}

int GeobaseSettings::GetSelectMode() const
{
    return ui->cbSelectMode->currentIndex();
}

int GeobaseSettings::GetDrawingShowMode() const
{
    return ui->cbDrawingShow->currentIndex();
}
