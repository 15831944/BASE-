#include "newmeeting.h"
#include "ui_newmeeting.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserRight.h"
#include "../UsersDlg/UserPropDlg.h"


#include <QFileDialog>

#include "../VProject/common.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/HomeData.h"
#include "../VProject/WaitDlg.h"
#include "../VProject/MainWindow.h"
#include "../VProject/qfcdialog.h"

#include "../UsersDlg/CustomerData.h"
#include "../UsersDlg/OrganizationsListDlg.h"
#include "../UsersDlg/OrganizationPropDlg.h"

#include "../ProjectLib/ProjectListDlg.h"
#include "../ProjectLib/ProjectData.h"

#include "projectrights.h"
#include <QDate>
#include <QTime>


NewMeeting::NewMeeting(QWidget *parent) : QFCDialog(parent, false), ui(new Ui::NewMeeting)
{
    ui->setupUi(this);

    QDateTime date_time_obj = QDateTime::currentDateTime();
    ui->le_DateTime->setText(date_time_obj.toString());

    ui->le_whoPlanning->setVisible(gUsers->HasPlotname());

    ui->de_currentdate->setDate(QDate::currentDate());
    ui->timeEdit_begin->setTime(QTime::currentTime());
    ui->timeEdit_end->setTime(QTime::currentTime());
 //   ui->timeEdit_begin->setTimeRange(QTime::fromString("00.00","hh.mm"), QTime::currentTime());
 //   ui->timeEdit_end->setTimeRange(QTime::fromString("00.00","hh.mm"), QTime::currentTime());

    ui->comboBox->setCurrentText("");

    if (ui->cB_date_other_end->isChecked())
    {

        ui->de_other_end->setDateRange(QDate::fromString("01.01.2000", "dd.MM.yyyy"), QDate::currentDate());
    }

}



QDate NewMeeting::getCurrentDate(){

    return ui->de_currentdate->date();

}

QTime NewMeeting::getCurrentTime(){

    return ui->timeEdit_begin->time();
}

QTime NewMeeting::getCurrentTime_end(){

    return ui->timeEdit_end->time();
}







void NewMeeting::on_btn_OK_clicked()
{
    ui->le_subject->selectedText();
    ui->le_address->selectedText();
    ui->te_apendix->selectAll();
}


void NewMeeting::on_tbPlus_Projects_clicked()
{
    QList <UserData *> lSelected;
    QStringList lIgnoreLogins;
    for (int i = 0; i < ui->tW_Projects->rowCount(); i++)
    {
        if (!ui->tW_Projects->isRowHidden(i))
            lIgnoreLogins.append(ui->tW_Projects->item(i, 0)->data(Qt::UserRole + 1).toString());
    }
    //lIgnoreLogins.append(db.userName());

    if (gUsers->SelectUsers(&lSelected, &lIgnoreLogins)) {
        //QStringList lStrsings;
        for (int i = 0; i < lSelected.length(); i++) {
            lItem = new QTableWidgetItem(lSelected.at(i)->NameConst());
            lItem->setData(Qt::UserRole + 1, lSelected.at(i)->LoginConst());
            if (lSelected.at(i)->NameConst().contains(QRegExp("[א-ת]")))
                lItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            else
                lItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            ui->tW_Projects->insertRow(ui->tW_Projects->rowCount());
            //ui->tableWidget->setRowHeight(ui->tableWidget->rowCount()-1, ui->tableWidget->rowHeight(ui->tableWidget->rowCount()-1) - 5);
            ui->tW_Projects->setItem(ui->tW_Projects->rowCount()-1, 0, lItem);
        }
        ui->tW_Projects->resizeRowsToContents();
        for (int i = 0; i < ui->tW_Projects->rowCount(); i++) {
            ui->tW_Projects->setRowHeight(i, ui->tW_Projects->rowHeight(i) - 5);
        }

        QApplication::processEvents();
        ui->tW_Projects->setColumnWidth(0, ui->tW_Projects->width() - ui->tW_Projects->columnWidth(1)
                                        - 4
                                        - (ui->tW_Projects->verticalScrollBar()->isVisible()?ui->tW_Projects->verticalScrollBar()->width():0));
        QApplication::processEvents();
        ui->tW_Projects->setColumnWidth(0, ui->tW_Projects->width() - ui->tW_Projects->columnWidth(1)
                                        - 4
                                        - (ui->tW_Projects->verticalScrollBar()->isVisible()?ui->tW_Projects->verticalScrollBar()->width():0));
    }
}

