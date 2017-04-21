#include "PlotProp.h"
#include "ui_PlotProp.h"
#include "../VProject/typetreeselect.h"
#include "../VProject/TreeData.h"
#include "../VProject/PlotListItemDelegate.h"
#include "../VProject/GlobalSettings.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserRight.h"
#include "../ProjectLib/ProjectData.h"
#include "../ProjectLib/ProjectTypeData.h"
#include "../ProjectLib/ProjectListDlg.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>

PlotProp::PlotProp(PlotData * aPlotData, PlotHistoryData * aPlotHistoryData, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::PlotProp),
    mPlotData(aPlotData), mNumHistData(0), mTreeData(NULL),
    mSheetSetted(false),
    mJustStarted(true)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    //connect(ui->leIdProject, SIGNAL(editingFinished()), this, SLOT(IdProjectChanged()));

    if (aPlotHistoryData) mNumHistData = aPlotHistoryData->Num();


    // read only
    QPalette lPaletteDis = ui->leIdProject->palette();
    lPaletteDis.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);
    ui->leId->setPalette(lPaletteDis);
    ui->leHistory->setPalette(lPaletteDis);
    ui->leExt->setPalette(lPaletteDis);
    ui->leCrDate->setPalette(lPaletteDis);
    ui->leCrUser->setPalette(lPaletteDis);
    ui->leChDate->setPalette(lPaletteDis);
    ui->leChUser->setPalette(lPaletteDis);
    ui->leAcadVer->setPalette(lPaletteDis);
    ui->leXrefTo->setPalette(lPaletteDis);
    ui->leXrefFor->setPalette(lPaletteDis);
    ui->leStatus->setPalette(lPaletteDis);
    ui->leMainSize->setPalette(lPaletteDis);
    ui->leOtherSize->setPalette(lPaletteDis);

    // required
    QPalette lPaletteReq = ui->leIdProject->palette();
    lPaletteReq.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    ui->leIdProject->setPalette(lPaletteReq);
    ui->leProjName->setPalette(lPaletteReq);
    ui->leTypeText->setPalette(lPaletteReq);
    if (gUserRight->CanUpdate("v_plot_simple", "version")) {
        ui->leVersionInt->setPalette(lPaletteReq);
    } else {
        ui->leVersionInt->setPalette(lPaletteDis);
    }
    if (gUserRight->CanUpdate("v_plot_simple", "version_ext")) {
        ui->leVersionExt->setPalette(lPaletteReq);
    } else {
        ui->leVersionExt->setPalette(lPaletteDis);
    }
    if (gUserRight->CanUpdate("v_plot_simple", "code")) {
        ui->leCode->setPalette(lPaletteReq);
    } else {
        ui->leCode->setPalette(lPaletteDis);
    }
    ui->cbCode->setPalette(lPaletteReq);
    ui->teBottomName->setPalette(lPaletteReq);

    for (int i = 0; i < gUsers->UsersConst().length(); i++)
        if (!gUsers->UsersConst().at(i)->Disabled())
            ui->cbSentUser->addItem(gUsers->UsersConst().at(i)->NameConst(), gUsers->UsersConst().at(i)->LoginConst());

    ui->leIdProject->setValidator(new QIntValidator(1, 1e9, this));

    PlotListItemDelegate *lDelegate = new ROPlotListItemDelegate(this);
    ui->twComments->setItemDelegate(lDelegate);
}

PlotProp::~PlotProp()
{
    delete ui;
}

void PlotProp::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;
        ShowData();
    }
}

