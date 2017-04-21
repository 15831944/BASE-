#include "CommonSettingsDlg.h"
#include "ui_CommonSettingsDlg.h"

#include <QFontDialog>
#include <QStyleFactory>

CommonSettingsDlg::CommonSettingsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommonSettingsDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    //ui->cbStyle->addItem("");
    ui->cbStyle->addItems(QStyleFactory::keys());
}

CommonSettingsDlg::~CommonSettingsDlg()
{
    delete ui;
}

QString CommonSettingsDlg::VisualStyle() const {
    return ui->cbStyle->currentText();
}

void CommonSettingsDlg::SetVisualStyle(const QString &aStyle) {
    ui->cbStyle->setCurrentText(aStyle);
}

bool CommonSettingsDlg::ConfirmQuit() const {
    return ui->cbConfirmQuit->isChecked();
}
void CommonSettingsDlg::SetConfirmQuit(bool aConfirmQuit) {
    ui->cbConfirmQuit->setChecked(aConfirmQuit);
}

bool CommonSettingsDlg::UseTabbedView() const {
    return ui->cbUseTabbedView->isChecked();
}

void CommonSettingsDlg::SetUseTabbedView(bool aUseTabbedView) {
    ui->cbUseTabbedView->setChecked(aUseTabbedView);

    ui->lblTabPos->setEnabled(aUseTabbedView);
    ui->cbTabPos->setEnabled(aUseTabbedView);
}

QTabWidget::TabPosition CommonSettingsDlg::TabPos() const {
    return static_cast<QTabWidget::TabPosition>(ui->cbTabPos->currentIndex());
}

void CommonSettingsDlg::SetTabPos(QTabWidget::TabPosition aTabPos) {
    ui->cbTabPos->setCurrentIndex(aTabPos);
}

bool CommonSettingsDlg::SaveWinState() const {
    return ui->cbSaveWinState->isChecked();
}
void CommonSettingsDlg::SetSaveWinState(bool aSaveWinState) {
    ui->cbSaveWinState->setChecked(aSaveWinState);
}

QFont CommonSettingsDlg::Font() const {
    return mFont;
}

void CommonSettingsDlg::SetFont(const QFont &lFont) {
    mFont = lFont;
}

int CommonSettingsDlg::AddRowHeight() const {
    return ui->sbAddRowHeight->value();
}

void CommonSettingsDlg::SetAddRowHeight(int aAddRowHeight) {
    ui->sbAddRowHeight->setValue(aAddRowHeight);
}


void CommonSettingsDlg::on_cbUseTabbedView_toggled(bool checked) {
    ui->lblTabPos->setEnabled(checked);
    ui->cbTabPos->setEnabled(checked);
}

bool CommonSettingsDlg::ShowAfterCopy() const {
    return ui->cbShowAfterCopy->isChecked();
}

void CommonSettingsDlg::SetShowAfterCopy(bool aShowAfterCopy) {
    ui->cbShowAfterCopy->setChecked(aShowAfterCopy);
}

void CommonSettingsDlg::on_pbSelectFont_clicked() {
    QFontDialog fd(this);

    fd.setCurrentFont(mFont);
    if (fd.exec() == QDialog::Accepted) {
        mFont = fd.currentFont();
    }

}
