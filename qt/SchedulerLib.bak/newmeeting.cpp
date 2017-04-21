#include "newmeeting.h"
#include "ui_newmeeting.h"

#include "../UsersDlg/UserData.h"
#include "../UsersDlg/UserRight.h"


#include <QFileDialog>

#include "../VProject/common.h"
#include "../VProject/GlobalSettings.h"
#include "../VProject/HomeData.h"
#include "../VProject/WaitDlg.h"
#include "../VProject/MainWindow.h"
#include "../VProject/qfcdialog.h"

#include "../UsersDlg/CustomerData.h"

#include "../ProjectLib/ProjectListDlg.h"
#include "../ProjectLib/ProjectData.h"

#include "projectrights.h"

NewMeeting::NewMeeting(QWidget *parent) : QFCDialog(parent, false), ui(new Ui::NewMeeting)
{
    ui->setupUi(this);
}


void NewMeeting::on_tB_Projects_clicked()
{
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    dSel.SetSelectedProject(ui->le_IdProjects->text().toLong() &&
                            ui->le_IdProjects_2->text().toLong()&&
                            ui->le_IdProjects_3->text().toLong() &&
                            ui->le_IdProjects_4->text().toLong()
                            );

    if (dSel.exec() == QDialog::Accepted/* && ui->twType->ProjectConst() != dSel.GetProjectData()*/) {
        ProjectData *lProject = dSel.GetProjectData();
        ui->le_IdProjects->setText(QString::number(lProject->Id()));
        ui->le_NameProjects->setText(lProject->FullShortName());

        ui->le_IdProjects_2->setText(QString::number(lProject->Id()));
        ui->le_NameProjects_2->setText(lProject->FullShortName());

        ui->le_IdProjects_3->setText(QString::number(lProject->Id()));
        ui->le_NameProjects_3->setText(lProject->FullShortName());

        ui->le_IdProjects_4->setText(QString::number(lProject->Id()));
        ui->le_NameProjects_4->setText(lProject->FullShortName());
    }
}

void NewMeeting::on_le_IdProjects_editingFinished()
{
    ProjectData *lProject = gProjects->FindByIdProject(ui->le_IdProjects->text().toLong());
    if (lProject) {
        ui->le_IdProjects->setText(QString::number(lProject->Id()));
        ui->le_NameProjects->setText(lProject->FullShortName());

    } else {
        ui->le_IdProjects->setText("");
        ui->le_NameProjects->setText("");

    }
}

void NewMeeting::on_le_IdProjects_2_editingFinished()
{
    ProjectData *lProject = gProjects->FindByIdProject(ui->le_IdProjects_2->text().toLong());
    if (lProject) {
        ui->le_IdProjects_2->setText(QString::number(lProject->Id()));
        ui->le_NameProjects_2->setText(lProject->FullShortName());
    } else {
        ui->le_IdProjects_2->setText("");
        ui->le_NameProjects_2->setText("");
    }
}

void NewMeeting::on_le_IdProjects_3_editingFinished()
{
    ProjectData *lProject = gProjects->FindByIdProject(ui->le_IdProjects_3->text().toLong());
    if (lProject) {
        ui->le_IdProjects_3->setText(QString::number(lProject->Id()));
        ui->le_NameProjects_3->setText(lProject->FullShortName());
    } else {
        ui->le_IdProjects_3->setText("");
        ui->le_NameProjects_3->setText("");
    }
}

void NewMeeting::on_le_IdProjects_4_editingFinished()
{
    ProjectData *lProject = gProjects->FindByIdProject(ui->le_IdProjects_4->text().toLong());
    if (lProject) {
        ui->le_IdProjects_4->setText(QString::number(lProject->Id()));
        ui->le_NameProjects_4->setText(lProject->FullShortName());
    } else {
        ui->le_IdProjects_4->setText("");
        ui->le_NameProjects_4->setText("");
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





//void NewMeeting::on_toolButton_3_clicked()
//{
//    ProjectRights NewUsers(this, this);
//    NewUsers.exec();

//}

void NewMeeting::on_btn_OK_clicked()
{
    ui->le_subject->selectedText();
    ui->le_address->selectedText();
    ui->te_apendix->selectAll();
}

void NewMeeting::on_tb_native_users_clicked()
{
    ui->cb_native_users1->setVisible(gUsers->HasPlotname());
    ui->cb_native_users2->setVisible(gUsers->HasPlotname());
    ui->cb_native_users3->setVisible(gUsers->HasPlotname());
    ui->cb_native_users4->setVisible(gUsers->HasPlotname());
    ui->cb_native_users5->setVisible(gUsers->HasPlotname());
}
