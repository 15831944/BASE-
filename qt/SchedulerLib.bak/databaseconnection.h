#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H


#include <QString>
#include <QDateTime>
#include <QSqlDatabase>

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

class DatabaseConnection
{

public:
    DatabaseConnection(const QString &path);
    virtual ~DatabaseConnection(){}

    static bool addEventToDB(const QString &task_name, const QString &subj, const QString &comments, const QString
                             &time_begin, const QString &time_end, const QString &location, const QString &time_alarm,
                             const QString resume, const QString duration, const QString insert_user, const QString update_user,
                             const QString date_begin_fact, const QString insert_date, const QString setup_date, const QString task_date,
                             const QString update_date);

    static int getEventIDFromEventName(const QString &task_name);

    static QSqlQuery getDataOfEvent(const QString &task_date);

    static void deleteEvent(const int &id);

    static void getNextEventData(QDate &nextEventRDate, QTime &nextEventRTime);

};

#endif // DATABASECONNECTION_H