void PlotProp::ShowData() {
    if (mPlotData) {

        mPlotData->RefreshData();

        mPlotData->InitIdDwgMax();
        mPlotData->InitEditStatus();
        mPlotData->InitAcadVer();

        mVerPrev = mPlotData->VersionExtConst();
        mStagePrev = mPlotData->StageConst();
        mComplectPrev = mPlotData->SectionConst();

        bool lWasSent = !mPlotData->SentDateConst().isNull(); // IsSent();

        ui->leId->setText(QString::number(mPlotData->Id()));

        PlotHistoryData *lHistory = mNumHistData?mPlotData->GetHistoryByNum(mNumHistData):NULL;
        if (!lHistory) {
            ui->leHistory->setText(QString::number(mPlotData->DwgVersionMax()));
        } else {
            ui->leHistory->setText(QString::number(lHistory->Num())+ "/" + QString::number(mPlotData->DwgVersionMax()));
        }

        ui->leIdProject->setText(QString::number(mPlotData->IdProject()));
        ProjectData * lProjectData = gProjects->FindByIdProject(mPlotData->IdProject());
        if (lProjectData) {
            ui->leProjName->setText(lProjectData->FullShortName());
        } else {
            ui->leProjName->setText("");
        }

        mTreeData = gTreeData->FindById(mPlotData->TDArea(), mPlotData->TDId());

        if (mTreeData)
            ui->leTypeText->setText(mTreeData->FullName());

        ui->leVersionInt->setReadOnly(lWasSent || !gUserRight->CanUpdate("v_plot_simple", "version"));
        ui->leVersionInt->setText(mPlotData->VersionIntConst());

        ui->leVersionExt->setReadOnly(lWasSent || !gUserRight->CanUpdate("v_plot_simple", "version_ext"));
        ui->leVersionExt->setText(mPlotData->VersionExtConst());

        ui->cbStage->blockSignals(true);
        ui->cbSection->blockSignals(true);

        ui->cbSection->setVisible(!lWasSent);
        ui->leSection->setVisible(lWasSent);

        ui->cbStage->setVisible(!lWasSent);
        ui->leStage->setVisible(lWasSent);
        // it is switching beetwen ComboBox (for edit) and LineEdit (for read only)
        if (!lWasSent) {
            ui->cbSection->clear();
            if (lProjectData) {
                ui->cbSection->addItems(lProjectData->ComplectListConst());
            }
            ui->cbSection->setCurrentText(mPlotData->SectionConst());

            ui->cbStage->clear();
            if (lProjectData) {
                ui->cbStage->addItems(gProjectTypes->GetById(lProjectData->IdProjType())->StagesConst());
                if (ui->cbStage->count()) ui->cbStage->insertItem(0, "");
                ui->cbStage->setEditable(ui->cbStage->count() < 2);
            }
            ui->cbStage->setCurrentText(mPlotData->StageConst());

        } else {
            ui->leSection->setText(mPlotData->SectionConst());
            ui->leStage->setText(mPlotData->StageConst());
        }
        ui->cbSection->blockSignals(false);
        ui->cbStage->blockSignals(false);

        // sheet before code
        bool lCanSheetBeVisible;
        if (lProjectData) {
            lCanSheetBeVisible = lProjectData->SheetDigitsActual() != -1;
        } else {
            lCanSheetBeVisible = true;
        }
        if (!lCanSheetBeVisible) {
            ui->lblSheet->setVisible(false);
            ui->leSheet->setVisible(false);
            ui->cbSheet->setVisible(false);
        }

        // code
        if (mPlotData->CodeConst().startsWith(lProjectData->ProjectType()->NoNumTemplConst().left(lProjectData->ProjectType()->NoNumTemplConst().indexOf("%")))
                && !lWasSent
                && gUserRight->CanUpdate("v_plot_simple", "code")) {
            // lists
            if (lCanSheetBeVisible) {
                ui->cbSheet->setCurrentText(mPlotData->SheetConst());
                ui->leSheet->setVisible(false);
            }

            // need regenlist
            RegenCodeList();

            int lFound;

            if ((lFound = ui->cbCode->findData(mPlotData->CodeConst(), Qt::DisplayRole)) == -1) {
                ui->cbCode->insertItem(0, mPlotData->CodeConst());
                lFound = 0;
            }
            ui->cbCode->setCurrentIndex(lFound);
            //ui->cbCode->setCurrentText(mPlotData->CodeConst());
            ui->leCode->setVisible(false);
        } else {
            // simple edits
            ui->leCode->setReadOnly(lWasSent || !gUserRight->CanUpdate("v_plot_simple", "code"));
            ui->leCode->setText(mPlotData->CodeConst());
            ui->cbCode->setVisible(false);

            if (lCanSheetBeVisible) {
                ui->leSheet->setReadOnly(lWasSent);
                ui->leSheet->setText(mPlotData->SheetConst());
                ui->cbSheet->setVisible(false);
            }
        }

        ui->leBlockName->setText(mPlotData->BlockNameConst());
        if ((mPlotData->FileType() < 20 || mPlotData->FileType() > 29) && mPlotData->ExtensionConst().toLower() == "dwg") {
            ui->leBlockName->setVisible(true);
            ui->lblBlockName->setVisible(true);
        } else {
            ui->leBlockName->setVisible(false);
            ui->lblBlockName->setVisible(false);
        }
        ui->leExt->setText(mPlotData->ExtensionConst());

        ui->leCrDate->setText(mPlotData->CrDateConst().toString("dd.MM.yy HH:mm"));
        ui->leCrUser->setText(gUsers->GetName(mPlotData->CrUserConst()));

        ui->leChDate->setText(mPlotData->EditDateConst().toString("dd.MM.yy HH:mm"));
        ui->leChUser->setText(gUsers->GetName(mPlotData->EditUserConst()));

        ui->cbSentUser->setVisible(false); // anyway


        ui->dteSentDate->setVisible(lWasSent);
        ui->dteSentDate->setReadOnly(lWasSent);
        ui->leSentUser->setVisible(lWasSent);

        ui->cbSent->blockSignals(true);
        ui->cbSent->setChecked(lWasSent);
        ui->cbSent->setReadOnly(lWasSent); // can't uncheck
        ui->cbSent->blockSignals(false);

        if (lWasSent) {
            // document was sent
            ui->dteSentDate->setDate(mPlotData->SentDateConst());
            // it is read-only always
            ui->leSentUser->setText(gUsers->GetName(mPlotData->SentUserConst()));
        } else {
            // document was not sent
            ui->dteSentDate->setDate(QDate::currentDate()); // set current date
            // set current user
            ui->cbSentUser->setCurrentText(gUsers->GetName(db.userName()));
        }

        ui->cbWorking->setChecked(mPlotData->Working());

        bool lIsAcad = lHistory?lHistory->AcadVer():mPlotData->AcadVer();

        if (!lHistory) {
            ui->leAcadVer->setText(mPlotData->AcadVerStr());
            ui->leXrefTo->setText(QString::number(mPlotData->XrefsCnt()));
        } else {
            ui->leAcadVer->setText(lHistory->AcadVerStr());
            ui->leXrefTo->setText(QString::number(lHistory->XrefsCnt()));
        }

        ui->lblAcadVer->setVisible(lIsAcad);
        ui->leAcadVer->setVisible(lIsAcad);
        ui->lblXrefTo->setVisible(lIsAcad);
        ui->leXrefTo->setVisible(lIsAcad);
        ui->lblXrefFor->setVisible(lIsAcad);
        ui->leXrefFor->setVisible(lIsAcad);
        if (!lIsAcad) {
            ui->vlAcad->insertStretch(-1, 10);
        }
        ui->hlBlockName->invalidate();
        ui->vlBlockName->invalidate();

        // xrefs counts

        ui->teTopName->setReadOnly(lWasSent);
        ui->teTopName->setPlainText(mPlotData->NameTopConst());

        ui->teBottomName->setReadOnly(lWasSent);
        ui->teBottomName->setPlainText(mPlotData->NameConst());

        ui->teNote->setReadOnly(lWasSent);
        ui->teNote->setPlainText(mPlotData->NotesConst());

        if (mPlotData->Cancelled()) {
            ui->cbCancelled->setChecked(true);
            ui->dteCancDate->setVisible(true);
            ui->leCancUser->setVisible(true);

            ui->dteCancDate->setDate(mPlotData->CancelDateConst());
            ui->leCancUser->setText(gUsers->GetName(mPlotData->CancelUserConst()));
        } else {
            ui->cbCancelled->setChecked(false);
            ui->dteCancDate->setVisible(false);
            ui->leCancUser->setVisible(false);

            ui->dteCancDate->setDate(QDate::currentDate());
            ui->leCancUser->setText(gUsers->GetName(db.userName()));
        }

        // can't delete from here
        ui->cbDeleted->setReadOnly(/*lWasSent*/true);
        if (mPlotData->Deleted()) {
            ui->cbDeleted->setChecked(true);
            ui->dteDeleteDate->setVisible(true);
            ui->leDeleteUser->setVisible(true);

            ui->dteDeleteDate->setDate(mPlotData->DeleteDateConst());
            ui->leDeleteUser->setText(gUsers->GetName(mPlotData->DeleteUserConst()));
        } else {
            ui->cbDeleted->setChecked(false);
            ui->dteDeleteDate->setVisible(false);
            ui->leDeleteUser->setVisible(false);

            ui->dteDeleteDate->setDate(QDate::currentDate());
            ui->leDeleteUser->setText(gUsers->GetName(db.userName()));
        }

        switch (mPlotData->ES()) {
        case PlotData::PESFree:
            ui->leStatus->setText(tr("Free"));
            break;
        case PlotData::PESError:
            ui->leStatus->setText(tr("Error: ") + gUsers->GetName(mPlotData->ESUserConst()));
            break;
        case PlotData::PESEditing:
            ui->leStatus->setText(tr("Editing by ") + gUsers->GetName(mPlotData->ESUserConst()));
            break;
        }

        if (!lHistory) {
            ui->leMainSize->setText(gSettings->FormatNumber(mPlotData->DataLength()));
        } else {
            ui->leMainSize->setText(gSettings->FormatNumber(lHistory->DataLength()));
        }

        ShowComments();
    }
    //ui->horizontalLayout_8->update();
    //updateGeometry();
}

