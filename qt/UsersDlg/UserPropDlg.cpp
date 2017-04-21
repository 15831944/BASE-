#include "UserPropDlg.h"
#include "ui_UserPropDlg.h"

#include "DepartData.h"
#include "UserRight.h"
#include "UserRightsDlg.h"

#include "../VProject/common.h"
#include "../VProject/GlobalSettings.h"

#include <QTimer>
#include <QMessageBox>

UserPropDlg::UserPropDlg(UserData *aUser, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::UserPropDlg),
    mUser(aUser), mWillRestore(false),
    mJustStarted(true)
{
    ui->setupUi(this);
    InitInConstructor();
}

UserPropDlg::~UserPropDlg()
{
    delete ui;
}

void UserPropDlg::InitInConstructor() {
    int i;
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    // required and disabled
    QPalette lPaletteReq = ui->leName->palette();
    QPalette lPaletteDis = lPaletteReq;
    lPaletteReq.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    lPaletteDis.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);

    if (gUserRight->CanUpdate(gUsers->TableName(), "name")) {
        ui->leName->setPalette(lPaletteReq);
    } else {
        ui->leName->setPalette(lPaletteDis);
        ui->leName->setReadOnly(true);
    }

    if (gUserRight->CanUpdate(gUsers->TableName(), "login")) {
        ui->leLogin->setVisible(false);
        ui->cbLogin->setPalette(lPaletteReq);

        QSqlQuery query(
                    (db.driverName() == "QPSQL")
                    ?"select rolname as username from pg_roles where rolcanlogin = true order by rolname"
                    :"select username from all_users order by username",
                    db);

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("User data"), query);
        } else {
            while (query.next()) {
                ui->cbLogin->addItem(qString("username"));
            }
        }
    } else {
        ui->leLogin->setPalette(lPaletteDis);
        ui->cbLogin->setVisible(false);
    }

    if (!gUsers->HasPlotname()) {
        ui->wdPlotname->setVisible(false);
    } else {
        if (!gUserRight->CanUpdate(gUsers->TableName(), "plotname")) {
            ui->lePlotName->setPalette(lPaletteDis);
            ui->lePlotName->setReadOnly(true);
        }
    }

    if (!gUserRight->CanUpdate(gUsers->TableName(), "long_name")) {
        ui->leFullName->setPalette(lPaletteDis);
        ui->leFullName->setReadOnly(true);
    }

    if (gUserRight->CanUpdate(gUsers->TableName(), "birthday")) {
        ui->cbBirthDay->setReadOnly(false);
    } else {
        ui->deBirthday->setReadOnly(true);
        ui->deBirthday->setPalette(lPaletteDis);
    }

    //if (!gUsers->HasCompany()) {
        ui->wdCompany->setVisible(false);
    //} else {

    //}


    if (!gUsers->HasDepartment()) {
        ui->wdDepartment->setVisible(false);
    } else {
        if (gUserRight->CanUpdate(gUsers->TableName(), "id_department")) {
            ui->leDepartment->setVisible(false);
            ui->cbDepartment->addItem("");
            for (int i = 0; i < gDeparts->DepartListConst().length(); i++) {
                ui->cbDepartment->addItem(gDeparts->DepartListConst().at(i)->NameConst(), gDeparts->DepartListConst().at(i)->Id());
            }
        } else {
            ui->cbDepartment->setVisible(false);
            ui->leDepartment->setPalette(lPaletteDis);
        }
    }

    if (gUserRight->CanUpdate(gUsers->TableName(), "jobtitle")) {
        ui->lePosition->setVisible(false);

        const QList <UserData *> &lAll = gUsers->UsersConst();
        QStringList lJobs;
        for (i = 0; i < lAll.length(); i++) {
            if (!lAll.at(i)->JobTitleConst().isEmpty()
                    && !lJobs.contains(lAll.at(i)->JobTitleConst()))
                lJobs.append(lAll.at(i)->JobTitleConst());
        }
        lJobs.sort(Qt::CaseInsensitive);

        ui->cbPosition->addItems(lJobs);
    } else {
        ui->cbPosition->setVisible(false);
        ui->lePosition->setPalette(lPaletteDis);
    }

    if (!gUserRight->CanUpdate(gUsers->TableName(), "email")) {
        ui->leEMail->setPalette(lPaletteDis);
        ui->leEMail->setReadOnly(true);
    }

    if (!gUserRight->CanUpdate(gUsers->TableName(), "phone1")) {
        ui->lePhone1->setPalette(lPaletteDis);
        ui->lePhone1->setReadOnly(true);
    }
    if (!gUserRight->CanUpdate(gUsers->TableName(), "phone2")) {
        ui->lePhone2->setPalette(lPaletteDis);
        ui->lePhone2->setReadOnly(true);
    }
    if (!gUserRight->CanUpdate(gUsers->TableName(), "phone3")) {
        ui->lePhone3->setPalette(lPaletteDis);
        ui->lePhone3->setReadOnly(true);
    }

    if (!gUserRight->CanUpdate(gUsers->TableName(), "addr")) {
        ui->leAddress->setPalette(lPaletteDis);
        ui->leAddress->setReadOnly(true);
    }
    if (!gUserRight->CanUpdate(gUsers->TableName(), "room")) {
        ui->leRoom->setPalette(lPaletteDis);
        ui->leRoom->setReadOnly(true);
    }

    if (!gUsers->HasHireDate()) {
        ui->widHireDate->setVisible(false);
    } else {
        if (gUserRight->CanUpdate(gUsers->TableName(), "hire_date")) {
            ui->cbHireDate->setReadOnly(false);
        } else {
            ui->deHireDate->setReadOnly(true);
            ui->deHireDate->setPalette(lPaletteDis);
        }
    }

    if (!gUsers->HasTrialPeriod()) {
        ui->widTrial->setVisible(false);
    } else {
        if (!gUserRight->CanUpdate(gUsers->TableName(), "trial_period")) {
            ui->leTrial->setPalette(lPaletteDis);
            ui->leTrial->setReadOnly(true);
        }
    }

    if (!gUsers->HasEPH()) {
        ui->widEPH->setVisible(false);
    } else {
        if (!gUserRight->CanUpdate(gUsers->TableName(), "eph")) {
            ui->leEPH->setPalette(lPaletteDis);
            ui->leEPH->setReadOnly(true);
        }
    }

    if (!gUserRight->CanUpdate(gUsers->TableName(), "remarks")) {
        ui->leComments->setPalette(lPaletteDis);
        ui->leComments->setReadOnly(true);
    }
}

