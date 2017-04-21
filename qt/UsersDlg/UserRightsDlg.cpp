#include "UserRightsDlg.h"
#include "ui_UserRightsDlg.h"

#include <QComboBox>

#include "UserData.h"
#include "UserRight.h"

#include "../VProject/common.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/HomeData.h"

UserRightsDlg::UserRightsDlg(UserData *aUser, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::UserRightsDlg),
    mUser(aUser), mJustStarted(true)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    QPalette lPalette = ui->twRights->palette();
    QPalette lPaletteReq = lPalette;
    lPalette.setColor(QPalette::Base, palette().color(QPalette::Window));
    ui->twRights->setPalette(lPalette);
    lPaletteReq.setColor(QPalette::Base, gSettings->Common.RequiredFieldColor);
    ui->lePass->setPalette(lPaletteReq);

    if (gSettings->ServerSecure.AskPassForUserMan == -1) {
        gSettings->ServerSecure.AskPassForUserMan = gHomeData->Get("ASK_PASS_FOR_USER_MAN").toInt();
    }

    ui->wdYourPass->setVisible(gSettings->ServerSecure.AskPassForUserMan);

    if (mCanGrantAnyRole = gUserRight->CanGrantAnyRole()) {
        UserRightsItemDelegate *lItemDelegate = new UserRightsItemDelegate(this, this);
        ui->twRights->setItemDelegate(lItemDelegate);
        connect(lItemDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));
    }
}

UserRightsDlg::~UserRightsDlg()
{
    delete ui;
}

void UserRightsDlg::ShowData() {
    ui->leName->setText(mUser->NameConst());
    ui->twRights->setRowCount(0);

    QSqlQuery queryGrp("select id, description from v_my_role_group order by id", db);
    if (queryGrp.lastError().isValid()) {
        gLogger->ShowSqlError(tr("User permissions"), queryGrp);
        return;
    }

    QSqlQuery queryRole(db);
    queryRole.prepare("select role_name, description from v_my_role"
                      " where id_group = :id_group order by id");
    if (queryRole.lastError().isValid()) {
        gLogger->ShowSqlError(tr("User permissions"), queryRole);
        return;
    }

    QSqlQuery queryUserRole(db);

    queryUserRole.prepare("select role_name, description from v_my_role_granted c"
                          " where grantee = :login and id_group = :id_group");

    if (queryUserRole.lastError().isValid()) {
        gLogger->ShowSqlError(tr("User permissions"), queryUserRole);
        return;
    }

    while (queryGrp.next()) {
        queryRole.bindValue(":id_group", queryGrp.value("id").toInt());
        if (!queryRole.exec()) {
            gLogger->ShowSqlError(tr("User permissions"), queryRole);
        } else {
            QList<QPair<QString, QString>> lRoles;
            while (queryRole.next()) {
                lRoles.append(qMakePair(queryRole.value("description").toString(), queryRole.value("role_name").toString()));
            }
            mAllRoles.append(lRoles);

            queryUserRole.bindValue(":login", mUser->LoginConst());
            queryUserRole.bindValue(":id_group", queryGrp.value("id").toInt());
            if (!queryUserRole.exec()) {
                gLogger->ShowSqlError(tr("User permissions"), queryUserRole);
                break;
            } else {
                QTableWidgetItem *lItem;
                bool lHasAny = false;

                while (queryUserRole.next()) {
                    ui->twRights->insertRow(ui->twRights->rowCount());

                    lItem = new QTableWidgetItem(queryGrp.value("description").toString());
                    lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
                    ui->twRights->setItem(ui->twRights->rowCount() - 1, 0, lItem);

                    lItem = new QTableWidgetItem(queryUserRole.value("description").toString());
                    lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    if (!mCanGrantAnyRole) {
                        lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
                    } else {
                        lItem->setBackgroundColor(ui->leName->palette().color(QPalette::Base));
                    }
                    ui->twRights->setItem(ui->twRights->rowCount() - 1, 1, lItem);
                    lItem->setData(Qt::UserRole, queryUserRole.value("role_name").toString());
                    lItem->setData(Qt::UserRole + 1, queryUserRole.value("role_name").toString());

                    lHasAny = true;
                }
                if (!lHasAny) {
                    ui->twRights->insertRow(ui->twRights->rowCount());

                    lItem = new QTableWidgetItem(queryGrp.value("description").toString());
                    lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
                    ui->twRights->setItem(ui->twRights->rowCount() - 1, 0, lItem);

                    lItem = new QTableWidgetItem();
                    lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    if (!mCanGrantAnyRole) {
                        lItem->setFlags(lItem->flags() & ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
                    } else {
                        lItem->setBackgroundColor(ui->leName->palette().color(QPalette::Base));
                    }
                    ui->twRights->setItem(ui->twRights->rowCount() - 1, 1, lItem);
                }
            }
        }
    }

    ui->twRights->resizeColumnsToContents();
    ui->twRights->resizeRowsToContents();

    int lHeight = 0;
    for (int i = 0; i < ui->twRights->rowCount(); i++) {
        lHeight += ui->twRights->rowHeight(0);
    }

    ui->twRights->setFixedHeight(lHeight);
}

void UserRightsDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        mJustStarted = false;

        ShowData();
    }
}