bool PlotProp::SaveData() {
    bool res = false;
    if (mPlotData) {
        bool lVersionsLoaded = false;
        // checking first

        if (mPlotData->VersionIntConst().trimmed() != ui->leVersionInt->text().trimmed()) {
            if (!mPlotData->LoadVersions()) return false;
            lVersionsLoaded = true;
            foreach (PlotData * lPlot, mPlotData->VersionsConst()) {
                if (lPlot->VersionIntConst().trimmed() == ui->leVersionInt->text().trimmed()) {
                    QMessageBox::critical(this, tr("Document properties"), tr("Internal version is not unique!"));
                    return false;
                }
            }
        }

        ProjectData * lProject = gProjects->FindByIdProject(mPlotData->IdProject());
        if (!lProject) return false;

        if (mTreeData->ActualIdGroup() < 2
                && ui->leVersionExt->text().trimmed() != mPlotData->VersionExtConst() /* it is temporary part */
                && lProject->ProjectType()->VerLenFixed()
                && ui->leVersionExt->text().length() != lProject->ProjectType()->VerLen()) {
            QMessageBox::critical(this, tr("Document properties"), tr("Version length must be equal to") + " " + QString::number(lProject->ProjectType()->VerLen()) + "!");
            ui->leVersionExt->setFocus();
            return false;
        }

        foreach (PlotData * lPlot, lProject->PlotListConst()) {
            if (lPlot->IdCommon() == mPlotData->IdCommon()) continue; // skip this

            if (ui->leSheet->isVisible()) {
                // with sheet number
                if ((mPlotData->CodeConst().trimmed() != ui->leCode->text().trimmed()
                     || mPlotData->SheetConst().trimmed() != ui->leSheet->text().trimmed())
                        && lPlot->CodeConst().trimmed() == ui->leCode->text().trimmed()
                        && lPlot->SheetConst().trimmed() == ui->leSheet->text().trimmed()) {
                    QMessageBox::critical(this, tr("Document properties"), tr("Code and sheet number is not unique!"));
                    return false;
                }
            } else {
                // without sheet number
                if (mPlotData->CodeConst().trimmed() != ui->leCode->text().trimmed()
                        && lPlot->CodeConst().trimmed() == ui->leCode->text().trimmed()) {
                    QMessageBox::critical(this, tr("Document properties"), tr("Code is not unique!"));
                    return false;
                }
            }

            if ((mPlotData->NameTopConst().trimmed() != ui->teTopName->toPlainText().trimmed()
                 || mPlotData->NameConst().trimmed() != ui->teBottomName->toPlainText().trimmed())
                    && lPlot->NameTopConst().trimmed() == ui->teTopName->toPlainText().trimmed()
                    && lPlot->NameConst().trimmed() == ui->teBottomName->toPlainText().trimmed()) {
                QMessageBox::critical(this, tr("Document properties"), tr("Name is not unique!"));
                return false;
            }
        }

        if (!lVersionsLoaded
                &&!mPlotData->LoadVersions()) return false;

        if (!db.transaction()) {
            gLogger->ShowSqlError(this, tr("Document properties"), tr("Can't start transaction"), db);
            return false;
        }

        QList <QVariant> lValues;

        // for all versions
        if (mPlotData->IdProject() != ui->leIdProject->text().toInt()) {
            lValues.clear();
            lValues.append(ui->leIdProject->text().toInt());
            mPlotData->SetPropWithVersions(false, false, PlotData::MATIdProject, lValues);

        }

        // for all versions
        if (mPlotData->TDArea() != mTreeData->Area()
                || mPlotData->TDId() != mTreeData->Id()) {
            lValues.clear();
            lValues.append(mTreeData->Area());
            lValues.append(mTreeData->Id());
            mPlotData->SetPropWithVersions(false, false, PlotData::MATTreeType, lValues);
        }

        // versions - only for current document
        mPlotData->VersionIntRef() = ui->leVersionInt->text().trimmed();
        mPlotData->VersionExtRef() = ui->leVersionExt->text().trimmed();

        // for all versions
        if (mPlotData->SectionConst().trimmed() != ui->cbSection->currentText().trimmed()) {
            lValues.clear();
            lValues.append(ui->cbSection->currentText().trimmed());
            mPlotData->SetPropWithVersions(false, false, PlotData::MATComplect, lValues);
        }

        // just for this document
        mPlotData->StageRef() = ui->cbStage->currentText().trimmed();
        if (ui->leCode->isVisible()) {
            mPlotData->CodeRef() = ui->leCode->text().trimmed();
        } else {
            mPlotData->CodeRef() = ui->cbCode->currentText().trimmed();
        }
        if (ui->lblSheet->isVisible()) {
            if (ui->leSheet->isVisible()) {
                mPlotData->SheetRef() = ui->leSheet->text().trimmed();
            } else if (ui->cbSheet->isVisible()) {
                mPlotData->SheetRef() = ui->cbSheet->currentText().trimmed();
            }
        }

//        // for all versions
//        if (mPlotData->CodeConst().trimmed() != ui->leCode->text().trimmed()) {
//            lValues.clear();
//            lValues.append(ui->leCode->text().trimmed());
//            mPlotData->SetPropWithVersions(false, false, PlotData::MATCode, lValues);
//        }
//        // for all versions
//        if (mPlotData->SheetConst().trimmed() != ui->leSheet->text().trimmed()) {
//            lValues.clear();
//            lValues.append(ui->leSheet->text().trimmed());
//            mPlotData->SetPropWithVersions(false, false, PlotData::MATSheet, lValues);
//        }

        // for all versions - not sure here
        if ((mPlotData->FileType() < 20 || mPlotData->FileType() > 29) && mPlotData->ExtensionConst().toLower() == "dwg") {
            mPlotData->BlockNameRef() = ui->leBlockName->text();
            if (mPlotData->BlockNameConst().trimmed() != ui->leBlockName->text().trimmed()) {
                lValues.clear();
                lValues.append(ui->leBlockName->text().trimmed());
                mPlotData->SetPropWithVersions(false, false, PlotData::MATBlockName, lValues);
            }
        }

        // sent - only for current document
        if (mPlotData->SentDateConst().isNull()
                && ui->cbSent->isChecked()) {
            mPlotData->SentDateRef() = ui->dteSentDate->date();
            mPlotData->SentUserRef() = ui->cbSentUser->currentData().toString();
        }

        // for all versions
        if (mPlotData->NameTopConst().trimmed() != ui->teTopName->toPlainText().trimmed()) {
            lValues.clear();
            lValues.append(ui->teTopName->toPlainText().trimmed());
            mPlotData->SetPropWithVersions(false, false, PlotData::MATNameTop, lValues);
        }
        // for all versions
        if (mPlotData->NameConst().trimmed() != ui->teBottomName->toPlainText().trimmed()) {
            lValues.clear();
            lValues.append(ui->teBottomName->toPlainText().trimmed());
            mPlotData->SetPropWithVersions(false, false, PlotData::MATNameBottom, lValues);
        }

        // just for this document
        mPlotData->NotesRef() = ui->teNote->toPlainText();

        // cancelled
        if (ui->cbCancelled->isChecked()
                && !mPlotData->Cancelled()
                || !ui->cbCancelled->isChecked()
                && mPlotData->Cancelled()) {
            lValues.clear();
            lValues.append(ui->cbCancelled->isChecked()?1:0);
            mPlotData->SetPropWithVersions(false, false, PlotData::MATCancelled, lValues);
        }

        // deleted
        // do not delete from here
        /*if (ui->cbDeleted->isChecked()
                && !mPlotData->Deleted()
                || !ui->cbDeleted->isChecked()
                && mPlotData->Deleted()) {
            lValues.clear();
            lValues.append(ui->cbDeleted->isChecked()?1:0);
            mPlotData->SetPropWithVersions(false, false, PlotData::MATDeleted, lValues);
        }*/
        //------------------------------------------------------------------------------

        if (mPlotData->SaveDataWithVersions()) {
            if (db.commit()) {
                res = true;
            } else {
                gLogger->ShowSqlError(this, tr("Document properties"), tr("Can't commit"), db);
                res = false;
            }
        } else {
            db.rollback();
            res = false;
        }

        PlotData * lPlot;
        if (mPlotData->Working()) {
            lPlot = mPlotData;
        } else {
            lPlot = gProjects->FindByIdProject(mPlotData->IdProject())->GetPlotByIdCommon(mPlotData->IdCommon());
        }
        //gProjects->EmitProjectBeforeUpdate(lPlot->IdProject());
        lPlot->RefreshData();
        //gProjects->EmitProjectNeedUpdate(lPlot->IdProject());
    }
    return res;
}

