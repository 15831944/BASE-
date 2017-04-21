#include "ChooseAcadDlg.h"
#include "ui_ChooseAcadDlg.h"

#include <QInputDialog>

#include "common.h"
#include "GlobalSettings.h"

ChooseAcadDlg::ChooseAcadDlg(RunningAcadList *aRunningAcadList, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ChooseAcadDlg),
    mRunningAcadList(aRunningAcadList), mRunningAcadData(NULL),
    mPreviousRow(-1)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);


    int i;
    QList<AcadParamData *> &lAcadParams = gSettings->AcadParamsRef();

    for (i = lAcadParams.length() - 1; i >= 0; i--)
        if (!gSettings->InstalledAcadListConst().GetByProductName(lAcadParams.at(i)->FullProductNameConst()))
            lAcadParams.removeAt(i);
    for (i = 0; i < lAcadParams.length(); i++) {
        mParams.append(new AcadParamData(lAcadParams.at(i)));
    }


    // always selected in normal color (not gray)
    QPalette lPalette = ui->lwAutocads->palette();
    lPalette.setColor(QPalette::Inactive, QPalette::Highlight, lPalette.color(QPalette::Active, QPalette::Highlight));
    lPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, lPalette.color(QPalette::Active, QPalette::HighlightedText));
    ui->lwAutocads->setPalette(lPalette);
    // ---------------------------------
    lPalette = ui->pteFullCmdLine->palette();
    lPalette.setColor(QPalette::Base, palette().color(QPalette::Window));
    ui->pteFullCmdLine->setPalette(lPalette);

    ui->lblLOBBufferSize->setVisible(db.driverName() == "QOCI");
    ui->sbLOBBufferSize->setVisible(db.driverName() == "QOCI");

    ShowList();
    ui->cbAlwaysAsk->setCurrentIndex(gSettings->AcadParams.AlwaysAsk);
    ui->cbKeepAfterEdit->setChecked(gSettings->AcadParams.KeepAfterEdit);
    ui->cbKeepAllBackupsLocally->setEnabled(gSettings->AcadParams.KeepAfterEdit);
    ui->cbKeepAllBackupsLocally->setChecked(gSettings->AcadParams.KeepAllBackupsLocally);

    ui->tabWidget->setCurrentIndex(0);
}

ChooseAcadDlg::~ChooseAcadDlg()
{
    delete ui;
    qDeleteAll(mParams);
}

const RunningAcadData *ChooseAcadDlg::GetRunningAcadData() const {
    return mRunningAcadData;
}

void ChooseAcadDlg::ShowList() {
    int i;
    ui->lwAutocads->blockSignals(true);
    ui->lwAutocads->clear();
    ui->lwAutocads->blockSignals(false);

    if (mRunningAcadList) {
        for (i = 0; i < mRunningAcadList->length(); i++) {
            QListWidgetItem *lLWI = new QListWidgetItem("Running " + mRunningAcadList->at(i)->GetCaption());
            lLWI->setData(Qt::UserRole + 1, QVariant::fromValue(mRunningAcadList->at(i)));
            ui->lwAutocads->addItem(lLWI);
        }
    }

    for (i = 0; i < mParams.length(); i++) {
        QListWidgetItem *lLWI = new QListWidgetItem(mParams.at(i)->FullDisplayName());
        lLWI->setData(Qt::UserRole, QVariant::fromValue(mParams.at(i)));
        ui->lwAutocads->addItem(lLWI);
        if (i == gSettings->AcadParams.UseAcadParamIndex) {
            ui->lwAutocads->setCurrentRow(ui->lwAutocads->count() - 1);
        }
    }

    if (ui->lwAutocads->currentRow() < 0
            && ui->lwAutocads->currentRow() >= ui->lwAutocads->count()
            && ui->lwAutocads->count() > 0) {
        ui->lwAutocads->setCurrentRow(0);
    }
}

void ChooseAcadDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);
}

