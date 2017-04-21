#include <QColorDialog>
#include "ContractDefVAT.h"
#include "ui_ContractDefVAT.h"

#include "../VProject/GlobalSettings.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

ContractDefVAT::ContractDefVAT(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ContractDefVAT),
    AnyChanged(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));
}

ContractDefVAT::~ContractDefVAT()
{
    delete ui;
}

void ContractDefVAT::showEvent(QShowEvent* event)
{
    QFCDialog::showEvent(event);

    //ui->leNDS->setText(QString::number(gSettings->NDS));

    ui->cbProject->setChecked(gSettings->Contract.UseProjectColor);
    ui->pbProject->setEnabled(gSettings->Contract.UseProjectColor);
    ui->cbContract->setChecked(gSettings->Contract.UseContractColor);
    ui->pbContract->setEnabled(gSettings->Contract.UseContractColor);
    ui->cbStage->setChecked(gSettings->Contract.UseStageColor);
    ui->pbStage->setEnabled(gSettings->Contract.UseStageColor);

    mProjectColor = gSettings->Contract.ProjectColor;
    mContractColor = gSettings->Contract.ContractColor;
    mStageColor = gSettings->Contract.StageColor;

    ui->wProject->setStyleSheet("background-color: rgb("
                                + QString::number(mProjectColor.red()) + ","
                                + QString::number(mProjectColor.green()) + ","
                                + QString::number(mProjectColor.blue()) + ");");
    ui->wContract->setStyleSheet("background-color: rgb("
                                 + QString::number(mContractColor.red()) + ","
                                 + QString::number(mContractColor.green()) + ","
                                 + QString::number(mContractColor.blue()) + ");");
    ui->wStage->setStyleSheet("background-color: rgb("
                              + QString::number(mStageColor.red()) + ","
                              + QString::number(mStageColor.green()) + ","
                              + QString::number(mStageColor.blue()) + ");");

    ui->cbWhenStart->setCurrentIndex(gSettings->Contract.ExpandOnStart);
    ui->cbMultiSelect->setChecked(gSettings->Contract.MultiSelect);
}


void ContractDefVAT::Accept() {
    bool lDoAccept = true;

    /*if (ui->leNDS->text().toDouble() != gSettings->NDS) {
        QSqlQuery qUpdate(db);
        qUpdate.prepare("update v_pkz_defvat set thevalue = ?");
        if (qUpdate.lastError().isValid()) {
            gLogger->ShowSqlError(this, "VAT", qUpdate);
            lDoAccept = false;
        } else {
            qUpdate.addBindValue(ui->leNDS->text().toDouble());
            if (!qUpdate.exec()) {
                gLogger->ShowSqlError(this, "VAT", qUpdate);
                lDoAccept = false;
            } else {
                gSettings->NDS = ui->leNDS->text().toDouble();
            }
        }
    }*/

    if (gSettings->Contract.UseProjectColor != (ui->cbProject->checkState() == Qt::Checked)) {
        gSettings->Contract.UseProjectColor = (ui->cbProject->checkState() == Qt::Checked);
        AnyChanged = true;
    }

    if (gSettings->Contract.ProjectColor != mProjectColor) {
        gSettings->Contract.ProjectColor = mProjectColor;
        AnyChanged = true;
    }

    if (gSettings->Contract.UseContractColor != (ui->cbContract->checkState() == Qt::Checked)) {
        gSettings->Contract.UseContractColor = (ui->cbContract->checkState() == Qt::Checked);
        AnyChanged = true;
    }

    if (gSettings->Contract.ContractColor != mContractColor) {
        gSettings->Contract.ContractColor = mContractColor;
        AnyChanged = true;
    }

    if (gSettings->Contract.UseStageColor != (ui->cbStage->checkState() == Qt::Checked)) {
        gSettings->Contract.UseStageColor = (ui->cbStage->checkState() == Qt::Checked);
        AnyChanged = true;
    }

    if (gSettings->Contract.StageColor != mStageColor) {
        gSettings->Contract.StageColor = mStageColor;
        AnyChanged = true;
    }

    if (gSettings->Contract.ExpandOnStart != ui->cbWhenStart->currentIndex()) {
        gSettings->Contract.ExpandOnStart = ui->cbWhenStart->currentIndex();
    }

    if (gSettings->Contract.MultiSelect != (ui->cbMultiSelect->checkState() == Qt::Checked)) {
        gSettings->Contract.MultiSelect = (ui->cbMultiSelect->checkState() == Qt::Checked);
    }
    if (lDoAccept) accept();
}

void ContractDefVAT::on_pbProject_clicked()
{
    QColorDialog cd(this);
    cd.setCurrentColor(mProjectColor);
    if (cd.exec() == QDialog::Accepted) {
        mProjectColor = cd.currentColor();
        ui->wProject->setStyleSheet("background-color: rgb("
                                    + QString::number(mProjectColor.red()) + ","
                                    + QString::number(mProjectColor.green()) + ","
                                    + QString::number(mProjectColor.blue()) + ");");
    }
}

void ContractDefVAT::on_pbContract_clicked()
{
    QColorDialog cd(this);
    cd.setCurrentColor(mContractColor);
    if (cd.exec() == QDialog::Accepted) {
        mContractColor = cd.currentColor();
        ui->wContract->setStyleSheet("background-color: rgb("
                                    + QString::number(mContractColor.red()) + ","
                                    + QString::number(mContractColor.green()) + ","
                                    + QString::number(mContractColor.blue()) + ");");
    }
}

void ContractDefVAT::on_pbStage_clicked()
{
    QColorDialog cd(this);
    cd.setCurrentColor(mStageColor);
    if (cd.exec() == QDialog::Accepted) {
        mStageColor = cd.currentColor();
        ui->wStage->setStyleSheet("background-color: rgb("
                                    + QString::number(mStageColor.red()) + ","
                                    + QString::number(mStageColor.green()) + ","
                                    + QString::number(mStageColor.blue()) + ");");
    }
}

void ContractDefVAT::on_cbProject_stateChanged(int arg1)
{
    ui->pbProject->setEnabled(ui->cbProject->checkState() == Qt::Checked);
}

void ContractDefVAT::on_cbContract_stateChanged(int arg1)
{
    ui->pbContract->setEnabled(ui->cbContract->checkState() == Qt::Checked);
}

void ContractDefVAT::on_cbStage_stateChanged(int arg1)
{
    ui->pbStage->setEnabled(ui->cbStage->checkState() == Qt::Checked);
}
