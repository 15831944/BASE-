#include <QMenu>
#include "PlotRightsDlg.h"
#include "ui_PlotRightsDlg.h"
#include "qcolor.h"

#include <QTimer>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>

#include "../UsersDlg/UserData.h"
#include "../VProject/common.h"
//#include <QTableWidgetItem>
#include "../UsersDlg/UserPropDlg.h"
#include "../VProject/GlobalSettings.h"

PlotRightsDlg::PlotRightsDlg(int ID, QWidget *parent) :
    QFCDialog(parent, false),
    mJustStarted(true),
    mIdPlot(ID),
  ui(new Ui::PlotRightsDlg)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
}

PlotRightsDlg::~PlotRightsDlg()
{
    delete ui;
}
void PlotRightsDlg::showEvent(QShowEvent* event)
{
    QFCDialog::showEvent(event);

    lPaletteDefault = ui->tableWidget->palette();
    lPaletteDis = lPaletteDefault;
    lPaletteDis.setColor(QPalette::Base, gSettings->Common.DisabledFieldColor);
    if (mJustStarted)
    {
        QTimer::singleShot(0, this, SLOT(ShowData()));
        mJustStarted = false;
    }
}
void PlotRightsDlg::resizeEvent(QResizeEvent *event)
{
    QFCDialog::resizeEvent(event);
    //ui->tableWidget->setColumnWidth(0,ui->tableWidget->width()-26-19);
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width() - ui->tableWidget->columnWidth(1)
                                    - 4
                                    - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));
}

void PlotRightsDlg::ShowData() {
    ui->cbUseRights->setEnabled(false);
    ui->tableWidget->setColumnWidth(1, 26);
    get_Restrict = false;
    QSqlQuery query(db);
    const QString &lCurrentUser = db.userName();
    query.prepare("select restrict_rights, cruser from v_plot_simple where id = :id_plot");
    if (query.lastError().isValid())
        gLogger->ShowSqlError(tr("Document access"), query);
    else
    {
        query.bindValue(":id_plot", mIdPlot);
        if (!query.exec())
            gLogger->ShowSqlError(tr("Document access"), query);
        else
        {
            if (query.next())
            {
                ui->cbUseRights->setChecked(query.value("restrict_rights").toBool());
                get_Restrict = query.value("restrict_rights").toBool();
                if (!ui->cbUseRights->isChecked())
                {
                    if (lCurrentUser == query.value("cruser").toString() || gUsers->FindByLogin(lCurrentUser)->IsGIP() || gUsers->FindByLogin(lCurrentUser)->IsBoss())
                        ui->cbUseRights->setEnabled(true);
                }
                else
                {
                    if (gUsers->FindByLogin(lCurrentUser)->IsBoss())
                        ui->cbUseRights->setEnabled(true);
                }
            }
            else
                gLogger->ShowError(tr("Document access"), QObject::tr("Data not found") + "\nv_plot_comments: id = " + QString::number(mIdPlot));
        }
    }
    query.prepare("select id, login, admin_option from v_plot_user where id_plot = :id_plot");
    if (query.lastError().isValid())
        gLogger->ShowSqlError(QObject::tr("Document access"), query);
    else
    {
        query.bindValue(":id_plot", mIdPlot);
        if (!query.exec())
            gLogger->ShowSqlError(QObject::tr("Document access"), query);
        else
        {
            ui->tableWidget->setSortingEnabled(false);
            while (query.next())
            {
                lItem = new QTableWidgetItem(gUsers->GetName(query.value("login").toString()));
                rItem = new QTableWidgetItem("");
                if (lCurrentUser == query.value("login").toString() && query.value("admin_option").toInt() == 1)
                {
                    lItem->setFlags(lItem->flags() & ~(Qt::ItemIsSelectable) & ~(Qt::ItemIsEnabled));
                    rItem->setFlags(lItem->flags() & ~(Qt::ItemIsSelectable) & ~(Qt::ItemIsEnabled));
                    ui->cbUseRights->setEnabled(true);
                }
                lItem->setData(Qt::UserRole,query.value("id"));
                lItem->setData(Qt::UserRole+1,query.value("login"));
                lItem->setData(Qt::UserRole+2,query.value("admin_option"));
                if (gUsers->GetName(query.value("login").toString()).contains(QRegExp("[א-ת]")))
                    lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                else
                    lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                rItem->setCheckState((query.value("admin_option").toInt() == 1) ? Qt::Checked : Qt::Unchecked);
                rItem->setTextAlignment(Qt::AlignCenter);
                ui->tableWidget->insertRow(ui->tableWidget->rowCount());
                //ui->tableWidget->setRowHeight(ui->tableWidget->rowCount()-1, ui->tableWidget->rowHeight(ui->tableWidget->rowCount()-1) - 5);
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,0,lItem);
                ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,1,rItem);

            }
            ui->tableWidget->setSortingEnabled(true);
        }
    }

    if(!ui->cbUseRights->isEnabled())
    {
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            ui->tableWidget->item(i,1)->setFlags(ui->tableWidget->item(i,1)->flags() /*& ~(Qt::ItemIsSelectable)*/ & ~(Qt::ItemIsUserCheckable));
        }
        ui->tableWidget->setPalette(lPaletteDis);
        ui->tbPlus->setEnabled(false);
        ui->tbMinus->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        if (ui->cbUseRights->isChecked())
        {
            ui->tbPlus->setEnabled(true);
            ui->tbMinus->setEnabled(false);
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            ui->tableWidget->setPalette(lPaletteDefault);
        }
        else
        {
            for (int i = 0; i < ui->tableWidget->rowCount(); i++)
            {
                ui->tableWidget->item(i,1)->setFlags(ui->tableWidget->item(i,1)->flags() /*& ~(Qt::ItemIsSelectable)*/ & ~(Qt::ItemIsUserCheckable));
            }
            ui->tableWidget->setPalette(lPaletteDis);
            ui->tbPlus->setEnabled(false);
            ui->tbMinus->setEnabled(false);
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }
    }

    ui->tableWidget->resizeRowsToContents();
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        ui->tableWidget->setRowHeight(i, ui->tableWidget->rowHeight(i) - 5);
    }

    QApplication::processEvents();
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width() - ui->tableWidget->columnWidth(1)
                                    - 4
                                    - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));

    ui->tableWidget->setCurrentCell(-1,-1);
}

