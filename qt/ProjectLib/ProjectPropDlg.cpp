#include "ProjectPropDlg.h"
#include "ui_ProjectPropDlg.h"

#include "ProjectData.h"
#include "ProjectTypeData.h"

#include "../VProject/GlobalSettings.h"

//#include "../VProject/UpdateThread.h"
//#include "../VProject/MainWindow.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserRight.h"
#include "../UsersDlg/CustomerData.h"

ProjectPropDlg::ProjectPropDlg(eProps, ProjectData *aProject, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ProjectPropDlg),
    mProject(aProject), mParentGroup(NULL)
{
    InitInConstructor();
}

ProjectPropDlg::ProjectPropDlg(eNew, ProjectData *aParentGroup, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ProjectPropDlg),
    mProject(NULL), mParentGroup(aParentGroup)
{
    InitInConstructor();
}

ProjectPropDlg::~ProjectPropDlg()
{
    delete ui;
}

int ProjectPropDlg::ProjectId() const {
    if (mProject)
        return mProject->Id();
    else
        return 0;
}

void ProjectPropDlg::InitInConstructor() {
    int i;
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    mPaletteNorm = ui->leId->palette();
    // required
    mPaletteReq = mPaletteNorm;
    mPaletteReq.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);

    // read only
    mPaletteDis = mPaletteNorm;
    mPaletteDis.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);
    ui->leId->setPalette(mPaletteDis);
    ui->leStage->setPalette(mPaletteDis);

    ui->leGIP->setPalette(mPaletteDis);
    ui->leCustomer->setPalette(mPaletteDis);

    ui->leContractDate->setPalette(mPaletteDis);

    ui->leCrDate->setPalette(mPaletteDis);
    ui->leCrUser->setPalette(mPaletteDis);
    ui->leEndDate->setPalette(mPaletteDis);
    ui->leEndUser->setPalette(mPaletteDis);

    ui->cbGIP->addItem("");
    for (i = 0; i < gUsers->UsersConst().length(); i++)
        if (!gUsers->UsersConst().at(i)->Disabled()
                && gUsers->UsersConst().at(i)->IsGIP()
                || mProject && gUsers->UsersConst().at(i)->LoginConst() == mProject->GipConst())
            ui->cbGIP->addItem(gUsers->UsersConst().at(i)->NameConst(), gUsers->UsersConst().at(i)->LoginConst());

    ui->cbCustomer->addItem("");
    for (i = 0; i < gCustomers->CustomerListConst().length(); i++)
        if (gCustomers->CustomerListConst().at(i)->DelUserConst().isEmpty()
                || mProject && gCustomers->CustomerListConst().at(i)->Id() == mProject->IdCustomer())
            ui->cbCustomer->addItem(gCustomers->CustomerListConst().at(i)->ShortNameConst(), gCustomers->CustomerListConst().at(i)->Id());
}

void ProjectPropDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    ShowData();
}

