#include "logger.h"

#include <QMessageBox>
#include <QWindow>
#include <QApplication>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <QVariant>
#include <QHostInfo> // temporary

QString Logger::mLogFile = "";

Logger::Logger() :
    mOwner(NULL), mDB(NULL), mLoggerWnd(NULL)
{
//    connect(gLogger2, SIGNAL(signalLogErrorToDb(const QString &)), this, SLOT(LogErrorToDb(const QString &)));
//    connect(gLogger2, SIGNAL(signalLogErrorToFile(const QString &)), this, SLOT(LogErrorToFile(const QString &)));
//    connect(gLogger2, SIGNAL(signalLogError(const QString &)), this, SLOT(LogError(const QString &)));

//    connect(gLogger2, SIGNAL(signalShowErrorInList(QWidget *, const QString &, const QString &, bool)),
//            this, SLOT(ShowErrorInList(QWidget *, const QString &, const QString &, bool)));
//    connect(gLogger2, SIGNAL(signalShowError(QWidget *, const QString &, const QString &, bool)),
//            this, SLOT(ShowError(QWidget *, const QString &, const QString &, bool)));
//    connect(gLogger2, SIGNAL(signalShowError(const QString &, const QString &, bool)),
//            this, SLOT(ShowError(const QString &, const QString &, bool)));

//    connect(gLogger2, SIGNAL(signalShowSqlError(QWidget *, const QString &, const QString &, const QSqlError &, bool)),
//            this, SLOT(ShowSqlError(QWidget *, const QString &, const QString &, const QSqlError &, bool)));
//    connect(gLogger2, SIGNAL(signalShowSqlError(QWidget *, const QString &, const QSqlError &, bool)),
//            this, SLOT(ShowSqlError(QWidget *, const QString &, const QSqlError &, bool)));
//    connect(gLogger2, SIGNAL(signalShowSqlError(const QString &, const QSqlError &, bool)),
//            this, SLOT(ShowSqlError(const QString &, const QSqlError &, bool)));
//    connect(gLogger2, SIGNAL(signalShowSqlError(const QString &, const QString &, const QSqlError &, bool)),
//            this, SLOT(ShowSqlError(const QString &, const QString &, const QSqlError &, bool)));

    // rotate file
    if (mLogFile.isEmpty()) {
        mLogFile = QApplication::applicationDirPath();
        mLogFile.resize(mLogFile.lastIndexOf(QChar('/')));
        mLogFile.resize(mLogFile.lastIndexOf(QChar('/')));
        mLogFile += "/log/new.log";
    }

    QFile lErrorLog(mLogFile);
    if (lErrorLog.size() > 1024 * 1024 * 100) {
        lErrorLog.remove();
    } else if (lErrorLog.size() > 1024 * 1024 * 1) {
        // remove last (9)
        QFile::remove(mLogFile + ".9");
        for (int i = 9; i > 0; i--) {
            QFile::rename(mLogFile + "." + QString::number(i - 1), mLogFile + "." + QString::number(i));
        }
        LogErrorToFile("Rotating log");
        lErrorLog.rename(mLogFile + ".0");
    }
}

Logger::~Logger() {
    // crazy
    //if (mOwner) delete mOwner;
}

void Logger::WindowDeleted() {
    mLoggerWnd = NULL;
}

void Logger::SetLogFileName(const QString &aLogFileName) {
    mLogFile = aLogFileName;
}

Logger * Logger::GetInstance() {
    static Logger * lLogger = NULL;
    if (!lLogger) {
        lLogger = new Logger();
        //qAddPostRoutine(ProjectDataList::clean);
    }
    return lLogger;
}

LoggerWnd *Logger::GetLoggerWnd() const {
    return mLoggerWnd;
}
void Logger::SetOwner(QWidget *aOwner) {
    mOwner = aOwner;
}

//void Logger::SetOwner(WId aOwner) {
//    mOwner = QWindow::fromWinId(aOwner);
//}

void Logger::SetDefaultTitle(const QString &aDefaultTitle) {
    mDefaultTitle = aDefaultTitle;
}

void Logger::SetDefaultTitle(const wchar_t *aDefaultTitle) {
    mDefaultTitle.fromWCharArray(aDefaultTitle);
}

void Logger::SetDB(QSqlDatabase *aDB) {
    mDB = aDB;
}

