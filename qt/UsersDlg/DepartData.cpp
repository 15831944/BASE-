#include "DepartData.h"

#include "../VProject/GlobalSettings.h"
#include "../VProject/common.h"

#include <QMessageBox>
#include <QVariant>

DepartData::DepartData(int aId, const QString &aName) :
    InitParRO(Id), InitPar(Name)
{
}

bool DepartData::RemoveFromDB() {
    bool res = false;
    QSqlQuery qDelete(db);
    qDelete.prepare("delete from department where id = :id");
    if (qDelete.lastError().isValid()) {
        gLogger->ShowSqlError(QObject::tr("Department data"), qDelete);
    } else {
        qDelete.bindValue(":id", mId);
        if (!qDelete.exec()) {
            gLogger->ShowSqlError(QObject::tr("Department data"), qDelete);
        } else {
            res = true;
        }
    }
    return res;
}

bool DepartData::SaveData() {
    bool res = false;
    if (!mId) {
        // new department data
        int lId;

        if (!gOracle->GetSeqNextVal("department_id_seq", lId)) {
            return false;
        }

        QSqlQuery qInsert(db);
        qInsert.prepare("insert into department (id, name) values (:id, :name)");
        if (qInsert.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Department data"), qInsert);
        } else {
            qInsert.bindValue(":id", lId);
            qInsert.bindValue(":name", mName);
            if (!qInsert.exec()) {
                gLogger->ShowSqlError(QObject::tr("Department data"), qInsert);
            } else {
                mId = lId;
                res = true;
            }
        }
    } else {
        QSqlQuery qUpdate(db);
        qUpdate.prepare("update department set name = :name where id = :id");
        if (qUpdate.lastError().isValid()) {
            gLogger->ShowSqlError(QObject::tr("Department data"), qUpdate);
        } else {
            qUpdate.bindValue(":id", mId);
            qUpdate.bindValue(":name", mName);
            if (!qUpdate.exec()) {
                gLogger->ShowSqlError(QObject::tr("Department data"), qUpdate);
            } else {
                res = true;
            }
        }
    }
    return res;
}

void DepartData::RollbackEdit() {
    RollbackPar(Name)
}

//-----------------------------------------------------------------------------------------------------------------------------------
DepartDataList::DepartDataList() :
    QObject(),
    mIsInited(false)
{
}

DepartDataList::~DepartDataList() {
    qDeleteAll(mDepartList);
}

void DepartDataList::InitDepartList() {
    qDeleteAll(mDepartList);
    mDepartList.clear();

    QSqlQuery query("select id, name from department order by name", db);

    if (query.lastError().isValid()) {
        gLogger->ShowSqlError("Department data", query);
    } else {
        while (query.next()) {
            mDepartList.append(new DepartData(query.value("id").toInt(), query.value("name").toString()));
        }
        mIsInited = true;
        emit DepartsNeedUpdate();
    }
}

QList<DepartData *> & DepartDataList::DepartListRef() {
    if (!mIsInited) InitDepartList();
    return mDepartList;
}

const QList<DepartData *> & DepartDataList::DepartListConst() {
    if (!mIsInited) InitDepartList();
    return mDepartList;
}

DepartData * DepartDataList::FindById(int aId) {
    if (!aId) return NULL;
    if (!mIsInited) InitDepartList();
    foreach (DepartData *lData, mDepartList) {
        if (lData->Id() == aId) return lData;
    }
    return NULL;
}
