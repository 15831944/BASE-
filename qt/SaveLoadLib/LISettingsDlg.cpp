#include "LISettingsDlg.h"
#include "ui_LISettingsDlg.h"

#include "../VProject/GlobalSettings.h"

LISettingsDlg::LISettingsDlg(QWidget *parent) :
    QFCDialog(parent, true),
    ui(new Ui::LISettingsDlg),
    mJustStarted(true)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
}

LISettingsDlg::~LISettingsDlg()
{
    delete ui;
}

void LISettingsDlg::Accept() {
    if (ui->widget->DoSave()) {
        accept();
    }
}