void ChooseAcadDlg::SavePreviousRow() {
    AcadParamData *lAcadParamData;
    if (mPreviousRow >= 0
            && mPreviousRow < ui->lwAutocads->count()
            && (lAcadParamData = ui->lwAutocads->item(mPreviousRow)->data(Qt::UserRole).value<AcadParamData *>())) {
        lAcadParamData->SetStartWithNoLogo(ui->cbNoLogo->isChecked()?1:0);
        if (ui->cbLoadAecBase->isVisible()) lAcadParamData->SetLoadAecBase(ui->cbLoadAecBase->isChecked()?1:0);

        if (lAcadParamData->UserProfileConst() == lAcadParamData->AddNameConst()) {
            if (ui->cbProfile->currentIndex() > 0) {
                lAcadParamData->SetAddName(ui->cbProfile->currentText());
            } else {
                lAcadParamData->SetAddName("");
            }
            ui->lwAutocads->item(mPreviousRow)->setText(lAcadParamData->FullDisplayName());
        }
        if (ui->cbProfile->currentIndex() > 0) {
            lAcadParamData->SetUserProfile(ui->cbProfile->currentText());
        } else {
            lAcadParamData->SetUserProfile("");
        }
        lAcadParamData->SetCommandLine(ui->leCommandLine->text());

        lAcadParamData->SetDelayAfterStart(ui->sbDelayAfterStart->value());
        lAcadParamData->SetMaxWaitForStart(ui->sbMaxWaitForStart->value());
        lAcadParamData->SetStartCheckInterval(ui->leStartCheckInterval->text().toUInt());
        lAcadParamData->SetDelayAfterOpen(ui->leDelayAfterOpen->text().toUInt());
        lAcadParamData->SetLOBBufferSize(ui->sbLOBBufferSize->value());
    }
}

void ChooseAcadDlg::RestoreCurrentRow() {
    const AcadParamData *lAcadParamData;
    if (ui->lwAutocads->currentItem()
            && (lAcadParamData = ui->lwAutocads->currentItem()->data(Qt::UserRole).value<AcadParamData *>())) {
        ui->stack->setCurrentIndex(0);
        const InstalledAcadData *lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(lAcadParamData->FullProductNameConst());

        ui->cbNoLogo->setChecked(lAcadParamData->StartWithNoLogo());
        ui->cbLoadAecBase->setVisible(lInstalledAcadData->ProductConst() == "C3D");
        if (lInstalledAcadData->ProductConst() == "C3D") {
            ui->cbLoadAecBase->setChecked(lAcadParamData->LoadAecBase());
        } else {
            ui->cbLoadAecBase->setChecked(false);
        }
        ui->cbProfile->clear();
        ui->cbProfile->addItem("<NOT SPECIFIED>");
        ui->cbProfile->addItems(lInstalledAcadData->Profiles());
        if (lAcadParamData->UserProfileConst().isEmpty()) {
            ui->cbProfile->setCurrentIndex(0);
        } else {
            ui->cbProfile->setCurrentText(lAcadParamData->UserProfileConst());
        }
        ui->leCommandLine->setText(lAcadParamData->CommandLineConst());

        ui->sbDelayAfterStart->setValue(lAcadParamData->DelayAfterStart());
        ui->sbMaxWaitForStart->setValue(lAcadParamData->MaxWaitForStart());
        ui->leStartCheckInterval->setText(QString::number(lAcadParamData->StartCheckInterval()));
        ui->leDelayAfterOpen->setText(QString::number(lAcadParamData->DelayAfterOpen()));
        ui->sbLOBBufferSize->setValue(lAcadParamData->LOBBufferSize());

        ShowFullCommandLine();
    } else {
        ui->stack->setCurrentIndex(1); // empty page
    }
}