void PlotProp::ShowComments() {
    int i;
    ui->twComments->clear();

    if (!mPlotData) return;

    const QList<PlotCommentData *> & lComments = mPlotData->CommentsConst();

    for (i = 0; i < lComments.length(); i++) {
        QStringList lStrings;

        lStrings.append(lComments.at(i)->DateConst().toString("dd.MM.yy"));
        lStrings.append(gUsers->GetName(lComments.at(i)->UserConst()));
        lStrings.append(lComments.at(i)->CommentConst());

        QTreeWidgetItem * lItem = new QTreeWidgetItem(lStrings);
        lItem->setFlags(lItem->flags() | Qt::ItemIsEditable);
        lItem->setTextAlignment(0, Qt::AlignCenter);
        ui->twComments->addTopLevelItem(lItem);
    }
    for (i = 0; i < ui->twComments->columnCount(); i++)
        ui->twComments->resizeColumnToContents(i);
    for (i = 0; i < ui->twComments->columnCount() - 1; i++)
        ui->twComments->setColumnWidth(i, ui->twComments->columnWidth(i) + 40);
}

void PlotProp::on_leIdProject_editingFinished()
{
    ProjectData * lProjectData = gProjects->FindByIdProject(ui->leIdProject->text().toInt());
    if (lProjectData) {
        ui->leProjName->setText(lProjectData->FullShortName());
        mPlotData->setIdProject(lProjectData->Id());
    } else {
        ui->leIdProject->setText(QString::number(mPlotData->IdProject()));
    }
}

