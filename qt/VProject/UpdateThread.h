#ifndef UPDATETHREAD_H
#define UPDATETHREAD_H

#include <QThread>
#include <QtSql/QSqlDriver>

#include "common.h"
#include "oracle.h"

class UpdateThread : public QThread
{
    Q_OBJECT
protected:
    // params
    QString mSessionId;
    QSqlDatabase mDB;

    QList<qint32> mIdProjects, mIdProjectsForAdd;
    QMutex mMutex4ModifyList;

    QList<tPairIntInt> mUpdateData;
    QMutex mMutex4UpdateData;

    bool mIsOk, mRun;
    QSqlQuery *mSqlAlert;

    void AddUpdateData(qint32 aType, qint64 aId);
public:
    explicit UpdateThread(const QSqlDatabase &aDB, QObject *parent = 0);
    virtual ~UpdateThread();

    void run() Q_DECL_OVERRIDE;
    void AddIdProject(qint32 aIdProject);

    bool GetUpdateData(tPairIntInt &aPair);

signals:
private slots:
public slots:
    void SlotNotification(const QString &aName, QSqlDriver::NotificationSource aSource, const QVariant &aPayload);
};

class EXP_IMP MyMutexLocker
{
    bool mIsLocked;
    QMutex *mMutex;
public:
    explicit MyMutexLocker(QMutex *aMutex, int aMS);
    virtual ~MyMutexLocker();

    bool IsLocked();
};

#endif // UPDATETHREAD_H

