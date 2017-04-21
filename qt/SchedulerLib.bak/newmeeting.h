#ifndef NEWMEETING_H
#define NEWMEETING_H


#include "../Login/Login.h"
#include "../VProject/qfcdialog.h"

class UserData;

namespace Ui {
class NewMeeting;
}

class NewMeeting : public QFCDialog
{
    Q_OBJECT

public:
    NewMeeting(QWidget *parent = 0);
    virtual ~NewMeeting(){}
    QDate getCurrentDate();
    QTime getCurrentTime();
    QTime getCurrentTime_end();


private slots:
    void on_tB_Projects_clicked();

    void on_le_IdProjects_editingFinished();

    void on_le_IdProjects_2_editingFinished();

    void on_le_IdProjects_3_editingFinished();

    void on_le_IdProjects_4_editingFinished();

//    void checkForDateTime();



   void on_btn_OK_clicked();

   void on_tb_native_users_clicked();

private:
    Ui::NewMeeting *ui;
};

#endif // NEWMEETING_H
