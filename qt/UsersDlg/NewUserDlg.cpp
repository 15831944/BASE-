#include "NewUserDlg.h"
#include "ui_NewUserDlg.h"

#include "DepartData.h"
#include "UserData.h"

#include "../VProject/common.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/HomeData.h"

NewUserDlg::NewUserDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewUserDlg),
    mUser(NULL)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    if (gSettings->ServerSecure.AskPassForUserMan == -1) {
        gSettings->ServerSecure.AskPassForUserMan = gHomeData->Get("ASK_PASS_FOR_USER_MAN").toInt();
    }
    ui->wdYourPass->setVisible(gSettings->ServerSecure.AskPassForUserMan);

    QPalette lPaletteReq = ui->leFullName->palette();
    lPaletteReq.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);

    ui->leName->setPalette(lPaletteReq);
    ui->leLogin->setPalette(lPaletteReq);
    ui->lePass1->setPalette(lPaletteReq);
    ui->lePass2->setPalette(lPaletteReq);
    ui->leYourPass->setPalette(lPaletteReq);

    ui->leLogin->setValidator(new QRegExpValidator(QRegExp("^[a-zA-Z][a-zA-Z0-9\\_]$"), this));

    //if (!gUsers->HasCompany()) {
        ui->wdCompany->setVisible(false);
    //} else {

    //}

    if (!gUsers->HasDepartment()) {
        ui->wdDepartment->setVisible(false);
    } else {
        ui->cbDepartment->addItem("");
        for (int i = 0; i < gDeparts->DepartListConst().length(); i++) {
            ui->cbDepartment->addItem(gDeparts->DepartListConst().at(i)->NameConst(), gDeparts->DepartListConst().at(i)->Id());
        }
    }

    ui->cbBirthDay->setReadOnly(false);
    ui->deBirthday->setEnabled(false);

    // plotname is absent MOTHERFUCKER


    if (!gUsers->HasHireDate()) {
        ui->widHireDate->setVisible(false);
    } else {
        ui->cbHireDate->setReadOnly(false);
        ui->deHireDate->setEnabled(false);
    }

    if (!gUsers->HasTrialPeriod()) {
        ui->widTrial->setVisible(false);
    }

    if (!gUsers->HasEPH()) {
        ui->widEPH->setVisible(false);
    }

    QSqlQuery query(db);
    // first group - main rights only
    query.prepare("select role_name, description from v_my_role where id_group = 1 order by id");
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(tr("User permissions"), query);
    } else {
        if (!query.exec()) {
            gLogger->ShowSqlError(tr("User permissions"), query);
        } else {
            while (query.next()) {
                ui->cbRights->addItem(qString("description"), qString("role_name"));
                if (qString("role_name").endsWith("_USER0")) {
                    ui->cbRights->setCurrentIndex(ui->cbRights->count() - 1);
                }
            }
        }
    }
}

NewUserDlg::~NewUserDlg()
{
    delete ui;
}

UserData *NewUserDlg::User() const {
    return mUser;
}

