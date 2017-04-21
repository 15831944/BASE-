#include <QColorDialog>
#include "DocTreeSettings.h"
#include "ui_DocTreeSettings.h"

DocTreeSettings::DocTreeSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocTreeSettings)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint));

    ui->wdExpandLevel->setVisible(false);
    ui->gbTypeTree->setVisible(false);
    ui->cbTTFontPlusOne->setVisible(false);
    ui->cbTTFontBold->setVisible(false);

    ui->wdWindowTitle->setVisible(false);
    ui->cbOpenSingle->setVisible(false);
    ui->cbShowGrid->setVisible(false);
    ui->cbAutoWidth->setVisible(false);
    ui->cbDocFontPlusOne->setVisible(false);
    ui->cbDocFontBold->setVisible(false);

    ui->wdAddRowHeight->setVisible(false);

    ui->cbDragDrop->setVisible(false);
    ui->wdDocDblClick->setVisible(false);
    ui->wdSecondLevel->setVisible(false);
    ui->cbExpandOnShow->setVisible(false);

    ui->gbColors->setVisible(false);
}

DocTreeSettings::~DocTreeSettings()
{
    delete ui;
}

int DocTreeSettings::TTExpandLevel() const {
    return ui->spExpandTT->value();
}

void DocTreeSettings::SetTTExpandLevel(int aTTExpandLevel) {
    ui->gbTypeTree->setVisible(true);
    ui->wdExpandLevel->setVisible(true);
    ui->spExpandTT->setValue(aTTExpandLevel);
}

bool DocTreeSettings::TTFontPlusOne() const {
    return ui->cbTTFontPlusOne->isChecked();
}

void DocTreeSettings::SetTTFontPlusOne(bool aTTFontPlusOne) {
    ui->gbTypeTree->setVisible(true);
    ui->cbTTFontPlusOne->setVisible(true);
    ui->cbTTFontPlusOne->setChecked(aTTFontPlusOne);
}

bool DocTreeSettings::TTFontBold() const {
    return ui->cbTTFontBold->isChecked();
}

void DocTreeSettings::SetTTFontBold(bool aTTFontBold) {
    ui->gbTypeTree->setVisible(true);
    ui->cbTTFontBold->setVisible(true);
    ui->cbTTFontBold->setChecked(aTTFontBold);
}

//-----------------------------------------
int DocTreeSettings::WindowTitleType() const {
    return ui->cbWindowTitle->currentIndex();
}
void DocTreeSettings::SetWindowTitleType(int aWindowTitleType) {
    ui->wdWindowTitle->setVisible(true);
    ui->cbWindowTitle->setCurrentIndex(aWindowTitleType);
}

bool DocTreeSettings::OpenSingleDocument() const {
    return ui->cbOpenSingle->isChecked();
}
void DocTreeSettings::SetOpenSingleDocument(bool aOpenSingleDocument) {
    ui->cbOpenSingle->setVisible(true);
    ui->cbOpenSingle->setChecked(aOpenSingleDocument);
}

bool DocTreeSettings::ShowGridLines() const {
    return ui->cbShowGrid->isChecked();
}
void DocTreeSettings::SetShowGridLines(bool aShowGridLines) {
//    ui->cbShowGrid->setVisible(true);
    ui->cbShowGrid->setChecked(aShowGridLines);
}

//bool DocTreeSettings::UniformRowHeights() const {
//    return ui->cbURH->isChecked();
//}
//void DocTreeSettings::SetUniformRowHeights(bool aUniformRowHeights) {
//    ui->cbURH->setChecked(aUniformRowHeights);
//}

bool DocTreeSettings::AutoWidth() const {
    return ui->cbAutoWidth->isChecked();
}
void DocTreeSettings::SetAutoWidth(bool aAutoWidth) {
    ui->cbAutoWidth->setVisible(true);
    ui->cbAutoWidth->setChecked(aAutoWidth);
}

bool DocTreeSettings::DocFontPlusOne() const {
    return ui->cbDocFontPlusOne->isChecked();
}
void DocTreeSettings::SetDocFontPlusOne(bool aDocFontPlusOne) {
    ui->cbDocFontPlusOne->setVisible(true);
    ui->cbDocFontPlusOne->setChecked(aDocFontPlusOne);
}

