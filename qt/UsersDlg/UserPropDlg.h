#ifndef USERPROPDLG_H
#define USERPROPDLG_H

#include "UserData.h"

#include "../VProject/qfcdialog.h"

#if defined(VPROJECT_MAIN_IMPORT)
    #define USERSDLG_LIBRARY_EXP_IMP Q_DECL_IMPORT
#else
    #if defined(USERSDLG_LIBRARY)
        #define USERSDLG_LIBRARY_EXP_IMP Q_DECL_EXPORT
    #else
        #define USERSDLG_LIBRARY_EXP_IMP
    #endif
#endif


namespace Ui {
class UserPropDlg;
}

class USERSDLG_LIBRARY_EXP_IMP UserPropDlg : public QFCDialog
{
    Q_OBJECT
protected:
    UserData *mUser;

    bool mWillRestore;

    virtual void showEvent(QShowEvent* event);
public:
    explicit UserPropDlg(UserData *aUser, QWidget *parent = 0);
    ~UserPropDlg();

private slots:
    void ShowData();

    void on_cbBirthDay_toggled(bool checked);

    void on_cbHireDate_toggled(bool checked);

    void Accept();
    void on_pbFired_clicked();

private:
    bool mJustStarted;
    void InitInConstructor();

    Ui::UserPropDlg *ui;
};

#undef USERSDLG_LIBRARY_EXP_IMP

#endif // USERPROPDLG_H