void NewUserDlg::Accept() {
//    if (ui->leFullName->text().isEmpty()) {
//        QMessageBox::critical(this, tr("New user"), tr("Full name must be specified"));
//        ui->leFullName->setFocus();
//        return;
//    }

    if (ui->leName->text().isEmpty()) {
        QMessageBox::critical(this, tr("New user"), tr("Name must be specified!"));
        ui->leName->setFocus();
        return;
    }

    UserData *lUser = gUsers->FindByName(ui->leName->text());
    if (lUser) {
        //if (!lUser->Disabled()) {
            if (QMessageBox::question(this, tr("New user"), tr("User with this name already exists\nShow properties?")) == QMessageBox::Yes) {
                mUser = lUser;
                done(200);
                return;
            }
        /*} else {
            if (QMessageBox::question(this, tr("New user"), tr("User with this name already exists\nFired\nRestore Show properties?")) == QMessageBox::Yes) {
                mUser = lUser;
                done(201);
                return;
            }
        }*/
        ui->leName->setFocus();
        return;
    }

    if (ui->leLogin->text().length() < 3) {
        QMessageBox::critical(this, tr("New user"), tr("Login must be at least 4 symbols length!"));
        ui->leLogin->setFocus();
        return;
    }

    if (ui->lePass1->text().isEmpty()) {
        QMessageBox::critical(this, tr("New user"), tr("Password must be specified!"));
        ui->lePass1->setFocus();
        return;
    }

    if (ui->lePass1->text().at(0).toUpper() < 'A'
            || ui->lePass1->text().at(0).toUpper() > 'Z') {
        QMessageBox::critical(this, tr("New user"), tr("Password must start with latin letter!"));
        ui->lePass1->setFocus();
        ui->lePass1->selectAll();
        return;
    }

    if (ui->lePass1->text() != ui->lePass2->text()) {
        QMessageBox::critical(this, tr("New user"), tr("Passwords must be equal!"));
        ui->lePass2->setFocus();
        ui->lePass2->selectAll();
        return;
    }

    if (ui->wdYourPass->isVisible()
            && ui->leYourPass->text() != db.password()) {
        QMessageBox::critical(this, tr("New user"), tr("Your password is wrong!"));
        ui->leYourPass->setFocus();
        ui->leYourPass->selectAll();
        return;
    }

    bool lIsErr = false;
    QSqlQuery qCreate(db);
    bool lExecRes;
    // create
    if (db.driverName() == "QPSQL") {
        lExecRes = qCreate.exec("create role " + ui->leLogin->text() + " nosuperuser login encrypted password '" + ui->lePass1->text() + "'");
    } else {
        lExecRes = qCreate.exec("create user " + ui->leLogin->text() + " identified by " + ui->lePass1->text());
    }
    if (!lExecRes
            || qCreate.lastError().isValid()) {
        if ((qCreate.lastError().text().startsWith("ORA-01920") || qCreate.lastError().text().contains("already exists"))
                && QMessageBox::question(this, tr("New user"), qCreate.lastError().text()
                                         + "\n\n" + tr("Try to reuse existing user?\n(NB: password will not be changed)")) == QMessageBox::Yes) {
            // reuse this user
            QSqlQuery qRevoke(db);
            for (int i = 0; i < ui->cbRights->count(); i++) {
                // ignore errors
                qRevoke.exec("revoke " + ui->cbRights->itemData(i).toString() + " from " + ui->leLogin->text());
            }
            goto createUserReuse;
        } else {
            gLogger->ShowSqlError(tr("New user"), qCreate);
            lIsErr = true;
        }
    } else {
createUserReuse:
        // grant role; no additional roles need n't
        if (db.driverName() == "QPSQL") {
            lExecRes = qCreate.exec("grant " + ui->cbRights->currentData().toString() + " to " + ui->leLogin->text()
                         + ";\nalter role " + ui->leLogin->text() + " set search_path to " + gSettings->CurrentSchema); // PROJ_SCHEMA
        } else {
            lExecRes = qCreate.exec("grant " + ui->cbRights->currentData().toString() + " to " + ui->leLogin->text());
        }
        if (!lExecRes
                || qCreate.lastError().isValid()) {
            gLogger->ShowSqlError(tr("New user"), qCreate);
            lIsErr = true;
        } else {
            // insert into table
            UserData *lUser = new UserData(0, ui->leName->text(), ui->leLogin->text(), 1, "", ui->leFullName->text(),
                                           ui->wdCompany->isVisible()?ui->cbCompany->currentData().toInt():0,
                                           ui->wdDepartment->isVisible()?ui->cbDepartment->currentData().toInt():0, ui->cbPosition->currentText(),
                                           ui->cbHireDate->isChecked()?ui->deHireDate->date():QDate(), ui->leTrial->text().toInt(), 0,
                                           ui->leEMail->text(), ui->lePhone1->text(), ui->lePhone2->text(), ui->lePhone3->text(), ui->leAddress->text(), ui->leRoom->text(),
                                           ui->leComments->text(), ui->cbBirthDay->isChecked()?ui->deBirthday->date():QDate(), ui->leEPH->text().toInt(), 0, 0);
            bool lDummyNotChanged;
            if (lUser->SaveData(lDummyNotChanged)) {
                lUser->CommitEdit();
                gUsers->UsersRef().append(lUser);
                std::sort(gUsers->UsersRef().begin(), gUsers->UsersRef().end(),
                          [] (const UserData * d1, const UserData * d2) { return d1->NameConst() < d2->NameConst(); });
                gHomeData->Set("NEED_UPDATE_RIGHTS", "1");
                accept();
            } else {
                delete lUser;
                lIsErr = true;
            }
        }
    }
}