void UserPropDlg::ShowData() {
    if (mJustStarted) {
        mJustStarted = false;
    }

    if (mUser->Disabled()) {
        ui->pbFired->setVisible(true);

        if (gUserRight->CanUpdate(gUsers->TableName(), "disabled")
                && (!mUser->HasLogin() && gUserRight->CanCreateUser() || mUser->HasLogin() && gUserRight->CanManageUser())) {
            ui->pbFired->setText("Fired. Press here for restore");
        } else {
            ui->pbFired->setText("Fired");
        }
    } else {
        ui->pbFired->setVisible(false);
    }


    ui->leName->setText(mUser->NameConst());
    if (ui->cbLogin->isVisible())
        ui->cbLogin->setCurrentText(mUser->LoginConst());
    else
        ui->leLogin->setText(mUser->LoginConst());
    ui->lePlotName->setText(mUser->PlotNameConst());
    ui->leFullName->setText(mUser->LongNameConst());
    if (mUser->BirthDateConst().isNull()) {
        ui->cbBirthDay->setChecked(false);
        ui->deBirthday->setEnabled(false);
    } else {
        ui->cbBirthDay->setChecked(true);
        ui->deBirthday->setEnabled(true);
        ui->deBirthday->setDate(mUser->BirthDateConst());
    }

    if (ui->cbDepartment->isVisible()) {
        if (mUser->IdDepartment()) {
            for (int i = 0; i < ui->cbDepartment->count(); i++) {
                if (mUser->IdDepartment() == ui->cbDepartment->itemData(i).toInt()) {
                    ui->cbDepartment->setCurrentIndex(i);
                    break;
                }
            }
        } else {
            ui->cbDepartment->setCurrentIndex(0);
        }
    } else {
        if (mUser->IdDepartment()) {
            ui->leDepartment->setText(gDeparts->FindById(mUser->IdDepartment())->NameConst());
        }
    }

    //
    ui->cbPosition->setCurrentText(mUser->JobTitleConst());
    ui->leEMail->setText(mUser->EMailConst());

    ui->lePhone1->setText(mUser->Phone1Const());
    ui->lePhone2->setText(mUser->Phone2Const());
    ui->lePhone3->setText(mUser->Phone3Const());

    ui->leAddress->setText(mUser->AddrConst());
    ui->leRoom->setText(mUser->RoomConst());

    if (mUser->HireDateConst().isNull()) {
        ui->cbHireDate->setChecked(false);
        ui->deHireDate->setEnabled(false);
    } else {
        ui->cbHireDate->setChecked(true);
        ui->deHireDate->setEnabled(true);
        ui->deHireDate->setDate(mUser->HireDateConst());
    }

    if (mUser->TrialPeriod()) {
        ui->leTrial->setText(QString::number(mUser->TrialPeriod()));
    } else {
        ui->leTrial->setText("");
    }

    if (mUser->EPH()) {
        ui->leEPH->setText(QString::number(mUser->EPH()));
    } else {
        ui->leEPH->setText("");
    }

    ui->leComments->setText(mUser->CommentsConst());
}

void UserPropDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        if (mUser) {
            QTimer::singleShot(0, this, SLOT(ShowData()));
        } else {
            QTimer::singleShot(0, this, SLOT(close()));
        }
    }
}


void UserPropDlg::on_cbBirthDay_toggled(bool checked) {
    ui->deBirthday->setEnabled(checked);
}

void UserPropDlg::on_cbHireDate_toggled(bool checked) {
    ui->deHireDate->setEnabled(checked);
}

void UserPropDlg::Accept() {
    // need checks

    // modify data
    mUser->setName(ui->leName->text());
    if (ui->cbLogin->isVisible()) {
        mUser->setLogin(ui->cbLogin->currentText());
    }
    mUser->setPlotName(ui->lePlotName->text());
    mUser->setLongName(ui->leFullName->text());

    if (ui->wdCompany->isVisible()) {
        mUser->setIdCustomer(ui->cbCompany->currentData().toInt());
    }
    if (ui->wdDepartment->isVisible()) {
        mUser->setIdDepartment(ui->cbDepartment->currentData().toInt());
    }
    mUser->setJobTitle(ui->cbPosition->currentText());

    if (ui->widHireDate->isVisible()
            && !ui->deHireDate->isReadOnly()) {
        if (ui->cbHireDate->isChecked()) {
            mUser->HireDateRef() = ui->deHireDate->date();
        } else {
            mUser->HireDateRef() = QDate();
        }
    }

    if (ui->widTrial->isVisible()
            && !ui->leTrial->isReadOnly()) {
        mUser->setTrialPeriod(ui->leTrial->text().toInt());
    }
    //DeclarePar(int, Disabled)

    mUser->setEMail(ui->leEMail->text());
    mUser->setPhone1(ui->lePhone1->text());
    mUser->setPhone2(ui->lePhone2->text());
    mUser->setPhone3(ui->lePhone3->text());

    mUser->setAddr(ui->leAddress->text());
    mUser->setRoom(ui->leRoom->text());
    mUser->setComments(ui->leComments->text());

    if (!ui->deBirthday->isReadOnly()) {
        if (ui->cbBirthDay->isChecked()) {
            mUser->BirthDateRef() = ui->deBirthday->date();
        } else {
            mUser->BirthDateRef() = QDate();
        }
    }

    if (ui->widEPH->isVisible()
            && !ui->leEPH->isReadOnly()) {
        mUser->setEPH(ui->leEPH->text().toInt());
    }

    if (mWillRestore) {
        mUser->setDisabled(0);
    }

    bool lNotChanged;
    if (mUser->SaveData(lNotChanged)) {
        if (lNotChanged) {
            reject();
        } else {
            mUser->CommitEdit();
            if (mWillRestore) {
                bool lCreated = false;
                if (!mUser->HasLogin()) {
                    if (!gUsers->CreateUser(mUser->LoginConst(), "a1")) {
                        return;
                    }
                    lCreated = true;
                }
                if (!gUsers->UnlockUser(mUser->LoginConst())) {
                    return;
                }
                QMessageBox::information(this, tr("User properties"), tr("User") + " '" + mUser->NameConst()
                                         + "' " + tr("restored successfully") + (lCreated?(" " + tr("with password") + " a1"):""));
                UserRightsDlg w1(mUser, this);
                w1.exec();
                //ui->pbUpdateRights->setVisible(true);
            }
            accept();
        }
    } else {
        mUser->RollbackEdit();
    }
}

void UserPropDlg::on_pbFired_clicked() {
    if (!mWillRestore
            && gUserRight->CanUpdate(gUsers->TableName(), "disabled")
            && (!mUser->HasLogin() && gUserRight->CanCreateUser() || mUser->HasLogin() && gUserRight->CanManageUser())
            && QMessageBox::question(this, tr("User properties"), tr("Restore this user?")) == QMessageBox::Yes) {
        mWillRestore = true;
        ui->pbFired->setText(tr("Fired. Will be restored after pressing OK"));
    }
}