void ProjectPropDlg::ShowData() {
    if (mProject) {
        ui->wdId->setVisible(true);
        ui->wdContract->setVisible(true);
        ui->wdCreated->setVisible(true);

        ui->leId->setText(QString::number(mProject->Id()));

        ui->leShortName->setText(mProject->ShortNameConst());
        if (gUserRight->CanUpdate("v_project", "shortname") && !mProject->Archived()) {
            ui->leShortName->setReadOnly(false);
            ui->leShortName->setPalette(mPaletteReq);
        } else {
            ui->leShortName->setReadOnly(true);
            ui->leShortName->setPalette(mPaletteDis);
        }

        if (gUserRight->CanUpdate("v_project", "stage") && !mProject->Archived()) {
            ui->cbStage->clear();
            const QStringList &lStages = mProject->ProjectType()->StagesConst();
            if (!lStages.isEmpty()) {
                ui->cbStage->addItem("");
            }
            ui->cbStage->addItems(lStages);
            ui->cbStage->setEditable(lStages.isEmpty());
            ui->cbStage->setCurrentText(mProject->StageConst());
            ui->cbStage->setVisible(true);
            ui->leStage->setVisible(false);
        } else {
            ui->leStage->setText(mProject->StageConst());
            ui->leStage->setVisible(true);
            ui->cbStage->setVisible(false);
        }

        const QString & lGipName = gUsers->GetName(mProject->GipConst());
        if (gUserRight->CanUpdate("v_project", "gip") && !mProject->Archived()) {
            ui->cbGIP->setCurrentText(lGipName);
            ui->cbGIP->setVisible(true);
            ui->tbSelManager->setVisible(true);
            ui->leGIP->setVisible(false);
        } else {
            ui->leGIP->setText(lGipName);
            ui->cbGIP->setVisible(false);
            ui->tbSelManager->setVisible(false);
            ui->leGIP->setVisible(true);
        }

        CustomerData *lCustomer;
        if (mProject->IdCustomer()
                && (lCustomer = gCustomers->GetCustomerById(mProject->IdCustomer()))) {
            ui->cbCustomer->setCurrentText(lCustomer->ShortNameConst());
            ui->leCustomer->setText(lCustomer->ShortNameConst());
        } else {
            ui->cbCustomer->setCurrentText("");
            ui->leCustomer->setText("");
        }
        if (gUserRight->CanUpdate("v_project", "id_customer") && !mProject->Archived()) {
            ui->cbCustomer->setVisible(true);
            ui->tbSelCustomer->setVisible(true);
            ui->leCustomer->setVisible(false);
        } else {
            ui->cbCustomer->setVisible(false);
            ui->tbSelCustomer->setVisible(false);
            ui->leCustomer->setVisible(true);
        }

        // contract
        if (gSettings->ContractMode == 1) {
            ui->leContract->setText(mProject->ContractConst());
            if (gUserRight->CanUpdate("v_project", "dogovor") && !mProject->Archived()) {
                ui->leContract->setReadOnly(false);
                ui->leContract->setPalette(mPaletteNorm);
            } else {
                ui->leContract->setReadOnly(true);
                ui->leContract->setPalette(mPaletteDis);
            }

            if (!mProject->ContractDateConst().isNull()) {
                ui->cbContractDate->setChecked(true);
                ui->deContractDate->setDate(mProject->ContractDateConst());
                ui->deContractDate->setEnabled(true);
                ui->leContractDate->setText(mProject->ContractDateConst().toString("dd.MM.yy"));
            } else {
                // no contract date
                ui->cbContractDate->setChecked(false);
                ui->deContractDate->setDate(QDate::currentDate());
                ui->deContractDate->setEnabled(false);
                ui->leContractDate->setText("");
            }
            if (gUserRight->CanUpdate("v_project", "dogdate") && !mProject->Archived()) {
                ui->leContractDate->setVisible(false);
                ui->cbContractDate->setVisible(true);
                ui->deContractDate->setVisible(true);
            } else {
                ui->leContractDate->setVisible(true);
                ui->cbContractDate->setVisible(false);
                ui->deContractDate->setVisible(false);
            }
        } else {

        }

        ui->ptFullName->setPlainText(mProject->NameConst());
        if (gUserRight->CanUpdate("v_project", "name") && !mProject->Archived()) {
            ui->ptFullName->setReadOnly(false);
            ui->ptFullName->setPalette(mPaletteReq);
        } else {
            ui->ptFullName->setReadOnly(true);
            ui->ptFullName->setPalette(mPaletteDis);
        }

        ui->ptComments->setPlainText(mProject->CommentsConst());
        if (gUserRight->CanUpdate("v_project", "comments") && !mProject->Archived()) {
            ui->ptComments->setReadOnly(false);
            ui->ptComments->setPalette(mPaletteNorm);
        } else {
            ui->ptComments->setReadOnly(true);
            ui->ptComments->setPalette(mPaletteDis);
        }

        ui->leCrDate->setText(mProject->StartDateConst().toString("dd.MM.yy"));
        ui->leCrUser->setText(gUsers->GetName(mProject->CrUserConst()));

        if (!mProject->EndDateConst().isNull()) {
            ui->leEndDate->setText(mProject->EndDateConst().toString("dd.MM.yy"));
            ui->leEndUser->setText(gUsers->GetName(mProject->EndUserConst()));
            ui->wdArchive->setVisible(true);
        } else {
            ui->wdArchive->setVisible(false);
        }
    } else {
        if (gUserRight->CanInsertAnyColumn("v_project")) {
            ui->wdId->setVisible(false);
            ui->wdCreated->setVisible(false);
            ui->wdArchive->setVisible(false);

            ui->leShortName->setReadOnly(false);
            ui->leShortName->setPalette(mPaletteReq);

            if (gUserRight->CanInsert("v_project", "stage")) {
                ui->cbStage->clear();
                ui->cbStage->setEditable(true);
                ui->cbStage->setVisible(true);
                ui->leStage->setVisible(false);
            } else {
                ui->leStage->setVisible(true);
                ui->cbStage->setVisible(false);
            }

            if (gUserRight->CanInsert("v_project", "gip")) {
                ui->cbGIP->setVisible(true);
                ui->leGIP->setVisible(false);
            } else {
                ui->cbGIP->setVisible(false);
                ui->leGIP->setVisible(true);
            }

            if (gUserRight->CanInsert("v_project", "id_customer")) {
                ui->cbCustomer->setVisible(true);
                ui->tbSelCustomer->setVisible(true);
                ui->leCustomer->setVisible(false);
            } else {
                ui->cbCustomer->setVisible(false);
                ui->tbSelCustomer->setVisible(false);
                ui->leCustomer->setVisible(true);
            }

            // contract
            if (gSettings->ContractMode == 1) {
                // doesn't finish, too lazy
                if (gUserRight->CanInsert("v_project", "dogovor")) {
                    ui->leContract->setReadOnly(false);
                    ui->leContract->setPalette(mPaletteNorm);
                } else {
                    ui->leContract->setReadOnly(true);
                    ui->leContract->setPalette(mPaletteDis);
                }

                ui->cbContractDate->setChecked(false);
                if (gUserRight->CanInsert("v_project", "dogdate")) {
                    ui->deContractDate->setDate(QDate::currentDate());
                    ui->deContractDate->setEnabled(false);

                    ui->leContractDate->setVisible(false);
                    ui->cbContractDate->setVisible(true);
                    ui->deContractDate->setVisible(true);
                } else {
                    ui->leContractDate->setVisible(true);
                    ui->cbContractDate->setVisible(false);
                    ui->deContractDate->setVisible(false);
                }
            } else {
                ui->wdContract->setVisible(false);
            }
            //--

            ui->ptFullName->setReadOnly(false);
            ui->ptFullName->setPalette(mPaletteReq);

            if (gUserRight->CanInsert("v_project", "comments")) {
                ui->ptComments->setReadOnly(false);
                ui->ptComments->setPalette(mPaletteNorm);
            } else {
                ui->ptComments->setReadOnly(true);
                ui->ptComments->setPalette(mPaletteDis);
            }

        } else {
            QTimer::singleShot(0, this, SLOT(close()));
        }
    }
}

