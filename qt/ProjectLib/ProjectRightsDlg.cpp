#include "ProjectRightsDlg.h"
#include "ui_ProjectRightsDlg.h"

#include <QMenu>
#include <QTimer>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>

#include "../UsersDlg/UserData.h"
#include "../VProject/common.h"
#include "../UsersDlg/UserPropDlg.h"
#include "../VProject/GlobalSettings.h"
#include "../UsersDlg/UserRight.h"
#include "../ProjectLib/ProjectData.h"

ProjectRightsDlg::ProjectRightsDlg(UserListType aULT, const ProjectData *PrjData, QWidget *parent) :
    QFCDialog(parent, false),
    mJustStarted(true),
    mULT(aULT),
    ui(new Ui::ProjectRightsDlg)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    mIdProj = PrjData->Id();

    ui->lineEdit_1->setText(QString::number(mIdProj));
    ui->lineEdit_2->setText(PrjData->FullShortName());
}

ProjectRightsDlg::~ProjectRightsDlg() {
    delete ui;
}

void ProjectRightsDlg::showEvent(QShowEvent *event) {
    QFCDialog::showEvent(event);

    lPaletteDis = ui->tableWidget->palette();
    lPaletteDis.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);

    if (mJustStarted) {
        switch (mULT) {
        case PRDRights:
            if (!gUserRight->CanSelect("v_project_right")) {
                QTimer::singleShot(0, this, SLOT(close()));
            } else {
                QTimer::singleShot(0, this, SLOT(ShowData()));
            }
            break;
        case PRDEnv:
            if (!gUserRight->CanSelect("v_project_env")) {
                QTimer::singleShot(0, this, SLOT(close()));
            } else {
                QTimer::singleShot(0, this, SLOT(ShowData()));
            }
            break;
        }
        mJustStarted = false;
    }

}

void ProjectRightsDlg::resizeEvent(QResizeEvent *event) {
    QFCDialog::resizeEvent(event);
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width()
                                    - 4
                                    - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));
}

void ProjectRightsDlg::ShowData() {
    //ui->tableWidget->setColumnWidth(1, 26);
    QSqlQuery query(db);
    //const QString & lCurrentUser = db.userName();
    ui->tbMinus->setEnabled(false);
    ui->tbPlus->setEnabled(mULT == PRDRights
                                && gUserRight->CanInsert("v_project_right")
                           || mULT == PRDEnv
                                && gUserRight->CanInsert("v_project_env"));

    switch (mULT) {
    case PRDRights:
        query.prepare("select id, login, \"RIGHT\" from v_project_right where id_project = :id_project");
        break;
    case PRDEnv:
        query.prepare("select id, login from v_project_env where id_project = :id_project");
        break;
    }

    if (query.lastError().isValid())
        gLogger->ShowSqlError(QObject::tr("Project rights"), query);
    else {
        query.bindValue(":id_project", mIdProj);
        if (!query.exec())
            gLogger->ShowSqlError(QObject::tr("Project rights"), query);
        else {
            while (query.next()) {
                lItem = new QTableWidgetItem(gUsers->GetName(query.value("login").toString()));
//                if (lCurrentUser == query.value("login").toString() && query.value("RIGHT").toInt() == 3)
//                {
//                    lItem->setFlags(lItem->flags() & ~(Qt::ItemIsSelectable) & ~(Qt::ItemIsEnabled));
//                }
                lItem->setData(Qt::UserRole, query.value("id"));
                lItem->setData(Qt::UserRole + 1, query.value("login"));
                if (gUsers->GetName(query.value("login").toString()).contains(QRegExp("[א-ת]")))
                    lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                else
                    lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                ui->tableWidget->insertRow(ui->tableWidget->rowCount());
                //ui->tableWidget->setRowHeight(ui->tableWidget->rowCount() - 1, ui->tableWidget->rowHeight(ui->tableWidget->rowCount()-1) - 5);
                ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, lItem);
            }
        }
    }

    QApplication::processEvents();
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width()
                                    - 4
                                    - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));

    if(!ui->tbPlus->isEnabled()) {
        ui->tableWidget->setPalette(lPaletteDis);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
         //ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
         //ui->tableWidget->setPalette(lPaletteReq);
    }

    ui->tableWidget->resizeRowsToContents();
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        ui->tableWidget->setRowHeight(i, ui->tableWidget->rowHeight(i) - 5);
    }

    //ui->tableWidget->setCurrentCell(-1, -1);
}


void ProjectRightsDlg::on_tableWidget_itemSelectionChanged() {
    if(ui->tableWidget->selectedItems().count() > 0)
        ui->tbMinus->setEnabled(true);
    else
        ui->tbMinus->setEnabled(false);

}

