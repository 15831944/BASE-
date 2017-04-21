#include "projectrights.h"
#include "ui_projectrights.h"

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

ProjectRights::ProjectRights(QWidget *parent) : QFCDialog(parent, false), ui(new Ui::ProjectRights)
{
    ui->setupUi(this);
}


void ProjectRights::on_le_IdProject_editingFinished()
{
    ProjectData *lProject = gProjects->FindByIdProject(ui->le_IdProject->text().toLong());
    if (lProject) {
        ui->le_IdProject->setText(QString::number(lProject->Id()));
        ui->le_NameProject->setText(lProject->FullShortName());
    } else {
        ui->le_IdProject->setText("");
        ui->le_NameProject->setText("");
    }
}

void ProjectRights::on_tb_Projects_clicked()
{
    ProjectListDlg dSel(ProjectListDlg::DTShowSelect, this);

    dSel.SetSelectedProject(ui->le_IdProject->text().toLong());

    if (dSel.exec() == QDialog::Accepted/* && ui->twType->ProjectConst() != dSel.GetProjectData()*/) {
        ProjectData *lProject = dSel.GetProjectData();
        ui->le_IdProject->setText(QString::number(lProject->Id()));
        ui->le_NameProject->setText(lProject->FullShortName());
    }
}

void ProjectRights::on_tb_select_plus_clicked()
{
    ui->tV_listUsers->setVisible(gUsers->HasPlotname());
}

void ProjectRights::on_tb_select_minus_clicked()
{

}

void ProjectRights::on_btn_OK_clicked()
{
    ui->tV_listUsers->selectAll();
    ui->le_IdProject->selectedText();
    ui->le_NameProject->selectedText();
}

void ProjectRights::on_btn_Cancel_clicked()
{
    mCancelled = true ;
}

void ProjectRights::on_tV_listUsers_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->tV_listUsers->indexAt(pos);
    QMenu *menu = new QMenu(this);

    menu->addAction(new QAction("Добавить", this));
    menu->addAction(new QAction("Удалить", this));
    menu->addAction(new QAction("Свойства пользователя", this));

    menu->popup(ui->tV_listUsers->viewport()->mapToGlobal(pos));

}
