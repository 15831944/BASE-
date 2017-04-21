#ifndef AUDITPURGEDLG_H
#define AUDITPURGEDLG_H

#include "qfcdialog.h"

namespace Ui {
class AuditPurgeDlg;
}

class AuditPurgeDlg : public QFCDialog
{
    Q_OBJECT

public:
    explicit AuditPurgeDlg(QWidget *parent = 0);
    ~AuditPurgeDlg();

private slots:
    void Accept();
    void CheckAnySelected(int aDummy);
private:
    Ui::AuditPurgeDlg *ui;
};


#endif // AUDITPURGEDLG_H