void PlotProp::on_tbProjSel_clicked() {
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    dSel.SetSelectedProject(ui->leIdProject->text().toInt());
    if (dSel.exec() == QDialog::Accepted
            && dSel.GetProjectData()) {
        if (dSel.GetProjectData()->Archived()) {
            QMessageBox::critical(this, tr("Document properties"), tr("Can't move document to archived project"));
        } else {
            ui->leIdProject->setText(QString::number(dSel.GetProjectData()->Id()));
            ui->leProjName->setText(dSel.GetProjectData()->FullShortName());
        }
    }
}

void PlotProp::on_tbTreeSel_clicked() {
    TypeTreeSelect dSel(mTreeData, this);

    if (dSel.exec() == QDialog::Accepted) {
        if (dSel.GetSelected()) {
            if (dSel.GetSelected()->CanExists()) {
                mTreeData = dSel.GetSelected();
                //mPlotData->setTDArea(lTreeData->Area());
                //mPlotData->setTDId(lTreeData->Id());
                ui->leTypeText->setText(mTreeData->FullName());
            } else {
                QMessageBox::critical(this, tr("Document properties"), tr("Document can't exists in that leaf"));
            }
        } else {
            gLogger->ShowError(this, tr("Internal error"), tr("Invalid return from tree data record dialog"));
        }
    }
}

