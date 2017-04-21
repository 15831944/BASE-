#include "AcadSupFileRight.h"
#include "ui_AcadSupFileRight.h"
#include "../VProject/UserData.h"

#include "../VProject/GlobalSettings.h"

#include "../Login/Login.h"
#include "../Logger/logger.h"

#include <QComboBox>

AcadSupFileRight::AcadSupFileRight(QWidget *parent) :
    QFCDialog(parent),
    ui(new Ui::AcadSupFileRight)
{
    ui->setupUi(this);

    int cnt;

    QSqlQuery query(
                "select count(*) from v_as_file_right"
                " where id_department is null and login = user", db);
    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "AutoCAD support files", "Select from V_AS_FILE_RIGHT", query.lastError());
    } else {
        if (query.next()) {
            cnt = query.value(0).toInt();
        }
    }

    if (cnt) {
        QSqlQuery query(
                    "select 0, '" + tr("All departments") + "', 0 from dual"
                    " union select id, name, 1 from department where deleted = 0"
                    " order by 3, 2", db);

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(this, "AutoCAD support files", "Select from DEPARTMENT", query.lastError());
        } else {
            while (query.next()) {
                mDDList.insert(query.value(1).toString(), query.value(0).toInt());
            }
        }
    } else {
        QSqlQuery query(
                    "select distinct nvl(b.id, 0), nvl(b.name, '" + tr("All departments") + "'),"
                    " case when b.id is null then 0 else 1 end"
                    " from v_as_file_right a, department b"
                    " where a.id_department = b.id (+)"
                    " and a.login = user"
                    " order by 3, 2", db);

        if (query.lastError().isValid()) {
            gLogger->ShowSqlError(this, "AutoCAD support files", "Select from V_AS_FILE_RIGHT and DEPARTMENT", query.lastError());
        } else {
            while (query.next()) {
                mDDList.insert(query.value(1).toString(), query.value(0).toInt());
            }
        }
    }

    if (mDDList.size() > 1) {
        QPCRDepartmentItemDelegate *idDepartment = new QPCRDepartmentItemDelegate(mDDList, this);
        ui->twRights->setItemDelegateForColumn(0, idDepartment);
    }

    QPCRPersonItemDelegate *idPerson = new QPCRPersonItemDelegate(this);
    ui->twRights->setItemDelegateForColumn(1, idPerson);
}

AcadSupFileRight::~AcadSupFileRight()
{
    delete ui;
}

void AcadSupFileRight::showEvent(QShowEvent* event)
{
    ui->twRights->setColumnWidth(0, 250);
    ui->twRights->setColumnWidth(1, 150);
    ui->twRights->setColumnWidth(2, 20);

    QFCDialog::showEvent(event);

    ui->tbMinus->setEnabled(false);
    ui->twRights->horizontalHeader()->setSectionsClickable(false);

    PopulateData();
}

void AcadSupFileRight::PopulateData() {
    //ui->twRights->mo

    ui->twRights->setColumnHidden(3, true);
    ui->twRights->setColumnHidden(4, true);
    ui->twRights->setColumnHidden(5, true);
    ui->twRights->setColumnHidden(6, true);
    ui->twRights->setColumnHidden(7, true);

    QSqlQuery query(
                "select nvl(b.name, '" + tr("All departments") + "'), pp.GetUserNameDisp(login), admin_option,"
                " case when login = user then 1 else 0 end,"
                " case when exists (select 1 from v_as_file_right where (nvl(id_department, 0) = nvl(a.id_department, 0) or id_department is null) and login = user and admin_option = 1) then 1 else 0 end,"
                " a.id, case when b.id is null then 0 else 1 end"
                " from v_as_file_right a, department b"
                " where a.id_department = b.id (+)"
                " order by 6, 1, 2", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError(this, "AutoCAD support files", "AcadSupFileRight::PopulateData()", query.lastError());
    } else {
        QTableWidgetItem *twi;
        while (query.next()) {
            ui->twRights->insertRow(ui->twRights->rowCount());

            twi = new QTableWidgetItem(query.value(0).toString());
            if (query.value(3).toInt() == 1 || query.value(4).toInt() != 1) {
                twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }
            ui->twRights->setItem(ui->twRights->rowCount() - 1, 0, twi);

            twi = new QTableWidgetItem(query.value(1).toString());
            if (query.value(3).toInt() == 1 || query.value(4).toInt() != 1) {
                twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }
            ui->twRights->setItem(ui->twRights->rowCount() - 1, 1, twi);

            twi = new QTableWidgetItem();
            if (query.value(3).toInt() == 1 || query.value(4).toInt() != 1) {
                twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
            } else {
                twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            }
            twi->setCheckState((query.value(2).toInt() == 1)?Qt::Checked:Qt::Unchecked);
            ui->twRights->setItem(ui->twRights->rowCount() - 1, 2, twi);

            // original values
            twi = new QTableWidgetItem(query.value(0).toString());
            ui->twRights->setItem(ui->twRights->rowCount() - 1, 3, twi);

            twi = new QTableWidgetItem(query.value(1).toString());
            ui->twRights->setItem(ui->twRights->rowCount() - 1, 4, twi);

            twi = new QTableWidgetItem(query.value(2).toString());
            ui->twRights->setItem(ui->twRights->rowCount() - 1, 5, twi);

            // id
            twi = new QTableWidgetItem(query.value(5).toString());
            ui->twRights->setItem(ui->twRights->rowCount() - 1, 6, twi);
        }
    }

    ui->twRights->resizeRowsToContents();
}

