#ifndef SCHEDULERLIB_H
#define SCHEDULERLIB_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QSqlQueryModel>
#include <array>


namespace Ui {
class SchedulerLib;
}

class SchedulerLib : public QMainWindow
{
    Q_OBJECT

public:
    explicit SchedulerLib(QWidget *parent = 0);
    ~SchedulerLib(){}
 //   QDate getCurrentDate();
 //   void setEventDate(QDate eventdate);
    void showNewHeaders(QSqlQueryModel *sqlmodel);



private slots:
    void on_tV_centralschedule_clicked(const QModelIndex &index);

    void on_calendarWidget_activated(const QDate &);
    void checkForEvents();

    void on_btn_close_clicked();

private:
    Ui::SchedulerLib *ui;
    //static int m_currentrow = -1;

};

#endif // SCHEDULERLIB_H
