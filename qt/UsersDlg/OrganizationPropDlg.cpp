#include "OrganizationPropDlg.h"
#include "ui_OrganizationPropDlg.h"
#include <QPushButton>

#include "CustomerData.h"
#include "UserRight.h"

#include "../VProject/common.h"
#include "../VProject/GlobalSettings.h"

OrganizationPropDlg::OrganizationPropDlg(CustomerData *aCustomer, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::OrganizationPropDlg),
    mCustomer(aCustomer),
    mJustStarted(true)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    if (mIsNew = !mCustomer) {
        mCustomer = new CustomerData();
    }

    QPalette lPaletteReq = ui->leShortName->palette();
    QPalette lPaletteDis = ui->leShortName->palette();
    lPaletteReq.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    lPaletteDis.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);

    ui->leShortName->setPalette(gUserRight->CanUpdate("v_customer", "shortname")?lPaletteReq:lPaletteDis);
    ui->leFullName->setPalette(gUserRight->CanUpdate("v_customer", "name")?lPaletteReq:lPaletteDis);


    if (gCustomers->HasIsClient()) {
        ui->cbClient->setVisible(true);
        ui->cbClient->setReadOnly(!gUserRight->CanUpdate("v_customer", "is_client"));
    } else {
        ui->cbClient->setVisible(false);
    }

    if (gCustomers->HasIsSupplier()) {
        ui->cbProvider->setVisible(true);
        ui->cbProvider->setReadOnly(!gUserRight->CanUpdate("v_customer", "is_provider"));
    } else {
        ui->cbProvider->setVisible(false);
    }

    //QAbstractItemDelegate *lTableItemDelegate = new PropertiesItemDelegate(this);
    //ui->twCustProps->setItemDelegateForColumn(1, lTableItemDelegate);
}

OrganizationPropDlg::~OrganizationPropDlg()
{
    if (mIsNew && mCustomer) {
        delete mCustomer;
    }
    delete ui;
}

int OrganizationPropDlg::IdCustomer() {
    return mCustomer->Id();
}

void OrganizationPropDlg::showEvent(QShowEvent *event) {
    QFCDialog::showEvent(event);
    if (mJustStarted) {
        mJustStarted = false;
        if (mIsNew
                && (!gUserRight->CanInsert("v_customer")
                    || !gUserRight->CanInsert("v_custdata"))) {
            QTimer::singleShot(0, this, SLOT(close()));
        } else {
            QTimer::singleShot(0, this, SLOT(ShowData()));
        }
    }
}

void OrganizationPropDlg::ShowData() {
    ui->twCustProps->clear();

    if (!mCustomer->DelUserConst().isEmpty()) {
        ui->cbDeleted->setVisible(true);
        ui->cbDeleted->setChecked(true);
        ui->cbDeleted->setReadOnly(!gUserRight->CanUpdate("v_customer", "deldate")
                                    || !gUserRight->CanUpdate("v_customer", "deluser"));
    } else {
        ui->cbDeleted->setVisible(false);
    }

    ui->leShortName->setText(mCustomer->ShortNameConst());
    ui->leFullName->setText(mCustomer->NameConst());
    if (ui->cbClient->isVisible()) {
        ui->cbClient->blockSignals(true);
        ui->cbClient->setCheckState((mCustomer->IsClient() == 1)?Qt::Checked:Qt::Unchecked);
        ui->cbClient->blockSignals(false);
    }
    if (ui->cbProvider->isVisible()) {
        ui->cbProvider->blockSignals(true);
        ui->cbProvider->setCheckState((mCustomer->IsProvider() == 1)?Qt::Checked:Qt::Unchecked);
        ui->cbProvider->blockSignals(false);
    }

    QTableWidgetItem *lItem;
    for (int i = 0; i < mCustomer->PropsConst().length(); i++) {
        ui->twCustProps->insertRow(ui->twCustProps->rowCount());

        lItem = new QTableWidgetItem(mCustomer->PropsConst().at(i)->TitleConst());
        lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lItem->setBackgroundColor(palette().color(QPalette::Window));
        lItem->setFlags(lItem->flags() & ~(Qt::ItemIsSelectable) & ~(Qt::ItemIsEditable));
        ui->twCustProps->setItem(ui->twCustProps->rowCount() - 1, 0, lItem);

        lItem = new QTableWidgetItem(mCustomer->PropsConst().at(i)->ValueConst());
        if (mCustomer->PropsConst().at(i)->Required()) {
            lItem->setBackgroundColor(gSettings->Common.RequiredFieldColor);
        }
        lItem->setData(Qt::UserRole, mCustomer->PropsConst().at(i)->OrderBy());
        lItem->setData(Qt::UserRole + 1, mCustomer->PropsConst().at(i)->Required());
        lItem->setData(Qt::UserRole + 2, mCustomer->PropsConst().at(i)->RegExpConst());
        lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->twCustProps->setItem(ui->twCustProps->rowCount() - 1, 1, lItem);
    }

    ui->twCustProps->resizeColumnsToContents();
    ui->twCustProps->resizeRowsToContents();

    int lHeight = 0;
    for (int i = 0; i < ui->twCustProps->rowCount(); i++) {
        ui->twCustProps->setRowHeight(i, ui->twCustProps->rowHeight(i) - gSettings->Common.SubRowHeight);
        lHeight += ui->twCustProps->rowHeight(i);
    }
    ui->twCustProps->setFixedHeight(lHeight);
    ui->twCustProps->setColumnWidth(0, ui->twCustProps->columnWidth(0) + gSettings->Common.AddColWidth);
}

