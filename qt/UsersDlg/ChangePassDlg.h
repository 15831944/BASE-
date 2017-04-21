#ifndef CHANGEPASSDLG_H
#define CHANGEPASSDLG_H

#include "../VProject/qfcdialog.h"

class UserData;

namespace Ui {
class ChangePassDlg;
}

class ChangePassDlg : public QFCDialog
{
    Q_OBJECT
protected:
    UserData *mUser;

public:
    explicit ChangePassDlg(UserData *aUser, QWidget *parent = 0);
    ~ChangePassDlg();

private slots:
    void Accept();

private:
    Ui::ChangePassDlg *ui;
};

#endif // CHANGEPASSDLG_H