void PlotProp::on_pbOK_clicked() {
    if (SaveData()) accept();
}

void PlotProp::on_twComments_customContextMenuRequested(const QPoint &pos) {
    QMenu popup(this);
    QAction *actAdd, *actRes;

    actAdd = popup.addAction(tr("Add..."));

    if (actRes = popup.exec(QCursor::pos())) {
        if (actRes == actAdd) {
            QInputDialog lInput(this);

            lInput.setWindowFlags(lInput.windowFlags() & ~Qt::WindowContextHelpButtonHint);

            lInput.setWindowTitle(tr("New comment"));
            lInput.setLabelText(tr("Enter new comment.\nAfter adding comment it can't be changed or removed."));
            //lInput.setInputMode(QInputDialog::TextInput);
            if (lInput.exec() == QDialog::Accepted) {
                PlotCommentData *lPlotCommentData = new PlotCommentData(mPlotData->Id(), lInput.textValue().trimmed());
                if (lPlotCommentData->SaveData()) {
                    lPlotCommentData->RefreshData();
                    mPlotData->CommentsRef().append(lPlotCommentData);
                    ShowComments();
                } else {
                    delete lPlotCommentData;
                }
            }
        }
    }
}

void PlotProp::on_cbSent_toggled(bool checked) {
    ui->dteSentDate->setVisible(checked);
    ui->cbSentUser->setVisible(checked);
}

