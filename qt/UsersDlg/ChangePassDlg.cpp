#include "ChangePassDlg.h"
#include "ui_ChangePassDlg.h"

#include "UserData.h"
#include "UserRight.h"

#include "../VProject/common.h"

ChangePassDlg::ChangePassDlg(UserData *aUser, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ChangePassDlg),
    mUser(aUser)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    if (gUserRight->CanManageUser()) {
        for (int i = 0; i < gUsers->UsersConst().length(); i++)
            if (!gUsers->UsersConst().at(i)->Disabled())
                ui->cbUser->addItem(gUsers->UsersConst().at(i)->NameConst(), gUsers->UsersConst().at(i)->LoginConst());
        ui->leUser->setVisible(false);
        ui->cbUser->setCurrentText(mUser->NameConst());
    } else {
        ui->cbUser->setVisible(false);
        ui->leUser->setVisible(false);
        ui->lblUser->setVisible(false);

        //ui->leUser->setText(mUser->NameConst());
    }
}

ChangePassDlg::~ChangePassDlg()
{
    delete ui;
}

void ChangePassDlg::Accept() {
    if (ui->lePass->text() != db.password()) {
        QMessageBox::critical(this, tr("Changing password"), tr("Your password is wrong!"));
        ui->lePass->setFocus();
        ui->lePass->selectAll();
        return;
    }

    if (ui->leNewPass1->text().isEmpty()) {
        QMessageBox::critical(this, tr("Changing password"), tr("New password must be specified!"));
        ui->leNewPass1->setFocus();
        return;
    }

    if (ui->leNewPass1->text().at(0).toUpper() < 'A'
            || ui->leNewPass1->text().at(0).toUpper() > 'Z') {
        QMessageBox::critical(this, tr("Changing password"), tr("Password must start with latin letter!"));
        ui->leNewPass1->setFocus();
        ui->leNewPass1->selectAll();
        return;
    }

    if (!ui->cbUser->isVisible()
            || ui->cbUser->currentData().toString() == db.userName()) {
        if (ui->leNewPass1->text().length() < 7) {
            QMessageBox::critical(this, tr("Changing password"), tr("Minimum password length is 7 characters!"));
            ui->leNewPass1->setFocus();
            ui->leNewPass1->selectAll();
            return;
        }
    }

    if (ui->leNewPass1->text() != ui->leNewPass2->text()) {
        QMessageBox::critical(this, tr("Changing password"), tr("Passwords must be equal!"));
        ui->leNewPass2->setFocus();
        ui->leNewPass2->selectAll();
        return;
    }

    if (ui->lePass->text() == ui->leNewPass1->text()) {
        QMessageBox::critical(this, tr("Changing password"), tr("New password can't be equal to old password!"));
        ui->leNewPass1->setFocus();
        ui->leNewPass1->selectAll();
        return;
    }

    QSqlQuery qPassword(db);
    bool lExecRes;
    if (ui->cbUser->isVisible()
            && ui->cbUser->currentData().toString() != db.userName()) {
        if (db.driverName() == "QPSQL") {
            lExecRes = qPassword.exec("alter role " + ui->cbUser->currentData().toString() + " encrypted password '" + ui->leNewPass1->text() + "'");
        } else {
            lExecRes = qPassword.exec("alter user " + ui->cbUser->currentData().toString() + " identified by \"" + ui->leNewPass1->text() + "\" account unlock");
        }
    } else {
        // not enougn rights for "account unlock"
        // and need not unlock current user
        if (db.driverName() == "QPSQL") {
            lExecRes = qPassword.exec("alter role session_user encrypted password '" + ui->leNewPass1->text() + "'");
        } else {
            lExecRes = qPassword.exec("alter user " + mUser->LoginConst() + " identified by \"" + ui->leNewPass1->text() + "\"");
        }
    }

    if (!lExecRes
            || qPassword.lastError().isValid()) {
        gLogger->ShowSqlError(tr("Changing password"), qPassword);
    } else {
        if (ui->cbUser->isVisible()) {
            gLogger->LogErrorToDb("Password changed for user " + ui->cbUser->currentData().toString());
            //QMessageBox::information(this, tr("Changing password"), "Password changed for user " + ui->cbUser->currentData().toString());
        } else {
            gLogger->LogErrorToDb("Password changed for user " + mUser->LoginConst());
            //QMessageBox::information(this, tr("Changing password"), "Password changed for user " + mUser->LoginConst());
        }
        if (!ui->cbUser->isVisible()
                || ui->cbUser->currentData().toString() == db.userName()) {
            db.setPassword(ui->leNewPass1->text());
        }
        accept();
    }
}
