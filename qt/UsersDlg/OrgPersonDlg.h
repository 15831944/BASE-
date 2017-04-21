#ifndef ORGPERSONDLG_H
#define ORGPERSONDLG_H

#include "../VProject/qfcdialog.h"
#include "CustomerData.h"

namespace Ui {
class OrgPersonDlg;
}

class OrgPersonDlg : public QFCDialog
{
    Q_OBJECT
protected:
    CustomerData *mCustomer;
    CustomerPerson *mPerson;

    virtual void showEvent(QShowEvent* event);
public:
    explicit OrgPersonDlg(CustomerData *aCustomer, CustomerPerson *aPerson, QWidget *parent = 0);
    ~OrgPersonDlg();

private slots:
    void ShowData();

    void Accept();
private:
    bool mJustStarted;
    void InitInConstructor();

    Ui::OrgPersonDlg *ui;
};

#endif // ORGPERSONDLG_H
