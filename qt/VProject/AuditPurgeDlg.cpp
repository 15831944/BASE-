#include "AuditPurgeDlg.h"
#include "ui_AuditPurgeDlg.h"

#include <QPushButton>

#include "GlobalSettings.h"

AuditPurgeDlg::AuditPurgeDlg(QWidget *parent) :
    QFCDialog(parent, false),
    ui(new Ui::AuditPurgeDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~(Qt::WindowContextHelpButtonHint));

    ui->cbPurgeRegApps->setChecked(gSettings->AuditPurge.PurgeRegapps);
    ui->cbPurgeAll->setChecked(gSettings->AuditPurge.PurgeAll);
    ui->cbExplodeProxy->setChecked(gSettings->AuditPurge.ExplodeProxy);
    ui->cbRemoveProxy->setChecked(gSettings->AuditPurge.RemoveProxy);
    ui->cbAudit->setChecked(gSettings->AuditPurge.Audit);
}

AuditPurgeDlg::~AuditPurgeDlg()
{
    delete ui;
}

void AuditPurgeDlg::Accept() {
    gSettings->AuditPurge.PurgeRegapps = ui->cbPurgeRegApps->isChecked();
    gSettings->AuditPurge.PurgeAll = ui->cbPurgeAll->isChecked();
    gSettings->AuditPurge.ExplodeProxy = ui->cbExplodeProxy->isChecked();
    gSettings->AuditPurge.RemoveProxy = ui->cbRemoveProxy->isChecked();
    gSettings->AuditPurge.Audit = ui->cbAudit->isChecked();

    accept();
}

void AuditPurgeDlg::CheckAnySelected(int aDummy) {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->cbPurgeRegApps->isChecked()
                                               || ui->cbPurgeAll->isChecked()
                                               || ui->cbExplodeProxy->isChecked()
                                               || ui->cbRemoveProxy->isChecked()
                                               || ui->cbAudit->isChecked());
}