void ChooseAcadDlg::ShowFullCommandLine() {
    QString lCmdLine;

    const AcadParamData *lAcadParamData;
    const InstalledAcadData *lInstalledAcadData;

    if (ui->lwAutocads->currentItem()
            && (lAcadParamData = ui->lwAutocads->currentItem()->data(Qt::UserRole).value<AcadParamData *>())
            && (lInstalledAcadData = gSettings->InstalledAcadListConst().GetByProductName(lAcadParamData->FullProductNameConst()))) {
        lCmdLine = lInstalledAcadData->FullCommandLine();
        if (ui->cbNoLogo->isChecked()) lCmdLine += " /nologo";
        if (ui->cbLoadAecBase->isChecked())
            lCmdLine += " /ld \"" + lInstalledAcadData->PathConst() + "\\AecBase.dbx\"";
        if (ui->cbProfile->currentIndex() > 0)
            lCmdLine += " /p \"" + ui->cbProfile->currentText() + "\"";
        if (!ui->leCommandLine->text().isEmpty()) lCmdLine += " " + ui->leCommandLine->text();
    }
    ui->pteFullCmdLine->setPlainText(lCmdLine);
}

void ChooseAcadDlg::Accept() {
    SavePreviousRow();

    QList<AcadParamData *> &lAcadParams = gSettings->AcadParamsRef();

    for (int i = 0; i < mParams.length(); i++) {
        if (i < lAcadParams.length()) {
            lAcadParams.at(i)->CopyFromOther(mParams.at(i));
        } else {
            lAcadParams.append(new AcadParamData(mParams.at(i)));
        }
        if (ui->lwAutocads->currentItem()
                && ui->lwAutocads->currentItem()->data(Qt::UserRole).value<AcadParamData *>() == mParams.at(i)) {
            gSettings->AcadParams.UseAcadParamIndex = i;
        }
    }

    if (mRunningAcadList
            && ui->lwAutocads->currentItem()
            && ui->lwAutocads->currentItem()->data(Qt::UserRole + 1).value<RunningAcadData *>()) {
        mRunningAcadData = ui->lwAutocads->currentItem()->data(Qt::UserRole + 1).value<RunningAcadData *>();
    }
    while (mParams.length() < lAcadParams.length()) delete lAcadParams.takeLast();

    gSettings->AcadParams.AlwaysAsk = ui->cbAlwaysAsk->currentIndex();
    gSettings->AcadParams.KeepAfterEdit = ui->cbKeepAfterEdit->isChecked()?1:0;
    gSettings->AcadParams.KeepAllBackupsLocally = ui->cbKeepAllBackupsLocally->isChecked()?1:0;

    gSettings->SaveAcadParams();
    accept();
}

void ChooseAcadDlg::on_lwAutocads_doubleClicked(const QModelIndex &index) {
    Accept();
}

void ChooseAcadDlg::on_lwAutocads_currentRowChanged(int currentRow) {
    const AcadParamData *lAcadParamDataCurrent, *lAcadParamData;

    SavePreviousRow();
    RestoreCurrentRow();
    mPreviousRow = currentRow;
    bool lIsFound = false;
    if (ui->lwAutocads->currentItem()
            && (lAcadParamDataCurrent = ui->lwAutocads->currentItem()->data(Qt::UserRole).value<AcadParamData *>())) {
        for (int i = 0; i < ui->lwAutocads->count(); i++) {
            if ((lAcadParamData = ui->lwAutocads->item(i)->data(Qt::UserRole).value<AcadParamData *>())
                    && lAcadParamDataCurrent != lAcadParamData
                    && lAcadParamDataCurrent->FullProductNameConst() == lAcadParamData->FullProductNameConst()) {
                lIsFound = true;
                break;
            }
        }
    }
    ui->pbRename->setEnabled(lIsFound);
    ui->pbRemove->setEnabled(lIsFound);
}

void ChooseAcadDlg::on_cbKeepAfterEdit_toggled(bool checked) {
    ui->cbKeepAllBackupsLocally->setEnabled(checked);
}

void ChooseAcadDlg::on_leCommandLine_textEdited(const QString &arg1) {
    ShowFullCommandLine();
}

void ChooseAcadDlg::on_cbNoLogo_toggled(bool checked) {
    ShowFullCommandLine();
}

void ChooseAcadDlg::on_cbProfile_currentIndexChanged(int index){
    ShowFullCommandLine();
}