void Logger::LogSqlQuery(const QSqlQuery &aQuery) {
    if (!aQuery.lastQuery().isEmpty()) {
        QString lLog = aQuery.lastQuery();
        QStringList lBindValues;

        QMapIterator<QString, QVariant> i(aQuery.boundValues());
        while (i.hasNext()) {
            i.next();
            lBindValues.append(QString("  ") + i.key().toUtf8().data() + ": " + i.value().toString().toUtf8().data());
        }

        QList<QVariant> list = aQuery.boundValues().values();
        for (int i = 0; i < list.size(); ++i)
            lBindValues.append("  " + QString::number(i) + ": " + list.at(i).toString().toUtf8().data());

        if (lBindValues.isEmpty()) {
            LogError(lLog);
        } else {
            LogError(lLog + "\r\n" + lBindValues.join("\r\n"));
        }
    } else {
        LogError("Empty query");
    }
}

void Logger::LogErrorToDb(const QString &aErrorText) {
    if (!mDB) return;

    QSqlQuery lQLog(*mDB);

    // in autonomous transaction
    lQLog.prepare("begin pp.adderrorlog(:text); end;");
    if (lQLog.lastError().isValid()) {
        LogErrorToFile("ErrorLog - prepare: " + lQLog.lastError().text());
    } else {
        lQLog.bindValue(":text", aErrorText);
        if (!lQLog.exec()) {
            // write to file - additional error
            LogErrorToFile("ErrorLog - execute: " + lQLog.lastError().text());
        }
    }
}

void Logger::LogErrorToFile(const QString &aErrorText) {
    // write to file
    QFile lErrorLog(mLogFile);

    if (lErrorLog.open(QFile::Append)) {
        QTextStream lOut(&lErrorLog);
        lOut.setCodec("UTF-8");
        lOut << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss ") << aErrorText << "\r\n";
        lErrorLog.flush();
        lErrorLog.close();
    }
}

void Logger::LogError(const QString &aErrorText) {
    // write to file
    LogErrorToFile(aErrorText);
    // save to DB
    LogErrorToDb(aErrorText);
}

void Logger::ShowErrorInList(QWidget *aOwner, const QString &aTitle, const QString &aErrorText, bool aSaveToDB) {
    if (aSaveToDB) {
        if (aTitle.isEmpty()) {
            LogError(aErrorText);
        } else {
            LogError(aTitle + ": " + aErrorText);
        }
    }
    if (!mLoggerWnd) {
        mLoggerWnd = new LoggerWnd(aOwner?aOwner:mOwner);
        mLoggerWnd->setAttribute(Qt::WA_DeleteOnClose);
        mLoggerWnd->show();

    }

    if (mLoggerWnd) {
        if (aTitle.isEmpty()) {
            mLoggerWnd->AddLineToLog(aErrorText);
        } else {
            mLoggerWnd->AddLineToLog(aTitle + ": " + aErrorText);
        }
    }
}

void Logger::ShowError(QWidget *aOwner, const QString &aTitle, const QString &aErrorText, bool aSaveToDB) {
    if (aSaveToDB) {
        if (aTitle.isEmpty()) {
            LogError(aErrorText);
        } else {
            LogError(aTitle + ": " + aErrorText);
        }
    }

    // show to user
    QMessageBox::critical(aOwner, aTitle.isEmpty()?mDefaultTitle:aTitle, aErrorText);
//    if (!mLoggerWnd) {
//        mLoggerWnd = new LoggerWnd(mOwner);
//        mLoggerWnd->setAttribute(Qt::WA_DeleteOnClose);
//        mLoggerWnd->show();
//    }
}

void Logger::ShowError(const QString &aTitle, const QString &aErrorText, bool aSaveToDB) {
    ShowError(mOwner, aTitle, aErrorText, aSaveToDB);
}

// query
void Logger::ShowSqlError(QWidget *aOwner, const QString &aTitle, const QString &aText, const QSqlQuery &aQuery, bool aSaveToDB) {
    ShowError(aOwner, aTitle, aText.isEmpty()?aQuery.lastError().text():(aText + "\r\n" + aQuery.lastError().text()), aSaveToDB);
    LogSqlQuery(aQuery);
}