void PlotProp::on_cbCancelled_toggled(bool checked) {
    ui->dteCancDate->setVisible(checked);
    ui->leCancUser->setVisible(checked);
}

void PlotProp::on_cbDeleted_toggled(bool checked) {
    ui->dteDeleteDate->setVisible(checked);
    ui->leDeleteUser->setVisible(checked);
}

void PlotProp::RegenCodePart(PlotData::PlotPropWithCode aPP, const QString &aOldVal, const QString &aNewVal) {
    bool lIsLE = ui->leCode->isVisible();
    QString aNewCode = lIsLE?ui->leCode->text():ui->cbCode->currentText();
    mPlotData->SetPropWithCodeForming(aPP, aOldVal, aNewVal, aNewCode);
    if (lIsLE) {
        ui->leCode->setText(aNewCode);
    } else {
        ui->cbCode->setCurrentText(aNewCode);
    }
/*


    ProjectData * lProject = gProjects->FindByIdProject(mPlotData->IdProject());

    if (lProject) {
        ProjectData * lProjectMain = lProject;
        while (lProjectMain->Parent()
               && lProjectMain->Parent()->Type() == ProjectData::PDProject
               && lProjectMain->CodeTemplateConst().isEmpty()) {
            //if (lConstructNumber.isEmpty()) lConstructNumber = lProjectMain->ShortNumConst();
            lProjectMain = lProjectMain->Parent();
        }

        if (mTreeData &&  mTreeData->ActualIdGroup() > 1
                || lProjectMain->CodeTemplateConst().isEmpty()) {
            // it was simple code generating, need not regenerate
            // !!!!??? need regenerate too (it is simple)
        } else {
            //regen
            bool lIsLE = ui->leCode->isVisible();
            QString aNewCode = lIsLE?ui->leCode->text():ui->cbCode->currentText();
            PlotData::SetPropWithCodeForming(aPP, lProjectMain->ProjectType(), aOldVal, aNewVal, lProjectMain->CodeTemplateConst(), aNewCode);
            if (lIsLE) {
                ui->leCode->setText(aNewCode);
            } else {
                ui->cbCode->setCurrentText(aNewCode);
            }
        }
    }*/
}

