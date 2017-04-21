#include "databaseconnection.h"



DatabaseConnection::DatabaseConnection(const QString &path)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL") ;
    db.setHostName    ( "localhost" );
    db.setPort        ( 5432        );
    db.setDatabaseName( path    );
    // TODO Это спрашивать у пользователя
    db.setUserName( "postgres" );
    db.setPassword( "1111"       );

    bool OK = db.open() ;
    if ( ! OK )
        qCritical() << "Cannot connect ro database" ;
    else
        qDebug() << "Database connected" ;
}

void DatabaseConnection::getNextEventData(QDate &nextEventRDate, QTime &nextEventRTime)
{
    QSqlQuery query;

    query.exec("SELECT * FROM V_SCHEDULE");
    query.first();
    query.record();


    nextEventRDate = QDate::fromString(query.value(0).toString());
    nextEventRTime = QTime::fromString(query.value(1).toString());
}

void DatabaseConnection::deleteEvent(const int &id)
{
    QSqlQuery query;

    query.prepare("DELETE FROM V_SCHEDULE WHERE ID = :ID");
    query.bindValue(":ID", id);
    query.exec();

}

QSqlQuery DatabaseConnection::getDataOfEvent(const QString &task_date)
{
    QSqlQuery query;

    query.prepare("SELECT * FROM V_SCHEDULE WHERE task_date = :task_date");
    query.bindValue(":task_date", task_date);
    query.exec();
    query.first();

    return query;
}

int DatabaseConnection::getEventIDFromEventName(const QString &task_name)
{
    // Currently unused:
    QSqlQuery query;
    query.prepare("SELECT ID FROM V_SCHEDULE WHERE task_name = :task_name");
    query.bindValue(":task_name", task_name);
    query.exec();
    query.first();

    return 0;
}

bool DatabaseConnection::addEventToDB(const QString &task_name, const QString &subj, const QString &comments, const QString
                                      &time_begin, const QString &time_end, const QString &location, const QString &time_alarm,
                                      const QString resume, const QString duration, const QString insert_user, const QString update_user,
                                      const QString date_begin_fact, const QString insert_date, const QString setup_date, const QString task_date,
                                      const QString update_date)
{
    QSqlQuery query;


    query.prepare("INSERT INTO V_SCHEDULE VALUES (NULL, :task_name, :subj, :usr, :comments, :time_begin, :time_end, :location, :time_alarm, :resume, :duration, :insert_user, :update_user, :date_begin_fact, :insert_date, :setup_date, :task_date, :update_date");
    query.bindValue(":task_name", task_name);
    query.bindValue(":subj", subj);
    query.bindValue(":usr", comments);
    query.bindValue(":time_begin", time_begin);
    query.bindValue(":time_end", time_end);
    query.bindValue(":location", location);
    query.bindValue(":time_alarm", time_alarm);

    query.bindValue(":resume", resume);
    query.bindValue(":duration", duration);
    query.bindValue(":insert_user", insert_user);
    query.bindValue(":update_user", update_user);
    query.bindValue(":date_begin_fact", date_begin_fact);
    query.bindValue(":insert_date", insert_date);

    query.bindValue(":setup_date", setup_date);
    query.bindValue(":task_date", task_date);
    query.bindValue(":update_date", update_date);




    if(query.exec())
    {
        return true;
    }
    else
    {
        qDebug() << "Error: "
                 << query.lastError();
        return false;
    }
}