void Logger::ShowSqlError(QWidget *aOwner, const QString &aTitle, const QSqlQuery &aQuery, bool aSaveToDB) {
    ShowError(aOwner, aTitle, aQuery.lastError().text(), aSaveToDB);
    LogSqlQuery(aQuery);
}

void Logger::ShowSqlError(const QString &aTitle, const QSqlQuery &aQuery, bool aSaveToDB) {
    ShowError(mOwner, aTitle, aQuery.lastError().text(), aSaveToDB);
    LogSqlQuery(aQuery);
}

void Logger::ShowSqlError(const QString &aTitle, const QString &aText, const QSqlQuery &aQuery, bool aSaveToDB) {
    ShowError(aTitle, aText.isEmpty()?aQuery.lastError().text():(aText + "\r\n" + aQuery.lastError().text()), aSaveToDB);
    LogSqlQuery(aQuery);
}

// db
void Logger::ShowSqlError(QWidget *aOwner, const QString &aTitle, const QString &aText, const QSqlDatabase &aDb, bool aSaveToDB) {
    ShowError(aOwner, aTitle, aText.isEmpty()?aDb.lastError().text():(aText + "\r\n" + aDb.lastError().text()), aSaveToDB);
}

void Logger::ShowSqlError(QWidget *aOwner, const QString &aTitle, const QSqlDatabase &aDb, bool aSaveToDB) {
    ShowError(aOwner, aTitle, aDb.lastError().text(), aSaveToDB);
}

void Logger::ShowSqlError(const QString &aTitle, const QSqlDatabase &aDb, bool aSaveToDB) {
    ShowError(mOwner, aTitle, aDb.lastError().text(), aSaveToDB);
}

void Logger::ShowSqlError(const QString &aTitle, const QString &aText, const QSqlDatabase &aDb, bool aSaveToDB) {
    ShowError(aTitle, aText.isEmpty()?aDb.lastError().text():(aText + "\r\n" + aDb.lastError().text()), aSaveToDB);
}

//----------------------------------------------------------------
LoggerInThread::LoggerInThread() {
    connect(this, SIGNAL(signalLogErrorToDb(const QString &)), gLogger, SLOT(LogErrorToDb(const QString &)));
    connect(this, SIGNAL(signalLogErrorToFile(const QString &)), gLogger, SLOT(LogErrorToFile(const QString &)));
    connect(this, SIGNAL(signalLogError(const QString &)), gLogger, SLOT(LogError(const QString &)));

    connect(this, SIGNAL(signalShowErrorInList(QWidget *, const QString &, const QString &, bool)),
            gLogger, SLOT(ShowErrorInList(QWidget *, const QString &, const QString &, bool)));
    connect(this, SIGNAL(signalShowError(QWidget *, const QString &, const QString &, bool)),
            gLogger, SLOT(ShowError(QWidget *, const QString &, const QString &, bool)));
    connect(this, SIGNAL(signalShowError(const QString &, const QString &, bool)),
            gLogger, SLOT(ShowError(const QString &, const QString &, bool)));
}

LoggerInThread::~LoggerInThread() {
}

LoggerInThread * LoggerInThread::GetInstance() {
    static LoggerInThread * lLogger = NULL;
    if (!lLogger) {
        lLogger = new LoggerInThread();
        //qAddPostRoutine(ProjectDataList::clean);
    }
    return lLogger;
}

void LoggerInThread::LogErrorToDb(QString aErrorText) {
    emit signalLogErrorToDb(aErrorText);
}

void LoggerInThread::LogErrorToFile(QString aErrorText) {
    emit signalLogErrorToFile(aErrorText);
}

void LoggerInThread::LogError(QString aErrorText) {
    emit signalLogError(aErrorText);
}

void LoggerInThread::ShowErrorInList(QWidget *aOwner, QString aTitle, QString aErrorText, bool aSaveToDB) {
    emit signalShowErrorInList(aOwner, aTitle, aErrorText, aSaveToDB);
}

void LoggerInThread::ShowError(QWidget *aOwner, QString aTitle, QString aErrorText, bool aSaveToDB) {
    emit signalShowError(aOwner, aTitle, aErrorText, aSaveToDB);
}

void LoggerInThread::ShowError(QString aTitle, QString aErrorText, bool aSaveToDB) {
    emit signalShowError(aTitle, aErrorText, aSaveToDB);
}
