#include "UpdateThread.h"

#include <QVariant>
#include <QTime>
#include <QApplication>

#include "GlobalSettings.h"

UpdateThread::UpdateThread(const QSqlDatabase &aDB, QObject *parent) :
    QThread(parent), mSqlAlert(NULL)
{
    mDB = QSqlDatabase::cloneDatabase(aDB, "AlertWatcher");

    QSqlQuery qSelect((mDB.driverName() == "QPSQL")
                      ?"select pg_backend_pid()"
                     :"select dbms_session.unique_session_id from dual", aDB);

    if (qSelect.lastError().isValid()) {
        gLogger->ShowSqlError("Session id", qSelect);
    } else {
        if (qSelect.next()) {
            mSessionId = ";" + mDB.userName() + ";" + qSelect.value(0).toString() + ";";
            //QMessageBox::critical(NULL, "", qSelect.value(0).toString());
        }
    }
}

UpdateThread::~UpdateThread() {
    if (mSqlAlert) delete mSqlAlert;
}

void UpdateThread::SlotNotification(const QString &aName, QSqlDriver::NotificationSource aSource, const QVariant &aPayload) {
    if (aPayload == mSessionId) return;
    if (aName == mSessionId) {
        QMutexLocker lLocker(&mMutex4ModifyList);
        while (!mIdProjectsForAdd.isEmpty()) {
            if (mIdProjectsForAdd.contains(0)) {
                mRun = false;
                break;
            }
            if (mDB.driverName() == "QPSQL") {
               if (mIsOk = mDB.driver()->subscribeToNotification("PLOTS_" + QString::number(mIdProjectsForAdd.first()))) {
                   // alert added
                   mIdProjects.append(mIdProjectsForAdd.first());
               } else {
                   gLogger2->ShowError("UpdateThread - subscribe", mSqlAlert->lastError().text());
               }
            } else {
                mSqlAlert->bindValue(":alert_name", "PLOTS_" + QString::number(mIdProjectsForAdd.first()));
                if (!mSqlAlert->exec()) {
                    gLogger2->ShowError("dbms_alert.register - exec", mSqlAlert->lastError().text());
                    mIsOk = false;
                } else {
                    // alert added
                    mIdProjects.append(mIdProjectsForAdd.first());
                }
            }
            mIdProjectsForAdd.removeFirst();
        }
    } else if (aName == "PROJECTS") {
        AddUpdateData(0, 0);
    } else if (aName == "PROJGROUPS") {
        AddUpdateData(1, 0);
    } else if (aName == "PROJECT_REMOVED") {
        AddUpdateData(2, 0);
    } else if (aName == "PROJGROUP_REMOVED") {
        AddUpdateData(3, 0);
    } else if (!aName.indexOf("PLOTS_")) {
        AddUpdateData(4, aName.mid(6).toULongLong());
    }
}