void NewMeeting::on_tbMinus_Projects_clicked()
{
    QList<QTableWidgetItem *> lSelected = ui->tW_Projects->selectedItems();
    for (int i = 0; i < lSelected.count(); i++)
        ui->tW_Projects->setRowHidden(lSelected.at(i)->row(), true);
    ui->tbMinus_Projects->setEnabled(false);

    QApplication::processEvents();
    ui->tW_Projects->setColumnWidth(0, ui->tW_Projects->width() - ui->tW_Projects->columnWidth(1)
                                    - 4
                                    - (ui->tW_Projects->verticalScrollBar()->isVisible()?ui->tW_Projects->verticalScrollBar()->width():0));
    QApplication::processEvents();
    ui->tW_Projects->setColumnWidth(0, ui->tW_Projects->width() - ui->tW_Projects->columnWidth(1)
                                    - 4
                                    - (ui->tW_Projects->verticalScrollBar()->isVisible()?ui->tW_Projects->verticalScrollBar()->width():0));

}


void NewMeeting::on_tW_nativeUsers_customContextMenuRequested(const QPoint &pos)
{
    if(ui->tW_nativeUsers->selectedItems().count() > 0) {
        QMenu lPopup;
        lPopup.addAction(ui->actionCreate);
        lPopup.addAction(ui->actionDelete);
        lPopup.addAction(ui->actionProperties);
        lPopup.exec(QCursor::pos());
    }
}


void NewMeeting::on_tbPlus_nativeUsers_clicked()
{
 //   ui->tbPlus_nativeUsers->setVisible(gUsers->HasPlotname());
    ui->tW_nativeUsers->setVisible(gUsers->HasPlotname());
}

void NewMeeting::on_tbMinus_nativeUsers_clicked()
{
    QList<QTableWidgetItem *> lSelected = ui->tW_nativeUsers->selectedItems();
    for (int i = 0; i < lSelected.count(); i++)
        ui->tW_nativeUsers->setRowHidden(lSelected.at(i)->row(), true);
    ui->tbMinus_nativeUsers->setEnabled(false);

    QApplication::processEvents();
    ui->tW_nativeUsers->setColumnWidth(0, ui->tW_nativeUsers->width() - ui->tW_nativeUsers->columnWidth(1)
                                    - 4
                                    - (ui->tW_nativeUsers->verticalScrollBar()->isVisible()?ui->tW_nativeUsers->verticalScrollBar()->width():0));
    QApplication::processEvents();
    ui->tW_nativeUsers->setColumnWidth(0, ui->tW_nativeUsers->width() - ui->tW_nativeUsers->columnWidth(1)
                                    - 4
                                    - (ui->tW_nativeUsers->verticalScrollBar()->isVisible()?ui->tW_nativeUsers->verticalScrollBar()->width():0));
}

void NewMeeting::on_tbMinus_otherUsers_clicked()
{
    QList<QTableWidgetItem *> lSelected = ui->tW_Customers->selectedItems();
    for (int i = 0; i < lSelected.count(); i++)
        ui->tW_Customers->setRowHidden(lSelected.at(i)->row(), true);
    ui->tbMinus_otherUsers->setEnabled(false);

    QApplication::processEvents();
    ui->tW_Customers->setColumnWidth(0, ui->tW_Customers->width() - ui->tW_Customers->columnWidth(1)
                                    - 4
                                    - (ui->tW_Customers->verticalScrollBar()->isVisible()?ui->tW_Customers->verticalScrollBar()->width():0));
    QApplication::processEvents();
    ui->tW_Customers->setColumnWidth(0, ui->tW_Customers->width() - ui->tW_Customers->columnWidth(1)
                                    - 4
                                    - (ui->tW_Customers->verticalScrollBar()->isVisible()?ui->tW_Customers->verticalScrollBar()->width():0));

}

void NewMeeting::on_tbPlus_otherUsers_clicked()
{
//    ui->tbPlus_otherUsers->setVisible(gCustomers->HasIsClient());
 //   ui->tW_Customers->setVisible(gCustomers->HasIsClient());
  //ui->tW_Customers->setVisible(gCustomers->HasIsClient());
//ui->tW_Customers->

//ui->tW_Customers->insertColumn(gCustomers->CustomerListConst());

  ui->tW_Customers->insertColumn(gCustomers->HasIsClient());


}

void NewMeeting::on_actionDelete_triggered()
{
    if (ui->tbMinus_nativeUsers->isEnabled())
        on_tbMinus_nativeUsers_clicked();
}

void NewMeeting::on_actionProperties_triggered()
{
    QList<QTableWidgetItem *> lSelected;
     lSelected = ui->tW_nativeUsers->selectedItems();
     if (!lSelected.isEmpty()) {
         UserData *lUser = gUsers->FindByName(ui->tW_nativeUsers->item(lSelected.at(0)->row(), 0)->text());
         if (lUser) {
             UserPropDlg w(lUser, this);
             w.exec();
         }
     }
}

void NewMeeting::on_actionCreate_triggered()
{
    if (ui->tbPlus_nativeUsers->isEnabled())
        on_tbPlus_nativeUsers_clicked();
}

void NewMeeting::on_btn_Cancel_clicked()
{
    ui->le_subject->setText("");
    ui->le_address->setText("");
    ui->te_apendix->setText("");
}