void ChooseAcadDlg::on_pbCopy_clicked() {
    const AcadParamData *lAcadParamData;

    if (ui->lwAutocads->currentItem()
            && (lAcadParamData = ui->lwAutocads->currentItem()->data(Qt::UserRole).value<AcadParamData *>())) {
        AcadParamData *lAcadParamDataNew = new AcadParamData(lAcadParamData);
        lAcadParamDataNew->SetAddName(lAcadParamDataNew->AddNameConst().isEmpty()?"1":lAcadParamDataNew->AddNameConst() + " - 1");

        bool lIsFound = false;
        for (int i = 0; i < mParams.length(); i++) {
            if (mParams.at(i) == lAcadParamData) {
                mParams.insert(i + 1, lAcadParamDataNew);

                QListWidgetItem *lLWI = new QListWidgetItem(lAcadParamDataNew->FullDisplayName());
                lLWI->setData(Qt::UserRole, QVariant::fromValue(lAcadParamDataNew));
                ui->lwAutocads->insertItem(ui->lwAutocads->currentRow() + 1, lLWI);
                lIsFound = true;
                break;
            }
        }
        if (!lIsFound) delete lAcadParamDataNew;
    }
}

void ChooseAcadDlg::on_pbRename_clicked() {
    AcadParamData *lAcadParamData;

    if (ui->lwAutocads->currentItem()
            && (lAcadParamData = ui->lwAutocads->currentItem()->data(Qt::UserRole).value<AcadParamData *>())) {

        QInputDialog lDlg(this);
        lDlg.setWindowFlags(lDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        lDlg.setWindowTitle(tr("Renaming configuration"));
        lDlg.setLabelText(tr("Enter additional name for configuration") + " '" + lAcadParamData->FullDisplayName() + "'");
        lDlg.setTextValue(lAcadParamData->AddNameConst());
        if (lDlg.exec() == QDialog::Accepted) {
            lAcadParamData->SetAddName(lDlg.textValue());
            ui->lwAutocads->item(ui->lwAutocads->currentRow())->setText(lAcadParamData->FullDisplayName());
        }
    }
}

void ChooseAcadDlg::on_pbRemove_clicked() {
    AcadParamData *lAcadParamDataCurrent;
    const AcadParamData *lAcadParamData;

    bool lIsFound = false;
    if (ui->lwAutocads->currentItem()
            && (lAcadParamDataCurrent = ui->lwAutocads->currentItem()->data(Qt::UserRole).value<AcadParamData *>())) {
        for (int i = 0; i < ui->lwAutocads->count(); i++) {
            if ((lAcadParamData = ui->lwAutocads->item(i)->data(Qt::UserRole).value<AcadParamData *>())
                    && lAcadParamDataCurrent != lAcadParamData
                    && lAcadParamDataCurrent->FullProductNameConst() == lAcadParamData->FullProductNameConst()) {
                lIsFound = true;
                break;
            }
        }
    }

    if (lIsFound
            && QMessageBox::question(this, tr("AutoCAD settings"), tr("Remove configuration")
                                     + "\n'" + lAcadParamDataCurrent->FullDisplayName() + "'?") == QMessageBox::Yes) {
        ui->lwAutocads->blockSignals(true);
        for (int i = ui->lwAutocads->count() - 1; i >= 0; i--)
            if (ui->lwAutocads->item(i)->data(Qt::UserRole).value<AcadParamData *>() == lAcadParamDataCurrent)
                delete ui->lwAutocads->takeItem(i);
        ui->lwAutocads->blockSignals(false);
        mParams.removeAll(lAcadParamDataCurrent);
        delete lAcadParamDataCurrent;
        mPreviousRow = -1;
        emit ui->lwAutocads->currentRowChanged(ui->lwAutocads->currentRow());
    }
}

void ChooseAcadDlg::on_pbResetAll_clicked() {
    if (QMessageBox::question(this, tr("AutoCAD settings"), tr("Reset all configurations?")) == QMessageBox::Yes) {
        qDeleteAll(mParams);
        mParams.clear();
        gSettings->InitAcadParamsList(mParams);
        mPreviousRow = -1;
        ShowList();
    }
}