void ProjectRightsDlg::on_tbPlus_clicked() {
    QList <UserData *> lSelected;
    QStringList lIgnoreLogins;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        if (!ui->tableWidget->isRowHidden(i))
            lIgnoreLogins.append(ui->tableWidget->item(i, 0)->data(Qt::UserRole + 1).toString());
    }
    //lIgnoreLogins.append(db.userName());

    if (gUsers->SelectUsers(lSelected, &lIgnoreLogins)) {
        //QStringList lStrsings;
        for (int i = 0; i < lSelected.length(); i++) {
            lItem = new QTableWidgetItem(lSelected.at(i)->NameConst());
            lItem->setData(Qt::UserRole + 1, lSelected.at(i)->LoginConst());
            if (lSelected.at(i)->NameConst().contains(QRegExp("[א-ת]")))
                lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            else
                lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());
            //ui->tableWidget->setRowHeight(ui->tableWidget->rowCount()-1, ui->tableWidget->rowHeight(ui->tableWidget->rowCount()-1) - 5);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, lItem);
        }
        ui->tableWidget->resizeRowsToContents();
        for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
            ui->tableWidget->setRowHeight(i, ui->tableWidget->rowHeight(i) - 5);
        }

        QApplication::processEvents();
        ui->tableWidget->setColumnWidth(0, ui->tableWidget->width() - ui->tableWidget->columnWidth(1)
                                        - 4
                                        - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));
        QApplication::processEvents();
        ui->tableWidget->setColumnWidth(0, ui->tableWidget->width() - ui->tableWidget->columnWidth(1)
                                        - 4
                                        - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));
    }
}

void ProjectRightsDlg::on_actionDelete_triggered() {
    if (ui->tbMinus->isEnabled())
        on_tbMinus_clicked();
}

void ProjectRightsDlg::on_actionProperties_triggered() {
    QList<QTableWidgetItem *> lSelected;
     lSelected = ui->tableWidget->selectedItems();
     if (!lSelected.isEmpty()) {
         UserData *lUser = gUsers->FindByName(ui->tableWidget->item(lSelected.at(0)->row(), 0)->text());
         if (lUser) {
             UserPropDlg w(lUser, this);
             w.exec();
         }
     }
 }

void ProjectRightsDlg::on_tbMinus_clicked() {
    QList<QTableWidgetItem *> lSelected = ui->tableWidget->selectedItems();
    for (int i = 0; i < lSelected.count(); i++)
        ui->tableWidget->setRowHidden(lSelected.at(i)->row(), true);
    ui->tbMinus->setEnabled(false);

    QApplication::processEvents();
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width() - ui->tableWidget->columnWidth(1)
                                    - 4
                                    - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));
    QApplication::processEvents();
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width() - ui->tableWidget->columnWidth(1)
                                    - 4
                                    - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));
}

void ProjectRightsDlg::on_tableWidget_customContextMenuRequested(const QPoint &pos) {
    if(ui->tableWidget->selectedItems().count() > 0) {
        QMenu lPopup;
        lPopup.addAction(ui->actionDelete);
        lPopup.addAction(ui->actionProperties);
        lPopup.exec(QCursor::pos());
    }
}

void ProjectRightsDlg::on_tableWidget_cellDoubleClicked(int row, int column) {
    on_actionProperties_triggered();
}

void ProjectRightsDlg::on_buttonBox_accepted() {
    bool qInsertPrepared = false, qDeletePrepared = false;
    bool lErr;
    if (!db.transaction()) {
        gLogger->ShowSqlError(this, tr("Project rights"), tr("Can't start transaction"), db);
        return;
    }
    QSqlQuery qInsert(db);
    QSqlQuery qDelete(db);
    lErr = false;
    for (int i = 0; i < ui->tableWidget->rowCount() && !lErr; i++) {
        if (!ui->tableWidget->isRowHidden(i)) {
            if(ui->tableWidget->item(i, 0)->data(Qt::UserRole).isNull()) {
                if (!qInsertPrepared) {
                    switch (mULT) {
                    case PRDRights:
                        qInsert.prepare("insert into v_project_right (login, \"RIGHT\", id_project) values (:login, 3, :id_project)");
                        break;
                    case PRDEnv:
                        qInsert.prepare("insert into v_project_env (login, id_project) values (:login, :id_project)");
                        break;
                    }
                    qInsertPrepared = true;
                    if (qInsert.lastError().isValid()) {
                        lErr = true;
                        gLogger->ShowSqlError(tr("Project rights/Inserting"), qInsert);
                        break;
                    }
                }
                qInsert.bindValue(":id_project", mIdProj);
                qInsert.bindValue(":login",  ui->tableWidget->item(i, 0)->data(Qt::UserRole + 1));
                if (!qInsert.exec()) {
                    lErr = true;
                    gLogger->ShowSqlError(tr("Project rights/Inserting"), qInsert);
                    break;
                }
            }
        } else {
            if(!ui->tableWidget->item(i, 0)->data(Qt::UserRole).isNull()) {
                if(!qDeletePrepared) {
                    switch (mULT) {
                    case PRDRights:
                        qDelete.prepare("delete from v_project_right where id = :id");
                        break;
                    case PRDEnv:
                        qDelete.prepare("delete from v_project_env where id = :id");
                        break;
                    }
                    qDeletePrepared =true;
                    if (qDelete.lastError().isValid()) {
                        lErr = true;
                        gLogger->ShowSqlError(tr("Project rights/Deleting"), qDelete);
                        break;
                    }
                }
                qDelete.bindValue(":id",  ui->tableWidget->item(i, 0)->data(Qt::UserRole));
                if (!qDelete.exec()) {
                    lErr = true;
                    gLogger->ShowSqlError(tr("Project rights/Deleting"), qDelete);
                    break;
                }
            }
        }
    }
    if (!lErr) {
        if (!db.commit()) {
            gLogger->ShowSqlError(this, tr("Project rights"), tr("Can't commit"), db);
        } else {
            accept();
        }
    } else {
        db.rollback();
    }
}