void PlotRightsDlg::on_cbUseRights_clicked()
{
    ui->tableWidget->setCurrentCell(-1,-1);
    if (!ui->cbUseRights->isChecked())
    {
        ui->tbPlus->setEnabled(false);
        ui->tbMinus->setEnabled(false);
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            ui->tableWidget->item(i,1)->setFlags(ui->tableWidget->item(i,1)->flags() /*& ~(Qt::ItemIsSelectable)*/ & ~(Qt::ItemIsUserCheckable));
        }
        ui->tableWidget->setPalette(lPaletteDis);

    }
    else
    {
        bool lErr = true;
        for (int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            if(ui->tableWidget->item(i,0)->data(Qt::UserRole+1).toString() != db.userName())
            {
                ui->tableWidget->item(i,1)->setFlags(ui->tableWidget->item(i,1)->flags() | (Qt::ItemIsSelectable) | (Qt::ItemIsUserCheckable));
            }
        }
        ui->tbPlus->setEnabled(true);
        ui->tbMinus->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        if(ui->tableWidget->rowCount() > 0)
        {
            for( int i = 0; i < ui->tableWidget->rowCount(); i++ )
            {
                if (ui->tableWidget->item(i,0)->text() == gUsers->GetName(db.userName()))
                    lErr = false;
            }
        }
        if (lErr)
        {
            ui->tableWidget->setSortingEnabled(false);
            lItem = new QTableWidgetItem(gUsers->GetName(db.userName()));
            rItem = new QTableWidgetItem("");
            rItem->setCheckState(Qt::Checked);
            lItem->setData(Qt::UserRole+1,db.userName());
            if (gUsers->GetName(db.userName()).contains(QRegExp("[א-ת]")))
                lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            else
                lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lItem->setFlags(lItem->flags() & ~(Qt::ItemIsSelectable) & ~(Qt::ItemIsEnabled));
            rItem->setFlags(lItem->flags() & ~(Qt::ItemIsSelectable) & ~(Qt::ItemIsEnabled));
            rItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());
            //ui->tableWidget->setRowHeight(ui->tableWidget->rowCount()-1, ui->tableWidget->rowHeight(ui->tableWidget->rowCount()-1) - 5);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,0,lItem);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,1,rItem);
            ui->tableWidget->setSortingEnabled(true);
        }
        ui->tableWidget->setPalette(lPaletteDefault);

        ui->tableWidget->resizeRowsToContents();
        for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
            ui->tableWidget->setRowHeight(i, ui->tableWidget->rowHeight(i) - 5);
        }

        on_tableWidget_itemSelectionChanged();
    }
}

