#ifndef LOGGER_H
#define LOGGER_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <QWidget>

#include "logger_global.h"
#include "LoggerWnd.h"

class LOGGERSHARED_EXPORT Logger : public QObject
{
    friend LoggerWnd;
    Q_OBJECT
protected:
    QWidget *mOwner;
    QString mDefaultTitle;

    QSqlDatabase *mDB;

    static QString mLogFile;

    LoggerWnd *mLoggerWnd;

    explicit Logger();
    virtual ~Logger();

    void WindowDeleted();
public:
    static void SetLogFileName(const QString &aLogFileName);
    static Logger * GetInstance();

    LoggerWnd *GetLoggerWnd() const;

    void SetOwner(QWidget *aOwner);
//    void SetOwner(WId aOwner);

    void SetDefaultTitle(const QString &aDefaultTitle);
    void SetDefaultTitle(const wchar_t *aDefaultTitle);

    void SetDB(QSqlDatabase *aDB);

private:
    void LogSqlQuery(const QSqlQuery &aQuery);

public slots:
    void LogErrorToDb(const QString &aErrorText);
    void LogErrorToFile(const QString &aErrorText);
    void LogError(const QString &aErrorText);

    void ShowErrorInList(QWidget *aOwner, const QString &aTitle, const QString &aErrorText, bool aSaveToDB = true);
    void ShowError(QWidget *aOwner, const QString &aTitle, const QString &aErrorText, bool aSaveToDB = true);
    void ShowError(const QString &aTitle, const QString &aErrorText, bool aSaveToDB = true);

    void ShowSqlError(QWidget *aOwner, const QString &aTitle, const QString &aText, const QSqlQuery &aQuery, bool aSaveToDB = true);
    void ShowSqlError(QWidget *aOwner, const QString &aTitle, const QSqlQuery &aQuery, bool aSaveToDB = true);
    void ShowSqlError(const QString &aTitle, const QSqlQuery &aQuery, bool aSaveToDB = true);
    void ShowSqlError(const QString &aTitle, const QString &aText, const QSqlQuery &aQuery, bool aSaveToDB = true);

    void ShowSqlError(QWidget *aOwner, const QString &aTitle, const QString &aText, const QSqlDatabase &aDb, bool aSaveToDB = true);
    void ShowSqlError(QWidget *aOwner, const QString &aTitle, const QSqlDatabase &aDb, bool aSaveToDB = true);
    void ShowSqlError(const QString &aTitle, const QSqlDatabase &aDb, bool aSaveToDB = true);
    void ShowSqlError(const QString &aTitle, const QString &aText, const QSqlDatabase &aDb, bool aSaveToDB = true);
};

class LOGGERSHARED_EXPORT LoggerInThread : public QObject
{
    Q_OBJECT
protected:
    explicit LoggerInThread();
    virtual ~LoggerInThread();
public:
    static LoggerInThread * GetInstance();

    void LogErrorToDb(QString aErrorText);
    void LogErrorToFile(QString aErrorText);
    void LogError(QString aErrorText);

    void ShowErrorInList(QWidget *aOwner, QString aTitle, QString aErrorText, bool aSaveToDB = true);
    void ShowError(QWidget *aOwner, QString aTitle, QString aErrorText, bool aSaveToDB = true);
    void ShowError(QString aTitle, QString aErrorText, bool aSaveToDB = true);
signals:
    void signalLogErrorToDb(const QString &aErrorText);
    void signalLogErrorToFile(const QString &aErrorText);
    void signalLogError(const QString &aErrorText);

    void signalShowErrorInList(QWidget *aOwner, const QString &aTitle, const QString &aErrorText, bool aSaveToDB);
    void signalShowError(QWidget *aOwner, const QString &aTitle, const QString &aErrorText, bool aSaveToDB);
    void signalShowError(const QString &aTitle, const QString &aErrorText, bool aSaveToDB);
};

#define gLogger Logger::GetInstance()
#define gLogger2 LoggerInThread::GetInstance()

#endif // LOGGER_H