void ProjectPropDlg::Accept() {
    if (ui->leShortName->text().trimmed().isEmpty()) {
        QMessageBox::critical(this, tr("Project properties"), tr("Short name must be specified!"));
        ui->leShortName->setFocus();
        return;
    }

    if (ui->ptFullName->toPlainText().trimmed().isEmpty()) {
        QMessageBox::critical(this, tr("Project properties"), tr("Full name must be specified!"));
        ui->ptFullName->setFocus();
        return;
    }

    bool lIsNew = !mProject;
    if (lIsNew) {
        int lNewId;
        if (gOracle->GetSeqNextVal("project_id_seq", lNewId)) {
            mProject = new ProjectData(lNewId, ProjectData::PDProject);
            if (mParentGroup) mProject->setIdGroup(mParentGroup->Id());
        } else {
            return;
        }
    }

    if (!ui->leShortName->isReadOnly())
        mProject->ShortNameRef() = ui->leShortName->text().trimmed();

    if (ui->cbStage->isVisible())
        mProject->StageRef() = ui->cbStage->currentText().trimmed();

    if (ui->cbGIP->isVisible()) {
        if (ui->cbGIP->currentData().isNull()) {
            mProject->GipRef().clear();
        } else {
            mProject->GipRef() = ui->cbGIP->currentData().toString();
        }
    }

    if (ui->cbCustomer->isVisible()) {
        if (ui->cbCustomer->currentText().isEmpty()) {
            mProject->setIdCustomer(0);
        } else {
            CustomerData *lCustomerData = gCustomers->GetCustomerByShortName(ui->cbCustomer->currentText());
            if (!lCustomerData) {
                QMessageBox::critical(this, tr("Project properties"), tr("Can't find customer") + " " + ui->cbCustomer->currentText() + "!");
                ui->cbCustomer->setFocus();
                return;
            }
            mProject->setIdCustomer(lCustomerData->Id());
        }
    }

    mProject->ContractRef() = ui->leContract->text().trimmed();
    if (!ui->cbContractDate->isChecked()) {
        mProject->ContractDateRef() = QDate();
    } else {
        mProject->ContractDateRef() = ui->deContractDate->date();
    }

    mProject->NameRef() = ui->ptFullName->toPlainText().trimmed();
    mProject->CommentsRef() = ui->ptComments->toPlainText().trimmed();

    if (mProject->SaveData()) {
        mProject->CommitEdit();
        if (lIsNew) {
            mProject->RefreshData();
            mProject->setParent(mParentGroup);
        }

        QTimer::singleShot(0, gProjects, SLOT(ProjectListNeedUpdate()));
        accept();
    } else {
        if (!lIsNew) {
            mProject->RollbackEdit();
        } else {
            delete mProject;
            mProject = NULL;
        }
    }
}

