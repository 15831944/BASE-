#ifndef NEWUSERDLG_H
#define NEWUSERDLG_H

#include <QDialog>

class UserData;

namespace Ui {
class NewUserDlg;
}

class NewUserDlg : public QDialog
{
    Q_OBJECT
private:
    UserData *mUser;
public:
    explicit NewUserDlg(QWidget *parent = 0);
    ~NewUserDlg();

    UserData *User() const;

private slots:
    void Accept();

    void on_leFullName_textEdited(const QString &arg1);

    void on_leName_textChanged(const QString &arg1);

    void on_leLogin_textEdited(const QString &arg1);

    void on_cbBirthDay_toggled(bool checked);

    void on_cbHireDate_toggled(bool checked);

private:
    Ui::NewUserDlg *ui;
};

#endif // NEWUSERDLG_H