bool DocTreeSettings::DocFontBold() const {
    return ui->cbDocFontBold->isChecked();
}
void DocTreeSettings::SetDocFontBold(bool aDocFontBold) {
    ui->cbDocFontBold->setVisible(true);
    ui->cbDocFontBold->setChecked(aDocFontBold);
}

int DocTreeSettings::AddRowHeight() const {
    return ui->sbAddRowHeight->value();
}

void DocTreeSettings::SetAddRowHeight(int aAddRowHeight) {
    ui->wdAddRowHeight->setVisible(true);
    ui->sbAddRowHeight->setValue(aAddRowHeight);
}

bool DocTreeSettings::DragDrop() const {
    return ui->cbDragDrop->isChecked();
}
void DocTreeSettings::SetDragDrop(bool aDragDrop) {
    ui->cbDragDrop->setVisible(true);
    ui->cbDragDrop->setChecked(aDragDrop);
}

GlobalSettings::DocumentTreeStruct::DBLDOC DocTreeSettings::OnDocDblClick() const {
    return static_cast<GlobalSettings::DocumentTreeStruct::DBLDOC>(ui->cbDocDblClick->currentIndex());
}

void DocTreeSettings::SetOnDocDblClick(GlobalSettings::DocumentTreeStruct::DBLDOC aOnDocDblClick) {
    ui->wdDocDblClick->setVisible(true);
    ui->cbDocDblClick->setCurrentIndex(aOnDocDblClick);
}

GlobalSettings::DocumentTreeStruct::SLT DocTreeSettings::SecondLevel() const {
    return static_cast<GlobalSettings::DocumentTreeStruct::SLT>(ui->cbSecondLevel->currentIndex());
}

void DocTreeSettings::SetSecondLevel(GlobalSettings::DocumentTreeStruct::SLT aSecondLevel) {
    ui->wdSecondLevel->setVisible(true);
    ui->cbSecondLevel->setCurrentIndex(aSecondLevel);
}

bool DocTreeSettings::ExpandOnShow() const {
    return ui->cbExpandOnShow->isChecked();
}
void DocTreeSettings::SetExpandOnShow(bool aExpandOnShow) {
    ui->cbExpandOnShow->setVisible(true);
    ui->cbExpandOnShow->setChecked(aExpandOnShow);
}

void DocTreeSettings::GetColors(bool &aUseDocColor, QColor &aDocColor, bool &aUseLayoutColor, QColor &aLayoutColor) {
    aUseDocColor = ui->cbDocument->isChecked();
    aDocColor = mDocColor;
    aUseLayoutColor = ui->cbLayout->isChecked();
    aLayoutColor = mLayoutColor;
}

void DocTreeSettings::SetColors(bool aUseDocColor, const QColor &aDocColor, bool aUseLayoutColor, const QColor &aLayoutColor) {
    ui->gbColors->setVisible(true);

    ui->cbDocument->setChecked(aUseDocColor);
    mDocColor = aDocColor;
    ui->wDocument->setStyleSheet("background-color: rgb("
                                 + QString::number(mDocColor.red()) + ","
                                 + QString::number(mDocColor.green()) + ","
                                 + QString::number(mDocColor.blue()) + ");");

    ui->cbLayout->setChecked(aUseLayoutColor);
    mLayoutColor = aLayoutColor;
    ui->wLayout->setStyleSheet("background-color: rgb("
                               + QString::number(mLayoutColor.red()) + ","
                               + QString::number(mLayoutColor.green()) + ","
                               + QString::number(mLayoutColor.blue()) + ");");


}

void DocTreeSettings::on_pbDocument_clicked()
{
    QColorDialog cd(this);
    cd.setCurrentColor(mDocColor);
    if (cd.exec() == QDialog::Accepted) {
        mDocColor = cd.currentColor();
        ui->wDocument->setStyleSheet("background-color: rgb("
                                     + QString::number(mDocColor.red()) + ","
                                     + QString::number(mDocColor.green()) + ","
                                     + QString::number(mDocColor.blue()) + ");");
    }
}

void DocTreeSettings::on_pbLayout_clicked()
{
    QColorDialog cd(this);
    cd.setCurrentColor(mLayoutColor);
    if (cd.exec() == QDialog::Accepted) {
        mLayoutColor = cd.currentColor();
        ui->wLayout->setStyleSheet("background-color: rgb("
                                     + QString::number(mLayoutColor.red()) + ","
                                     + QString::number(mLayoutColor.green()) + ","
                                     + QString::number(mLayoutColor.blue()) + ");");
    }
}
