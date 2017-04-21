#ifndef NEWMEETING_H
#define NEWMEETING_H


#include "../Login/Login.h"
#include "../VProject/qfcdialog.h"
#include <qtablewidget.h>
#include <QScrollBar>

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

   void on_btn_OK_clicked();

   void on_tbPlus_Projects_clicked();

   void on_tbMinus_Projects_clicked();

   void on_tbPlus_nativeUsers_clicked();

   void on_tW_nativeUsers_customContextMenuRequested(const QPoint &pos);

   void on_tbMinus_nativeUsers_clicked();

   void on_tbMinus_otherUsers_clicked();

   void on_tbPlus_otherUsers_clicked();

   void on_actionDelete_triggered();

   void on_actionProperties_triggered();

   void on_actionCreate_triggered();


   void on_btn_Cancel_clicked();

protected:
   bool mShowDeleted;
       bool mFilterWithPersons, mFilterWithData;

private:
    Ui::NewMeeting *ui;
    QTableWidgetItem *lItem;

};

#endif // NEWMEETING_H