void OrganizationPropDlg::on_buttonBox_accepted() {
    bool lErr;

    int i;

    QString lFormat;
    for (i = 0; i < ui->twCustProps->rowCount(); i++) {
        if (ui->twCustProps->item(i, 1)->data(Qt::UserRole + 1).toInt()
                && ui->twCustProps->item(i, 1)->text().isEmpty()) {
            if (ui->twCustProps->item(i, 1)->data(Qt::UserRole + 2).isNull()) {
                lFormat = "";
            } else {
                lFormat = "\nFormat: " + ui->twCustProps->item(i, 1)->data(Qt::UserRole + 2).toString();
            }
            QMessageBox::critical(this, tr("Company properties"), ui->twCustProps->item(i, 0)->text() + " " + tr("is required") + lFormat);
            ui->twCustProps->setFocus();
            ui->twCustProps->setCurrentCell(i, 1);
            return;
        }
    }

    for (i = 0; i < ui->twCustProps->rowCount(); i++) {
        if (!ui->twCustProps->item(i, 1)->text().isEmpty()
                && !ui->twCustProps->item(i, 1)->data(Qt::UserRole + 2).isNull()
                && !ui->twCustProps->item(i, 1)->text().contains(QRegExp(ui->twCustProps->item(i, 1)->data(Qt::UserRole + 2).toString()))) {
            QMessageBox::critical(this, tr("Company properties"),
                                  ui->twCustProps->item(i, 0)->text() + " " + tr("must be in format") + " " + ui->twCustProps->item(i, 1)->data(Qt::UserRole + 2).toString());
            ui->twCustProps->setFocus();
            ui->twCustProps->setCurrentCell(i, 1);
            return;
        }
    }

    if (!db.transaction())     {
        gLogger->ShowSqlError(this, tr("Company properties"), tr("Can't start transaction"), db);
        return;
    }
    lErr = false;

    // customers
    mCustomer->setShortName(ui->leShortName->text());
    mCustomer->setName(ui->leFullName->text());
    if (ui->cbClient->isVisible()) {
        mCustomer->setIsClient(ui->cbClient->isChecked()?1:0);
    }
    if (ui->cbProvider->isVisible()) {
        mCustomer->setIsProvider(ui->cbProvider->isChecked()?1:0);
    }

    // customer props
    for (i = 0; i < mCustomer->PropsConst().length(); i++) {
        mCustomer->PropsConst().at(i)->setValue(ui->twCustProps->item(i,1)->text());
    }

    bool lNoChanges;
    if (!mCustomer->SaveData(lNoChanges)) {
        lErr = true;
    } else {
        if (!(lErr = db.commit())) {
            gLogger->ShowSqlError(this, tr("Company properties"), tr("Can't commit"), db);
        } else {
            if (lNoChanges) {
                reject();
            } else {
                mCustomer->CommitEdit();
                if (mIsNew) {
                    gCustomers->CustomerList().append(mCustomer);
                    mIsNew = false;
                }
                accept();
            }
        }
    }

    if (lErr) {
        db.rollback();
        mCustomer->RollbackEdit();
    }
}

void OrganizationPropDlg::on_cbClient_toggled(bool checked) {
    if (!checked && !ui->cbProvider->isChecked()) {
        ui->cbClient->blockSignals(true);
        ui->cbClient->setCheckState(Qt::Checked);
        ui->cbClient->blockSignals(false);
    }
}

void OrganizationPropDlg::on_cbProvider_toggled(bool checked) {
    if (!checked && !ui->cbClient->isChecked()) {
        ui->cbProvider->blockSignals(true);
        ui->cbProvider->setCheckState(Qt::Checked);
        ui->cbProvider->blockSignals(false);
    }
}
