#include "DocListSettingsDlg.h"
#include "ui_DocListSettingsDlg.h"

#include "TreeData.h"

#include "../ProjectLib/ProjectData.h"

#include <QDateTime>

DocListSettingsDlg::DocListSettingsDlg(ProjectData * aProject, QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::DocListSettingsDlg),
    mProject(aProject)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    ui->wdSaveData->setDisabled(true);

    if (mProject) {
        ui->leIdProject->setText(QString::number(mProject->Id()));
        ui->leProjName->setText(mProject->FullShortName());
    } else {
        ui->leIdProject->setText("0");
    }

    mTreeData = gTreeData->FindByGroupId(17);
    if (mTreeData) {
        ui->leTypeText->setText(mTreeData->FullName());
    }

    ui->leNameBottom->setText(QDateTime::currentDateTime().toString("dd.MM.yy hh:mm"));

    QSqlQuery query(db);

    query.prepare("select id, name, working from v_plot_simple where id_common = -17 and cancelled = 0 order by 2");

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Document list settings", query);
    } else {
        if (!query.exec()) {
            gLogger->ShowSqlError("Document list settings", query);
        } else {
            while (query.next()) {
                ui->cbTemplate->addItem(query.value("name").toString(), query.value("id"));
                if (query.value("working").toInt() == 1) ui->cbTemplate->setCurrentIndex(ui->cbTemplate->count() - 1);
            }
        }
    }
}

DocListSettingsDlg::~DocListSettingsDlg() {
    delete ui;
}

int DocListSettingsDlg::IdTemplate() {
    return ui->cbTemplate->currentData().toInt();
}

bool DocListSettingsDlg::NeedSave() {
    return ui->cbSave->isChecked();
}

int DocListSettingsDlg::IdProject() {
    return mProject->Id();
}

int DocListSettingsDlg::TypeArea() {
    if (mTreeData) {
        return mTreeData->Area();
    } else {
        return 0;
    }
}

int DocListSettingsDlg::TypeId() {
    if (mTreeData) {
        return mTreeData->Id();
    } else {
        return 0;
    }
}

QString DocListSettingsDlg::Code() {
    return ui->leCode->text().trimmed();
}

QString DocListSettingsDlg::NameTop() {
    return ui->leNameTop->text().trimmed();
}

QString DocListSettingsDlg::NameBottom() {
    return ui->leNameBottom->text().trimmed();
}

void DocListSettingsDlg::on_cbSave_toggled(bool checked) {
    ui->wdSaveData->setDisabled(!checked);

}

void DocListSettingsDlg::on_leIdProject_editingFinished() {
    ProjectData *lProject = gProjects->FindByIdProject(ui->leIdProject->text().toInt());
    if (lProject) {
        mProject = lProject;
        ui->leProjName->setText(mProject->FullShortName());
        // regen code
    } else {
        if (mProject) {
            ui->leIdProject->setText(QString::number(mProject->Id()));
        } else {
            ui->leIdProject->setText("0");
        }
    }
}
