#include "schedulerlib.h"
#include "ui_schedulerlib.h"
#include "databaseconnection.h"

SchedulerLib::SchedulerLib(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SchedulerLib)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QTimer *timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(checkForEvents()));
    timer->start(30000);

}

void SchedulerLib::setEventDate(QDate eventdate){

    ui->de_currentdate->setDate(eventdate);

}

QDate SchedulerLib::getCurrentDate()
{

    return ui->de_currentdate->date();
}

void SchedulerLib::on_tV_centralschedule_clicked(const QModelIndex &index)
{
   // m_currentrow = index.row();
    ui->tV_centralschedule->indexWidget(index);
}

void SchedulerLib::checkForEvents(){


    QDate nextEventRDate, currentDate = QDate::currentDate();
    QTime nextEventRTime, currentTime = QTime::currentTime();
    DatabaseConnection::getNextEventData(nextEventRDate, nextEventRTime);

}

void SchedulerLib::on_calendarWidget_activated(const QDate &date)
{

    //ui->calendarWidget->
    SchedulerLib dateConfig(this);
    dateConfig.setEventDate(date);
    //dateConfig.exec();
    connect(ui->calendarWidget, SIGNAL(selectionChanged()), this, SLOT(checkForEvents()));
}

void SchedulerLib::showNewHeaders(QSqlQueryModel *sqlmodel){

    ui->tV_centralschedule->setColumnHidden(0, true);

    std::array<std::string, 17> headerdataArray = {"task_name", "subj", "usr", "comments", "time_begin", "time_end", "location", "time_alarm", "resume", "duration", "insert_user", "update_user", "date_begin_fact", "insert_date", "setup_date", "task_date", "update_date"};

    int current = 0;
    while(current <= 16)
        sqlmodel->setHeaderData(++current, Qt::Horizontal, QString::fromStdString(headerdataArray.at(current)));

}

void SchedulerLib::on_btn_close_clicked()
{
    close();
}