void PlotProp::RegenCodeList() {
    ProjectData * lProject = gProjects->FindByIdProject(mPlotData->IdProject());

    if (lProject) {
        QStringList lCodeList, lSheetList;
        QString lCodeNew, lSheet;

        if (ui->lblSheet->isVisible()) {
            if (ui->leSheet->isVisible()) {
                lSheet = ui->leSheet->text();
            } else if (ui->cbSheet->isVisible()) {
                lSheet = ui->cbSheet->currentText();
            }
        }

        if (ui->leCode->isVisible()) {
            ui->leCode->setVisible(false);
            ui->cbCode->setVisible(true);
            if (ui->lblSheet->isVisible()
                    && ui->leSheet->isVisible()) {
                ui->leSheet->setVisible(false);
                ui->cbSheet->setVisible(true);
            }
        }

        PlotData::RegenCodeStatic(lProject, mTreeData, ui->cbSection->isVisible()?ui->cbSection->currentText():ui->leSection->text(), ui->cbStage->currentText(), ui->leVersionExt->text(),
                                  lCodeList, lCodeNew, lSheetList, lSheet, mSheetSetted, mPlotData->IdCommon());

        QString lPrevCode = ui->cbCode->currentText();

        //ui->cbCode->blockSignals(true);
        ui->cbCode->clear();
        ui->cbCode->addItems(lCodeList);
        // index
        if (!lCodeList.isEmpty()
                && lCodeList.at(lCodeList.length() - 1) == lCodeNew)
            ui->cbCode->setCurrentIndex(lCodeList.length() - 1);
        // text
        if (lPrevCode != lCodeNew) {
            ui->cbCode->setCurrentText(lCodeNew);
            //mPrevCode = lCodeNew;
//            if (mCodeSetted) {
//                QMessageBox::critical(this, tr("New document"), tr("Code was modified!"));
//                mCodeSetted = false;
//            }
        }
        //ui->cbCode->blockSignals(false);

        ui->cbSheet->blockSignals(true);
        ui->cbSheet->clear();
        ui->cbSheet->addItems(lSheetList);
        if (mSheetSetted) {
            ui->cbSheet->setCurrentText(lSheet);
        } else {
            if (ui->cbSheet->count())
                ui->cbSheet->setCurrentIndex(ui->cbSheet->count() - 1);
        }
        ui->cbSheet->blockSignals(false);
    }
}

void PlotProp::on_leVersionExt_textEdited(const QString &arg1) {
    RegenCodePart(PlotData::PPWCVersionExt, mVerPrev, arg1);
    mVerPrev = arg1;
}

void PlotProp::on_cbStage_currentTextChanged(const QString &arg1) {
//    QMessageBox::critical(NULL, "", "on_cbStage_currentTextChanged");
    RegenCodePart(PlotData::PPWCStage, mStagePrev, arg1);
    mStagePrev = arg1;
}

void PlotProp::on_cbSection_editTextChanged(const QString &arg1) {
    RegenCodePart(PlotData::PPWCComplect, mComplectPrev, arg1);
    mComplectPrev = arg1;
}

void PlotProp::on_leCode_customContextMenuRequested(const QPoint &pos) {
    QMenu *lPopup = ui->leCode->createStandardContextMenu();
    if (!ui->leCode->isReadOnly()) {
        lPopup->addSeparator();
        lPopup->addAction(ui->actionRegen_code);
    }
    lPopup->exec(QCursor::pos());
}

void PlotProp::on_leVersionExt_editingFinished() {
    ProjectData * lProject = gProjects->FindByIdProject(mPlotData->IdProject());
    if (!lProject) return;

    if (mTreeData->ActualIdGroup() < 2
            && lProject->ProjectType()->VerLenFixed()) {
        QString lVerExt = ui->leVersionExt->text();
        if (lVerExt.contains(QRegExp("^[0-9a-zA-Z]$"))) {
            QString lVerExtOrig = lVerExt;
            while (lVerExt.length() <  lProject->ProjectType()->VerLen()) {
                lVerExt.prepend('0');
            }
            while (lVerExt.length() >  lProject->ProjectType()->VerLen()
                   && lVerExt.at(0) == '0') lVerExt = lVerExt.mid(1);
            if (lVerExtOrig != lVerExt) {
                ui->leVersionExt->setText(lVerExt);
                RegenCodePart(PlotData::PPWCVersionExt, mVerPrev, lVerExt);
                mVerPrev = lVerExt;
            }
        }
    }
}

void PlotProp::on_cbCode_customContextMenuRequested(const QPoint &pos) {
    QMenu *lPopup = ui->cbCode->lineEdit()->createStandardContextMenu();
    lPopup->addSeparator();
    lPopup->addAction(ui->actionRegen_code);
    lPopup->exec(QCursor::pos());
}

/*void PlotProp::on_cbStage_editTextChanged(const QString &arg1) {
    QMessageBox::critical(NULL, "", "on_cbStage_editTextChanged");
}*/
