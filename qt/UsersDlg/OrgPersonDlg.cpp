#include "OrgPersonDlg.h"
#include "ui_OrgPersonDlg.h"

#include "CustomerData.h"

#include "../VProject/GlobalSettings.h"

OrgPersonDlg::OrgPersonDlg(CustomerData *aCustomer, CustomerPerson *aPerson, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::OrgPersonDlg),
    mCustomer(aCustomer), mPerson(aPerson)
{
    ui->setupUi(this);
    InitInConstructor();
}

OrgPersonDlg::~OrgPersonDlg()
{
    delete ui;
}

void OrgPersonDlg::InitInConstructor() {
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    ui->lblMidName->setVisible(gCustomers->PersonHasMidName());
    ui->leMidName->setVisible(gCustomers->PersonHasMidName());
    ui->gbDP->setVisible(gCustomers->PersonHasDP());

    // required and disabled
    //QPalette lPaletteReq = ui->leCustomer->palette();
    QPalette lPaletteDis = ui->leCustomer->palette();
    //lPaletteReq.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    lPaletteDis.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);

    ui->leCustomer->setPalette(lPaletteDis);

    //ui->leLastName->setPalette(lPaletteReq);
    //ui->leFirstName->setPalette(lPaletteReq);
    //ui->lePhone1->setPalette(lPaletteReq);
}

void OrgPersonDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        if (!mCustomer) {
            QTimer::singleShot(0, this, SLOT(close()));
        } else {
            QTimer::singleShot(0, this, SLOT(ShowData()));
        }
    }
}

void OrgPersonDlg::ShowData() {
    ui->leCustomer->setText(mCustomer->ShortNameConst());

    if (mPerson) {
        ui->leLastName->setText(mPerson->LastNameConst());
        ui->leFirstName->setText(mPerson->FirstNameConst());
        if (ui->leMidName->isVisible()) {
            ui->leMidName->setText(mPerson->MiddleNameConst());
        }
        ui->lePosition->setText(mPerson->PostConst());

        ui->lePhoneMob->setText(mPerson->TelMobConst());
        ui->leEmail->setText(mPerson->EmailConst());
        ui->lePhone1->setText(mPerson->Tel1Const());
        ui->lePhone2->setText(mPerson->Tel2Const());

        if (ui->gbDP->isVisible()) {
            ui->cbMastD->setCurrentText(mPerson->MastDConst());
            ui->cbPostD->setCurrentText(mPerson->PostDConst());
            ui->leFioD->setText(mPerson->FioDConst());
        }
    }
}

void OrgPersonDlg::Accept() {
    if (ui->leLastName->text().isEmpty()
            && ui->leFirstName->text().isEmpty()) {
        QMessageBox::critical(this, tr("Employee of company"), tr("First name or last name must be specified!"));
        ui->leLastName->setFocus();
        return;
    }

    if (ui->lePhoneMob->text().isEmpty()
            && ui->leEmail->text().isEmpty()
            && ui->lePhone1->text().isEmpty()
            && ui->lePhone2->text().isEmpty()) {
        QMessageBox::critical(this, tr("Employee of company"), tr("One of contact must be specified!"));
        ui->lePhoneMob->setFocus();
        return;
    }

    if (!ui->leEmail->text().isEmpty()
            && !ui->leEmail->text().contains(QRegExp("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$"))) {
        QMessageBox::critical(this, tr("Employee of company"), tr("Invalid format for e-mail!"));
        ui->leEmail->setFocus();
        return;
    }

    bool lIsNew;
    if (lIsNew = !mPerson) {
        mPerson = new CustomerPerson(0, mCustomer->Id(), ui->leLastName->text(), ui->leFirstName->text(), ui->leMidName->text(),
                                     ui->lePosition->text(), ui->lePhone1->text(), ui->lePhone2->text(), ui->lePhoneMob->text(),
                                     ui->leEmail->text(), ui->leFioD->text(), ui->cbPostD->currentText(), ui->cbMastD->currentText(),
                                     QDateTime(), "");
    } else {
        mPerson->setLastName(ui->leLastName->text());
        mPerson->setFirstName(ui->leFirstName->text());
        mPerson->setMiddleName(ui->leMidName->text());
        mPerson->setPost(ui->lePosition->text());
        mPerson->setTel1(ui->lePhone1->text());
        mPerson->setTel2(ui->lePhone2->text());
        mPerson->setTelMob(ui->lePhoneMob->text());
        mPerson->setEmail(ui->leEmail->text());
        mPerson->setFioD(ui->leFioD->text());
        mPerson->setPostD(ui->cbPostD->currentText());
        mPerson->setMastD(ui->cbMastD->currentText());
    }

    bool lNotChanged;
    if (mPerson->SaveData(lNotChanged)) {
        if (lNotChanged) {
            reject();
        } else {
            mPerson->CommitEdit();
            if (lIsNew) {
                mCustomer->PersonsRef().append(mPerson);
            }
            accept();
        }
    } else {
        if (lIsNew) {
            delete mPerson;
            mPerson = NULL;
        } else {
            mPerson->RollbackEdit();
        }
    }
}
