#include "HomeData.h"

#include "common.h"

HomeData::HomeData()
{

}

QString HomeData::Get(const QString &aName) {
    QMap<QString, QString>::const_iterator itr = mData.find(aName);
    // do not use cache for "NEED_UPDATE_RIGHTS"
    if (aName != "NEED_UPDATE_RIGHTS" && itr != mData.end() && itr.key() == aName) {
        return itr.value();
    } else {
        // not found
        QString lValue;

        QSqlQuery qHomeData("select thevalue from homedata where thename = '" + aName + "'", db);

        if (qHomeData.lastError().isValid()) {
            // no error - it is mean NULL (empty string for strings or 0 for numbers)
            //gLogger->ShowSqlError("get home data", qHomeData);
        } else {
            if (qHomeData.next()) {
                lValue = qHomeData.value(0).toString();
            }
        }

        // do not use cache for "NEED_UPDATE_RIGHTS"
        if (aName != "NEED_UPDATE_RIGHTS") {
            mData[aName] = lValue; // store in memory cache
        }
        return lValue;
    }

}

void HomeData::Set(const QString &aName, const QString &aValue) {
    if (aName != "NEED_UPDATE_RIGHTS") return; // update NEED_UPDATE_RIGHTS only

    QSqlQuery qUpdate(db);
    qUpdate.prepare("update homedata set thevalue = :thevalue where thename = :thename");
    if (qUpdate.lastError().isValid()) {
        gLogger->ShowSqlError("GlobalSettings::SetHomeData", qUpdate);
    } else {
        qUpdate.bindValue(":thevalue", aValue);
        qUpdate.bindValue(":thename", aName);
        if (!qUpdate.exec()) {
            gLogger->ShowSqlError("GlobalSettings::SetHomeData", qUpdate);
        }
    }
}