void AcadSupFileRight::Accept() {
    bool lDoAccept = false;

    int i;
    bool lErr = false, lChanged = false;

    if (db.transaction()) {

        QSqlQuery qInsert(db), qUpdate(db);
        qInsert.prepare("insert into v_as_file_right(type, id_department, login, admin_option)"
                        "  values (:type, :id_department, :login, :admin_option)");
        if (qInsert.lastError().isValid()) {
            gLogger->ShowSqlError(this, "AutoCAD support files", "Insert into V_AS_FILE_RIGHT - prepare", qInsert.lastError());
            lErr = true;
        } else {
            qInsert.bindValue(":type", 0); // for future use

            qUpdate.prepare("update v_as_file_right set id_department = :id_department, login = :login, admin_option = :admin_option where id = :id");
            if (qUpdate.lastError().isValid()) {
                gLogger->ShowSqlError(this, "AutoCAD support files", "Update V_AS_FILE_RIGHT - prepare", qUpdate.lastError());
                lErr = true;
            }
        }

        if (!lErr) {
            ui->twRights->setCurrentCell(0, 1);
            ui->twRights->setCurrentCell(0, 0);

            for (i = 0; i < ui->twRights->rowCount(); i++) {
                if (ui->twRights->item(i, 1)
                        && (ui->twRights->item(i, 1)->flags() & Qt::ItemIsEditable) == Qt::ItemIsEditable) {
                    QString depart, user, origDepart, origUser;
                    int admin, origAdmin;
                    bool lNew = false;
                    if (ui->twRights->item(i, 0)) depart = ui->twRights->item(i, 0)->text();
                    if (ui->twRights->item(i, 1)) user = ui->twRights->item(i, 1)->text();
                    if (ui->twRights->item(i, 2)->checkState() == Qt::Checked) {
                        admin = 1;
                    } else {
                        admin = 0;
                    }

                    if (ui->twRights->item(i, 6)) {
                        origDepart = ui->twRights->item(i, 3)->text();
                        origUser = ui->twRights->item(i, 4)->text();
                        origAdmin = ui->twRights->item(i, 5)->text().toInt();
                    } else {
                        lNew = true;
                    }

                    if (lNew) {
                        if (!depart.isEmpty() && !user.isEmpty()) {
                            lChanged = true;
                            int idDepart = 0;
                            QMap<QString, int>::const_iterator itr = mDDList.find(depart);
                            if (itr != mDDList.end() && itr.key() == depart) {
                                idDepart = itr.value(); // it can be 0
                            }
                            if (idDepart) {
                                qInsert.bindValue(":id_department", idDepart);
                            } else {
                                qInsert.bindValue(":id_department", QVariant());
                            }

                            UserData * lUserData = gUsers->FindByName(user);
                            if (lUserData) {
                                qInsert.bindValue(":login", lUserData->LoginConst());
                                qInsert.bindValue(":admin_option", admin);
                                if (!qInsert.exec()) {
                                    gLogger->ShowSqlError(this, "AutoCAD support files", "Insert into V_AS_FILE_RIGHT - execute", qInsert.lastError());
                                    lErr = true;
                                    break;
                                }
                            } else {
                                QMessageBox::critical(this, "AutoCAD support files", tr("User") + " " + user + " " + tr("doesn't exist!"));
                                lErr = true;
                                break;
                            }
                        }
                    } else {
                        if (depart != origDepart || user != origUser || admin != origAdmin) {
                            lChanged = true;
                            int idDepart = 0;
                            QMap<QString, int>::const_iterator itr = mDDList.find(depart);
                            if (itr != mDDList.end() && itr.key() == depart) {
                                idDepart = itr.value();
                            }
                            if (idDepart) {
                                qUpdate.bindValue(":id_department", idDepart);
                            } else {
                                qUpdate.bindValue(":id_department", QVariant());
                            }
                            UserData * lUserData = gUsers->FindByName(user);
                            if (lUserData) {
                                qUpdate.bindValue(":login", lUserData->LoginConst());
                                qUpdate.bindValue(":admin_option", admin);
                                qUpdate.bindValue(":id", ui->twRights->item(i, 6)->text().toInt());
                                if (!qUpdate.exec()) {
                                    gLogger->ShowSqlError(this, "AutoCAD support files", "Update V_AS_FILE_RIGHT - prepare", qUpdate.lastError());
                                    lErr = true;
                                    break;
                                }
                            } else {
                                QMessageBox::critical(this, "AutoCAD support files", tr("User") + " " + user + " " + tr("doesn't exist!"));
                                lErr = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (!lErr && !mListForDel.isEmpty()) {
            lChanged = true;

            QSqlQuery qDelete(db);
            qDelete.prepare("delete from v_as_file_right where id = :id");
            if (qDelete.lastError().isValid()) {
                gLogger->ShowSqlError(this, "AutoCAD support files", "Delete from V_AS_FILE_RIGHT - prepare", qDelete.lastError());
                lErr = true;
            } else {
                for (i = 0; i < mListForDel.length(); i++) {
                    qDelete.bindValue(":id", mListForDel.at(i));
                    if (!qDelete.exec()) {
                        gLogger->ShowSqlError(this, "AutoCAD support files", "Delete from V_AS_FILE_RIGHT - execute", qDelete.lastError());
                        lErr = true;
                        break;
                    }
                }
            }

        }

        if (lChanged && !lErr) {
            if (!db.commit()) {
                gLogger->ShowSqlError(this, "AutoCAD support files", tr("Can't commit"), db.lastError());
            } else {
                lDoAccept = true;
            }
        } else {
            db.rollback();
            if (!lChanged) lDoAccept = true;
        }

    } else {
        gLogger->ShowSqlError(this, "AutoCAD support files", tr("Can't start transaction"), db.lastError());
    }
    if (lDoAccept) accept();
}

void AcadSupFileRight::on_actionAdd_triggered()
{
    QTableWidgetItem *twi;
    ui->twRights->insertRow(ui->twRights->rowCount());

    twi = new QTableWidgetItem();
    twi->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    twi->setCheckState(Qt::Unchecked);
    ui->twRights->setItem(ui->twRights->rowCount() - 1, 2, twi);

    ui->twRights->setRowHeight(ui->twRights->rowCount() - 1, ui->twRights->rowHeight(ui->twRights->rowCount() - 2));
    ui->twRights->setCurrentCell(ui->twRights->rowCount() - 1, 0);
}

void AcadSupFileRight::on_actionDel_triggered()
{
    QTableWidgetItem *twi;

    if (!ui->twRights->item(ui->twRights->currentRow(), 1)
            || (ui->twRights->item(ui->twRights->currentRow(), 1)->flags() & Qt::ItemIsEditable) == Qt::ItemIsEditable) {
        // can delete any new row or any row with editableee
        if (twi = ui->twRights->item(ui->twRights->currentRow(), 6)) {
            // delete from base; in other case it is new record, need not delete
            mListForDel.append(twi->text().toInt());
        }
        ui->twRights->removeRow(ui->twRights->currentRow());
    }
}

void AcadSupFileRight::on_twRights_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    ui->tbMinus->setEnabled(!ui->twRights->item(ui->twRights->currentRow(), 1)
                            || (ui->twRights->item(ui->twRights->currentRow(), 1)->flags() & Qt::ItemIsEditable) == Qt::ItemIsEditable);
}

QPCRDepartmentItemDelegate::QPCRDepartmentItemDelegate(QMap<QString, int> aDDList, QWidget * parent) :
    QStyledItemDelegate(parent)
{
    mDDList = aDDList;
}

QWidget * QPCRDepartmentItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QComboBox *ul = new QComboBox(parent);

    QMap<QString, int>::const_iterator itr = mDDList.constBegin();
    while (itr != mDDList.constEnd()) {
        ul->addItem(itr.key());
        itr++;
    }
    return ul;
}

QPCRPersonItemDelegate::QPCRPersonItemDelegate(QWidget * parent) :
    QStyledItemDelegate(parent)
{
}

QWidget * QPCRPersonItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
    QComboBox *ul = new QComboBox(parent);
    ul->setEditable(true);
    foreach (const UserData * lUserData, gUsers->UserListConst()) ul->addItem(lUserData->NameConst());
    return ul;
}
