#ifndef ORGANIZATIONPROPDLG_H
#define ORGANIZATIONPROPDLG_H

#include <QStyledItemDelegate>

#include "../VProject/qfcdialog.h"
#include "CustomerData.h"

namespace Ui {
class OrganizationPropDlg;
}

class OrganizationPropDlg : public QFCDialog
{
    Q_OBJECT
public:
    explicit OrganizationPropDlg(CustomerData *aCustomer, QWidget *parent = 0);
    ~OrganizationPropDlg();

    int IdCustomer();

protected:
    virtual void showEvent(QShowEvent* event);

private slots:
    void ShowData();

    void on_buttonBox_accepted();

    void on_cbClient_toggled(bool checked);
    void on_cbProvider_toggled(bool checked);
private:
    bool mIsNew;
    CustomerData *mCustomer;

    Ui::OrganizationPropDlg *ui;
    bool mJustStarted;
};

#endif // ORGANIZATIONPROPDLG_H