void UserRightsDlg::OnCommitData(QWidget *editor) {
    if (qobject_cast<QComboBox *> (editor)) {
        ui->twRights->item(qobject_cast<QComboBox *> (editor)->itemData(0, Qt::UserRole + 1).toInt(), 1)->setData(Qt::UserRole + 1, qobject_cast<QComboBox *> (editor)->currentData().toString());
    }
}

UserRightsItemDelegate::UserRightsItemDelegate(QWidget *parent, UserRightsDlg *aUserRightsDlg) :
    QStyledItemDelegate(parent),
    mUserRightsDlg(aUserRightsDlg)
{

}

QWidget *UserRightsItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QComboBox *cb = new QComboBox(parent);
    QLineEdit *le = new QLineEdit();

    QPalette lPalette = cb->palette();
    lPalette.setColor(QPalette::Base, le->palette().color(QPalette::Base));
    cb->setPalette(lPalette);
    delete le;

    const QList<QPair<QString, QString>> & lRoles = mUserRightsDlg->AllRolesConst().at(index.row());

    if (index.row()) cb->addItem(""); // add empty for not first row
    for (int i = 0; i < lRoles.length(); i++) {
        cb->addItem(lRoles.at(i).first, lRoles.at(i).second);
    }
    if (cb->count())
        cb->setItemData(0, index.row(), Qt::UserRole + 1);
    return cb;
}

void UserRightsDlg::Accept() {
    if (ui->wdYourPass->isVisible()
            && ui->lePass->text() != db.password()) {
        QMessageBox::critical(this, tr("User permissions"), tr("Your password is wrong!"));
        ui->lePass->setFocus();
        ui->lePass->selectAll();
        return;
    }

    if (ui->twRights->rowCount()
            && ui->twRights->item(0, 1)->data(Qt::UserRole + 1).isNull()) {
        QMessageBox::critical(this, tr("User permissions"), ui->twRights->item(0, 0)->text() + " " + tr("must be specified!"));
        ui->twRights->setFocus();
        ui->twRights->setCurrentCell(0, 1);
        return;
    }

    bool lIsErr = false;
    for (int i = 0; i < ui->twRights->rowCount(); i++) {
        QString lOldRight = ui->twRights->item(i, 1)->data(Qt::UserRole).toString();
        QString lNewRight = ui->twRights->item(i, 1)->data(Qt::UserRole + 1).toString();

        if (lOldRight != lNewRight) {
            if (!lOldRight.isEmpty()) {
                QSqlQuery qRevoke(db);
                if (!qRevoke.exec("revoke " + lOldRight + " from " + mUser->LoginConst())) {
                    gLogger->ShowSqlError(tr("User permissions"), qRevoke);
                    lIsErr = true;
                    break;
                }
            }

            if (!lNewRight.isEmpty()) {
                QSqlQuery qGrant(db);
                if (!qGrant.exec("grant " + lNewRight + " to " + mUser->LoginConst())) {
                    gLogger->ShowSqlError(tr("User permissions"), qGrant);
                    lIsErr = true;
                    break;
                }
            }
        }
    }

    if (!lIsErr
            && !ui->leNewPass1->text().isEmpty()) {
        if (ui->leNewPass1->text() != ui->leNewPass2->text()) {
            QMessageBox::critical(this, tr("User permissions"), tr("Passwords must be equal!"));
            ui->leNewPass2->setFocus();
            ui->leNewPass2->selectAll();
            lIsErr = true;
        } else {
            if (ui->leNewPass1->text().at(0).toUpper() < 'A'
                    || ui->leNewPass1->text().at(0).toUpper() > 'Z') {
                QMessageBox::critical(this, tr("User permissions"), tr("Password must start with latin letter!"));
                ui->leNewPass1->setFocus();
                ui->leNewPass1->selectAll();
                lIsErr = true;
            } else {
                QSqlQuery qPassword(db);
                qPassword.prepare("alter user " + mUser->LoginConst() + " identified by " + ui->leNewPass1->text() + " account unlock");
                if (qPassword.lastError().isValid()) {
                    gLogger->ShowSqlError(tr("User permissions"), qPassword);
                    lIsErr = true;
                } else {
                    if (qPassword.exec()) {
                        gLogger->LogErrorToDb("Password changed for user " + mUser->LoginConst());
                        if (mUser->LoginConst() == db.userName()) {
                            db.setPassword(ui->leNewPass1->text());
                        }
                    } else {
                        gLogger->ShowSqlError(tr("User permissions"), qPassword);
                        lIsErr = true;
                    }
                }
            }
        }
    }

    if (lIsErr) {
        ShowData();
    } else {
        gHomeData->Set("NEED_UPDATE_RIGHTS", "1");
        accept();
    }
}
