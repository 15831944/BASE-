#include "DwgCmpSettingsDlg.h"
#include "ui_DwgCmpSettingsDlg.h"

#include "../VProject/GlobalSettings.h"

#include <QFileDialog>

DwgCmpSettingsDlg::DwgCmpSettingsDlg(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::DwgCmpSettingsDlg),
    mJustStarted(true)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    QIntValidator *lIntVal = new QIntValidator(1, 255, this);
    ui->leOldColor->setValidator(lIntVal);
    ui->leNewColor->setValidator(lIntVal);

    ui->leTextHeight->setValidator(new QDoubleValidator(0.0001, 1000, 4, this));
}

DwgCmpSettingsDlg::~DwgCmpSettingsDlg()
{
    delete ui;
}

void DwgCmpSettingsDlg::FirstInit() {
    ui->leOldColor->setText(QString::number(gSettings->Compare.OldColor));
    ui->leNewColor->setText(QString::number(gSettings->Compare.NewColor));

    ui->gpOutputDates->setChecked(gSettings->Compare.OutputDates);
    ui->leTextHeight->setText(QString::number(gSettings->Compare.TextHeight));

    ui->cbAskDir->setChecked(gSettings->Compare.AlwaysAskOutputDir);
    ui->leOutputDir->setText(gSettings->Compare.OutputDir);

    ui->cbAlwaysShow->setChecked(gSettings->Compare.AlwaysAskAll);
}

void DwgCmpSettingsDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        QTimer::singleShot(0, this, SLOT(FirstInit()));
        mJustStarted = false;
    }
}

void DwgCmpSettingsDlg::on_pbOK_clicked() {
    if (ui->leOldColor->text() == ui->leNewColor->text()) {
        QMessageBox::critical(this, tr("Comparing settings"), tr("Colors must be different!"));
        ui->leOldColor->setFocus();
        return;
    }

    if (!ui->cbAskDir->isChecked()
            && ui->leOutputDir->text().isEmpty()) {
        QMessageBox::critical(this, tr("Comparing settings"), tr("Output directory must be specified!"));
        ui->leOutputDir->setFocus();
        return;

    }

    gSettings->Compare.OldColor = ui->leOldColor->text().toUInt();
    gSettings->Compare.NewColor = ui->leNewColor->text().toUInt();

    gSettings->Compare.OutputDates = ui->gpOutputDates->isChecked();
    gSettings->Compare.TextHeight = ui->leTextHeight->text().toDouble();

    gSettings->Compare.AlwaysAskOutputDir = ui->cbAskDir->isChecked();
    gSettings->Compare.OutputDir = ui->leOutputDir->text();

    gSettings->Compare.AlwaysAskAll = ui->cbAlwaysShow->isChecked();
    accept();
}

void DwgCmpSettingsDlg::on_tbSelectPath_clicked() {
    QFileDialog dlg(this);
    dlg.setFileMode(QFileDialog::DirectoryOnly);

    dlg.setDirectory(ui->leOutputDir->text());
    if (dlg.exec() == QDialog::Accepted) {
        ui->leOutputDir->setText(dlg.selectedFiles().at(0));
    }
}