void PlotRightsDlg::on_buttonBox_accepted()
{
    if (ui->cbUseRights->isEnabled())
    {
        bool qInsertPrepared = false, qUpdatePrepared = false, qDeletePrepared = false;
        bool lErr;
        if (!db.transaction()) {
            gLogger->ShowSqlError(this, tr("Document access"), tr("Can't start transaction"), db);
            return;
        }
        QSqlQuery qUpdate(db);
        QSqlQuery qInsert(db);
        QSqlQuery qDelete(db);
        lErr = false;
        //if(!lErr && ui->cbUseRights->isChecked())
        {
            for (int i = 0; i < ui->tableWidget->rowCount() && !lErr; i++)
            {
                if (!ui->tableWidget->isRowHidden(i))
                {
                    if(!ui->tableWidget->item(i,0)->data(Qt::UserRole).isNull())
                    {
                        if (!qUpdatePrepared)
                        {
                            qUpdate.prepare("update v_plot_user set admin_option =:is_Admin where id = :id");
                            qUpdatePrepared = true;
                            if (qUpdate.lastError().isValid()) {
                                lErr = true;
                                gLogger->ShowSqlError(tr("User rights/Updating"), qUpdate);
                                break;
                            }
                        }
                        qUpdate.bindValue(":is_Admin",(ui->tableWidget->item(i,1)->checkState() == Qt::CheckState(Qt::Checked))? 1 : 0);
                        qUpdate.bindValue(":id",  ui->tableWidget->item(i,0)->data(Qt::UserRole));
                        if (!qUpdate.exec()) {
                            lErr = true;
                            gLogger->ShowSqlError(tr("User rights/Updating"), qUpdate);
                            break;
                        }
                    }
                    else
                    {
                        if (!qInsertPrepared)
                        {
                            qInsert.prepare("insert into v_plot_user (login, admin_option, id_plot) values (:login, :is_Admin, :id_plot)");
                            qInsertPrepared = true;
                            if (qInsert.lastError().isValid()) {
                                lErr = true;
                                gLogger->ShowSqlError(tr("User rights/Inserting"), qInsert);
                                break;
                            }
                        }
                        qInsert.bindValue(":id_plot", mIdPlot);
                        qInsert.bindValue(":is_Admin", (ui->tableWidget->item(i,1)->checkState() == Qt::CheckState(Qt::Checked))? 1 : 0);
                        qInsert.bindValue(":login",  ui->tableWidget->item(i,0)->data(Qt::UserRole+1));
                        if (!qInsert.exec()) {
                            lErr = true;
                            gLogger->ShowSqlError(tr("User rights/Inserting"), qInsert);
                            break;
                        }
                    }
                }
                else //isHidden ()
                {
                    if(!ui->tableWidget->item(i,0)->data(Qt::UserRole).isNull())
                    {
                        if(!qDeletePrepared)
                        {
                            qDelete.prepare("delete from v_plot_user where id = :id");
                            qDeletePrepared =true;
                            if (qDelete.lastError().isValid()) {
                                lErr = true;
                                gLogger->ShowSqlError(tr("User rights/Deleting"), qDelete);
                                break;
                            }
                        }
                        qDelete.bindValue(":id",  ui->tableWidget->item(i,0)->data(Qt::UserRole));
                        if (!qDelete.exec()) {
                            lErr = true;
                            gLogger->ShowSqlError(tr("User rights/Deleting"), qDelete);
                            break;
                        }
                    }
                }
            }
            if (!lErr)
            {
                if (get_Restrict != ui->cbUseRights->isChecked())
                {
                    qUpdate.prepare("update v_plot_simple set restrict_rights = :r_Rights where id = :id_plot");
                    if (qUpdate.lastError().isValid()) {
                        lErr = true;
                        gLogger->ShowSqlError(tr("Document access"), qUpdate);
                    } else
                    {
                        qUpdate.bindValue(":id_plot", mIdPlot);
                        qUpdate.bindValue(":r_Rights", ui->cbUseRights->isChecked() ? 1 : 0);
                        if (!qUpdate.exec()) {
                            lErr = true;
                            gLogger->ShowSqlError(tr("Document access"), qUpdate);
                        }
                    }
                }
            }
        }
        if (!lErr)
        {
            if (!db.commit())
                gLogger->ShowSqlError(this, tr("Document access"), tr("Can't commit"), db);
            else
                accept();
        }
        else
            db.rollback();
    }
}

