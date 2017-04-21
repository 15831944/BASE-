#include "ProjectTypeDlg.h"
#include "ui_ProjectTypeDlg.h"

#include "ProjectTypeData.h"

ProjectTypeDlg::ProjectTypeDlg(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::ProjectTypeDlg),
    mJustStarted(true)
{
    ui->setupUi(this);
}

ProjectTypeDlg::~ProjectTypeDlg()
{
    delete ui;
}

void ProjectTypeDlg::showEvent(QShowEvent* event) {
    QFCDialog::showEvent(event);

    if (mJustStarted) {
        ShowData();
        mJustStarted = false;
    }
}

void ProjectTypeDlg::ShowData() {
    int i;

    ui->cbProjectType->clear();

    for (i = 0; i < gProjectTypes->ProjTypeListConst().length(); i++) {
        ui->cbProjectType->addItem(gProjectTypes->ProjTypeListConst().at(i)->TypeNameConst(),
                                   gProjectTypes->ProjTypeListConst().at(i)->Id());
    }
}