void ProjectPropDlg::on_cbContractDate_clicked(bool checked) {
    ui->deContractDate->setEnabled(checked);
}

static int CheckUserForInclude(const UserData *aUser, void *apData) {
    if (static_cast<ProjectData *>(apData)
            && static_cast<ProjectData *>(apData)->GipConst() == aUser->LoginConst()) return 2; // include in list
    return 1; // standard check
}

void ProjectPropDlg::on_tbSelManager_clicked() {
    UserData *lUser = NULL;
    if (!ui->cbGIP->currentData().isNull()) lUser = gUsers->FindByLogin(ui->cbGIP->currentData().toString());
    lUser = gUsers->SelectUser(lUser, UsersDlgShowGip, CheckUserForInclude, mProject);
    if (lUser) {
        ui->cbGIP->setCurrentText(lUser->NameConst());
    }
}

static int CheckCustomerForInclude(const CustomerData *aCustomer, void *apData) {
    if (static_cast<ProjectData *>(apData)
            && static_cast<ProjectData *>(apData)->IdCustomer() == aCustomer->Id()) return 2; // include in list
    return 1; // standard check
}

void ProjectPropDlg::on_tbSelCustomer_clicked() {
    int lNewId;
    if (lNewId = gCustomers->SelectCustomer(ui->cbCustomer->currentData().toInt(), CheckCustomerForInclude, mProject))
        ui->cbCustomer->setCurrentText(gCustomers->GetCustomerById(lNewId)->ShortNameConst());
}