void PlotRightsDlg::on_tbPlus_clicked()
{
    QList <UserData *> lSelected;
    QStringList lIgnoreLogins;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        if (!ui->tableWidget->isRowHidden(i))
            lIgnoreLogins.append(ui->tableWidget->item(i,0)->data(Qt::UserRole + 1).toString());
    }
    if (gUsers->SelectUsers(lSelected, &lIgnoreLogins)) {
        ui->tableWidget->setSortingEnabled(false);
        for (int i = 0; i < lSelected.length(); i++)
        {
            lItem = new QTableWidgetItem(lSelected.at(i)->NameConst());
            lItem->setData(Qt::UserRole+1,lSelected.at(i)->LoginConst());
            if (lSelected.at(i)->NameConst().contains(QRegExp("[א-ת]")))
                lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            else
                lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            rItem = new QTableWidgetItem("");
            rItem->setCheckState(Qt::Unchecked);
            rItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());
            //ui->tableWidget->setRowHeight(ui->tableWidget->rowCount()-1, ui->tableWidget->rowHeight(ui->tableWidget->rowCount()-1) - 5);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,0,lItem);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,1,rItem);
        }
        ui->tableWidget->setSortingEnabled(true);

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

void PlotRightsDlg::on_tbMinus_clicked()
{
    QList<QTableWidgetItem *> lSelected = ui->tableWidget->selectedItems();
    for (int i = 0; i < lSelected.count(); i++)
        ui->tableWidget->setRowHidden(lSelected.at(i)->row(), true);

    QApplication::processEvents();
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width() - ui->tableWidget->columnWidth(1)
                                    - 4
                                    - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));
    QApplication::processEvents();
    ui->tableWidget->setColumnWidth(0, ui->tableWidget->width() - ui->tableWidget->columnWidth(1)
                                    - 4
                                    - (ui->tableWidget->verticalScrollBar()->isVisible()?ui->tableWidget->verticalScrollBar()->width():0));
}

void PlotRightsDlg::on_tableWidget_itemSelectionChanged()
{
    if(ui->tableWidget->selectedItems().count() > 0 && ui->cbUseRights->isEnabled() && ui->cbUseRights->isChecked())
        ui->tbMinus->setEnabled(true);
    else
        ui->tbMinus->setEnabled(false);

}

void PlotRightsDlg::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu lPopup;
    if(ui->cbUseRights->isEnabled() && ui->cbUseRights->isChecked())
    {
       ui->actionDelete->setEnabled(true);
    }
    else
    {
        ui->actionDelete->setEnabled(false);
    }
    lPopup.addAction(ui->actionDelete);
    lPopup.addAction(ui->actionProperties);
    lPopup.exec(QCursor::pos());
}

void PlotRightsDlg::on_actionDelete_triggered()
{
    if (ui->tbMinus->isEnabled())
        on_tbMinus_clicked();
}

void PlotRightsDlg::on_actionProperties_triggered()
{
   QList<QTableWidgetItem *> lSelected;
    lSelected = ui->tableWidget->selectedItems();
    if (!lSelected.isEmpty())
    {
        UserData *lUser = gUsers->FindByName(ui->tableWidget->item(lSelected.at(0)->row(), 0)->text());
        if (lUser)
        {
            UserPropDlg w(lUser, this);
            w.exec();
        }
    }
}

void PlotRightsDlg::on_tableWidget_cellDoubleClicked(int row, int column)
{
    on_actionProperties_triggered();
}