void NewUserDlg::on_leFullName_textEdited(const QString &arg1) {
    QString lOrig = arg1, lStr;

    lOrig.remove(QRegExp("[א-ת]"));

    if (!lOrig.count(' ')
            || lOrig.indexOf(' ') == lOrig.length() - 1) {
        lStr = lOrig;
    } else {
        lStr = lOrig.left(lOrig.indexOf(' ') + 2) + '.';
        if (lOrig.count(' ') == 2
                && lOrig.lastIndexOf(' ') < lOrig.length() - 1) {
            lStr += lOrig.mid(lOrig.lastIndexOf(' ') + 1, 1) + ".";
        }
    }

    if (lStr != ui->leName->text())
        ui->leName->setText(lStr);
}

void NewUserDlg::on_leName_textChanged(const QString &arg1) {
    QString lLogin;

    QStringList lTrans;

    lTrans << "A" << "B" << "V" << "G" << "D" << "E" << "ZH" << "Z" << "I" << "J" << "K" << "L" << "M" << "N"
           << "O" << "P" << "R" << "S" << "T" << "U" << "F" << "KH" << "TS" << "CH" << "SH" << "SHCH" << "" << "Y" << "" << "E" << "YU" << "YA";

    for (int i = 0; i < arg1.length(); i++) {
        if (arg1.at(i).toUpper() >= 'A' && arg1.at(i).toUpper() <= 'Z'
                || arg1.at(i) >= '0' && arg1.at(i) <= '9') {

            if (db.driverName() == "QPSQL") {
                lLogin += arg1.at(i).toLower();
            } else {
                lLogin += arg1.at(i).toUpper();
            }
        } else if (arg1.at(i) == ' ' && i != arg1.length() - 1) {
            lLogin += '_';
        } else if (arg1.at(i).toUpper() >= 0x0410
                   && arg1.at(i).toUpper() <= 0x042F) {
            if (db.driverName() == "QPSQL") {
                lLogin += lTrans.at(arg1.at(i).toUpper().unicode() - 0x0410).toLower();
            } else {
                lLogin += lTrans.at(arg1.at(i).toUpper().unicode() - 0x0410);
            }
        } else if (arg1.at(i).toUpper() == 0x0401) {
            if (db.driverName() == "QPSQL") {
                lLogin += "yo";
            } else {
                lLogin += "YO";
            }
        }
    }

    if (ui->leLogin->text() != lLogin) {
        UserData *lData = gUsers->FindByLogin(lLogin);

        QString lLoginNew = lLogin;
        int i = 1;

        while (lData) {
            lLoginNew = lLogin + QString::number(i);
            lData = gUsers->FindByLogin(lLoginNew);
            i++;
        }
        ui->leLogin->setText(lLoginNew);
    }
}

void NewUserDlg::on_leLogin_textEdited(const QString &arg1) {
    ui->leLogin->setText(arg1.toUpper());
}

void NewUserDlg::on_cbBirthDay_toggled(bool checked) {
    ui->deBirthday->setEnabled(checked);
}

void NewUserDlg::on_cbHireDate_toggled(bool checked) {
    ui->deHireDate->setEnabled(checked);
}