void UpdateThread::run() {
    mIsOk = true;

    if (!mDB.open()) {
        gLogger2->ShowError("UpdateThread - login", mDB.lastError().text());
        mIsOk = false;
    } else {
        if (mDB.driverName() == "QPSQL") {
            if (mIsOk = mDB.driver()->subscribeToNotification(mSessionId)
                    && mDB.driver()->subscribeToNotification("PROJECTS")
                    && mDB.driver()->subscribeToNotification("PROJGROUPS")
                    && mDB.driver()->subscribeToNotification("PROJECT_REMOVED")
                    && mDB.driver()->subscribeToNotification("PROJGROUP_REMOVED")
                    && connect(mDB.driver(), SIGNAL(notification(const QString &, QSqlDriver::NotificationSource, const QVariant &)),
                               this, SLOT(SlotNotification(const QString &, QSqlDriver::NotificationSource, const QVariant &)))) {
                QSqlQuery qDummy(mDB);
                mRun = true;
                while (mIsOk && mRun) {
                    sleep(15);
                    if (!qDummy.exec("select 1")) {
                        gLogger2->ShowError("UpdateThread", qDummy.lastError().text());
                        mIsOk = false;
                    }
                }
            } else {
                gLogger2->ShowError("UpdateThread - start", mDB.lastError().text());
            }
        } else {
            QSqlQuery qAlter("alter session set current_schema = " + gSettings->CurrentSchema, mDB);

            if (qAlter.lastError().isValid()) {
                gLogger2->ShowError("UpdateThread", qAlter.lastError().text());
                mIsOk = false;
            } else {
                if (!qAlter.exec()) {
                    gLogger2->ShowError("UpdateThread", qAlter.lastError().text());
                    mIsOk = false;
                } else {
                    mSqlAlert = new QSqlQuery(mDB);
                    mSqlAlert->prepare("begin dbms_alert.register(:alert_name); end;");
                    if (mSqlAlert->lastError().isValid()) {
                        gLogger2->ShowError("dbms_alert.register - prepare", mSqlAlert->lastError().text());
                        mIsOk = false;
                    } else {
                        QStringList lAlerts;
                        lAlerts.append(mSessionId);
                        lAlerts.append("PROJECTS");
                        lAlerts.append("PROJGROUPS");
                        lAlerts.append("PROJECT_REMOVED");
                        lAlerts.append("PROJGROUP_REMOVED");

                        for (int i = 0; i < lAlerts.length(); i++) {
                            mSqlAlert->bindValue(":alert_name", lAlerts.at(i));
                            if (!mSqlAlert->exec()) {
                                gLogger2->ShowError("dbms_alert.register - exec", mSqlAlert->lastError().text());
                                mIsOk = false;
                            }
                        }

                        if (mIsOk) {
                            QSqlQuery lSqlWait(mDB);
                            lSqlWait.prepare("begin dbms_alert.waitany(:alert_name, :message, :status); end;");
                            if (lSqlWait.lastError().isValid()) {
                                gLogger2->ShowError("dbms_alert.waitany - prepare", lSqlWait.lastError().text());
                                mIsOk = false;
                            } else {
                                int lStatus;
                                QByteArray a(38, ' '), b(350, ' ');

                                mRun = true;
                                while (mIsOk && mRun) {
                                    // QString::setUnicode don't want to work on XP!
                                    //lAlertName.setUnicode(&lSpace, 38);
                                    //lMessage.setUnicode(&lSpace, 350);

                                    QString lAlertName(a), lMessageDUMMY(b);

                                    lSqlWait.bindValue(":alert_name", lAlertName, QSql::Out);
                                    lSqlWait.bindValue(":message", lMessageDUMMY, QSql::Out);
                                    lSqlWait.bindValue(":status", lStatus, QSql::Out);
                                    if (lSqlWait.exec()) {
                                        if (!lSqlWait.boundValue(":status").toInt()) {
                                            lAlertName = lSqlWait.boundValue(":alert_name").toString();
                                            /////
                                            if (lAlertName == mSessionId) {
                                                QMutexLocker lLocker(&mMutex4ModifyList);
                                                while (!mIdProjectsForAdd.isEmpty()) {
                                                    if (mIdProjectsForAdd.contains(0)) {
                                                        mRun = false;
                                                        break;
                                                    }
                                                    mSqlAlert->bindValue(":alert_name", "PLOTS_" + QString::number(mIdProjectsForAdd.first()));
                                                    if (!mSqlAlert->exec()) {
                                                        gLogger2->ShowError("dbms_alert.register - exec", mSqlAlert->lastError().text());
                                                        mIsOk = false;
                                                    } else {
                                                        // alert added
                                                        mIdProjects.append(mIdProjectsForAdd.first());
                                                    }
                                                    mIdProjectsForAdd.removeFirst();
                                                }
                                            } else if (lAlertName == "PROJECTS") {
                                                AddUpdateData(0, 0);
                                            } else if (lAlertName == "PROJGROUPS") {
                                                AddUpdateData(1, 0);
                                            } else if (lAlertName == "PROJECT_REMOVED") {
                                                AddUpdateData(2, 0);
                                            } else if (lAlertName == "PROJGROUP_REMOVED") {
                                                AddUpdateData(3, 0);
                                            } else if (!lAlertName.indexOf("PLOTS_")) {
                                                AddUpdateData(4, lAlertName.mid(6).toULongLong());
                                            }
                                            /////
                                        }
                                    } else {
                                        //                                        if (lSqlWait.lastError().text().contains("ORA-03113") // ORA-03113: end-of-file on communication channel
                                        //                                                || lSqlWait.lastError().text().contains("ORA-00028") // ORA-00028: your session has been killed
                                        //                                                || lSqlWait.lastError().text().contains("ORA-03114")) { // ORA-03114: not connected to ORACLE
                                        //                                        } else {
                                        //                                            gLogger2->LogError("dbms_alert.waitany - exec: " + lSqlWait.lastError().text());
                                        //                                        }
                                        gLogger2->ShowError("dbms_alert.waitany - exec", lSqlWait.lastError().text());
                                        mIsOk = false;
                                    }
                                } // while
                            }
                        }
                    }
                }
            }
        }
        mDB.close();
    }
    if (!mIsOk) {
        AddUpdateData(5, 0); // close main connection, need restart program
    }
    exit();
}

void UpdateThread::AddIdProject(qint32 aIdProject) {
    QMutexLocker lLocker(&mMutex4ModifyList);
    if (!mIdProjects.contains(aIdProject)
            && !mIdProjectsForAdd.contains(aIdProject)) {
        mIdProjectsForAdd.append(aIdProject);

        QSqlQuery lSqlAlert(db);
        if (db.driverName() == "QPSQL") {
            if (!lSqlAlert.exec("select pg_notify('" + mSessionId + "', '')")) {
                gLogger->ShowSqlError("UpdateThread::AddIdProject", lSqlAlert);
            }
        } else {
            lSqlAlert.prepare("begin dbms_alert.signal(:alert_name, ''); end;");
            if (lSqlAlert.lastError().isValid()) {
                gLogger->ShowSqlError("dbms_alert.signal - prepare", lSqlAlert);
            } else {
                lSqlAlert.bindValue(":alert_name", mSessionId);
                if (!lSqlAlert.exec()) {
                    gLogger->ShowSqlError("dbms_alert.signal - exec", lSqlAlert);
                }
            }
        }
    }
}

void UpdateThread::AddUpdateData(qint32 aType, qint64 aId) {
    QMutexLocker lLocker(&mMutex4UpdateData);
    tPairIntInt lPair = qMakePair(aType, aId);
    if (!mUpdateData.contains(lPair)) mUpdateData.append(lPair);
}

bool UpdateThread::GetUpdateData(tPairIntInt &aPair) {
    QMutexLocker lLocker(&mMutex4UpdateData);

    if (mUpdateData.isEmpty()) return false;
    aPair = mUpdateData.takeFirst();
    return true;
}

MyMutexLocker::MyMutexLocker(QMutex *aMutex, int aMS) :
    mIsLocked(false), mMutex(aMutex)
{
    if (mMutex) {
        mIsLocked = mMutex->tryLock(aMS);
    }
}

MyMutexLocker::~MyMutexLocker() {
    if (mIsLocked) {
        mMutex->unlock();
    }
}

bool MyMutexLocker::IsLocked() {
    return mIsLocked;
}
